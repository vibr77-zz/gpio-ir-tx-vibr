// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017 Sean Young <sean@mess.org>
 * Copyright (C) 2020 Vincent Besson VIBR <vincent@besson.be>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <media/rc-core.h>

#define DRIVER_NAME	"gpio-ir-tx-vibr"
#define DEVICE_NAME	"GPIO IR Bit Banging Transmitter modified by VIBR"

#define MAX_GPIO 8

struct gpio_ir {
	struct gpio_desc *gpio_vibr[MAX_GPIO];
	struct gpio_desc *gpio;
	struct device *dev;
	unsigned int num_transmitters;
	unsigned int carrier;
	unsigned int tx_mask;
	unsigned int duty_cycle;
	/* we need a spinlock to hold the cpu while transmitting */
	spinlock_t lock;
};

static const struct of_device_id gpio_ir_tx_of_match[] = {
	{ .compatible = "gpio-ir-tx-vibr", },
	{ },
};
MODULE_DEVICE_TABLE(of, gpio_ir_tx_of_match);

static bool inline gpio_ir_transmitter_enabled(int n,unsigned int tx_mask) 
{
	return tx_mask & (1 << n);
}


static int gpio_ir_tx_set_duty_cycle(struct rc_dev *dev, u32 duty_cycle)
{
	struct gpio_ir *gpio_ir = dev->priv;

	gpio_ir->duty_cycle = duty_cycle;

	return 0;
}

static int gpio_ir_tx_set_carrier(struct rc_dev *dev, u32 carrier)
{
	struct gpio_ir *gpio_ir = dev->priv;

	if (!carrier)
		return -EINVAL;

	gpio_ir->carrier = carrier;

	return 0;
}

static int gpio_ir_tx(struct rc_dev *dev, unsigned int *txbuf,unsigned int count)
{
	struct gpio_ir *gpio_ir = dev->priv;
	struct gpio_desc * active_gpio=NULL; // <- use as pointer to ease 
	spinlock_t active_lock;
	unsigned long flags;
	ktime_t edge;
	/*
	 * delta should never exceed 0.5 seconds (IR_MAX_DURATION) and on
	 * m68k ndelay(s64) does not compile; so use s32 rather than s64.
	 */
	s32 delta;
	int i,j;
	unsigned int pulse, space;

	/* Ensure the dividend fits into 32 bit */
	pulse = DIV_ROUND_CLOSEST(gpio_ir->duty_cycle * (NSEC_PER_SEC / 100),gpio_ir->carrier);
	space = DIV_ROUND_CLOSEST((100 - gpio_ir->duty_cycle) *(NSEC_PER_SEC / 100), gpio_ir->carrier);

	edge = ktime_get();
	spin_lock_irqsave(&active_lock, flags);
	for (j = 0; j < gpio_ir->num_transmitters; j++){
		if (gpio_ir_transmitter_enabled(j,gpio_ir->tx_mask)){
			dev_info(gpio_ir->dev,"VIBR sending tx on Transmitter:%d frame:%d\n",j+1,count);
			
			active_gpio=gpio_ir->gpio_vibr[j];
			active_lock=gpio_ir->lock;
			
			

			for (i = 0; i < count; i++) {
				if (i % 2) {
					// space
					edge = ktime_add_us(edge, txbuf[i]);
					delta = ktime_us_delta(edge, ktime_get());
					if (delta > 10) {
						spin_unlock_irqrestore(&active_lock, flags);
						usleep_range(delta, delta + 10);
						spin_lock_irqsave(&active_lock, flags);
					} else if (delta > 0) {
						udelay(delta);
					}
				} else {
					// pulse
					ktime_t last = ktime_add_us(edge, txbuf[i]);

					while (ktime_before(ktime_get(), last)) {
						gpiod_set_value(active_gpio, 1);
						edge = ktime_add_ns(edge, pulse);
						delta = ktime_to_ns(ktime_sub(edge,ktime_get()));
						if (delta > 0)
							ndelay(delta);
						gpiod_set_value(active_gpio, 0);
						edge = ktime_add_ns(edge, space);
						delta = ktime_to_ns(ktime_sub(edge,ktime_get()));
						if (delta > 0)
							ndelay(delta);
					}

					edge = last;
				}
			}
			
		}	
	}
	spin_unlock_irqrestore(&active_lock, flags);
	dev_info(gpio_ir->dev,"VIBR end tx frame:%d",count);
	return count;
}

// Add By VIBR

static int gpio_ir_set_tx_mask(struct rc_dev *dev, uint32_t mask)
{
	// Add by VIBR
	struct gpio_ir * gpio_ir = dev->priv;
	dev_info(gpio_ir->dev,"VIBR setting the tx_mask to:%u\n",mask);
	gpio_ir->tx_mask=mask;

	return 0;
}
static int gpio_ir_tx_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev,"VIBR module bye bye baby\n");
	return 0;
}
static int gpio_ir_tx_probe(struct platform_device *pdev)
{
	struct gpio_ir *gpio_ir;
	struct rc_dev *rcdev;
	int rc;
	int i;

	dev_info(&pdev->dev,"VIBR module start\n");
	dev_info(&pdev->dev,"VIBR module Max Transmitters:%d\n",MAX_GPIO);
	
	gpio_ir = devm_kmalloc(&pdev->dev, sizeof(*gpio_ir), GFP_KERNEL);
	if (!gpio_ir)
		return -ENOMEM;
	
	gpio_ir->num_transmitters=0;
	
	for(i=0;i<MAX_GPIO;i++){
		gpio_ir->gpio_vibr[i]=NULL; // just in case ;)
	}

	for (i=0;i<MAX_GPIO;i++){
		gpio_ir->gpio_vibr[i]=devm_gpiod_get_index(&pdev->dev, "vibr", i, GPIOD_OUT_LOW);

		if (IS_ERR(gpio_ir->gpio_vibr[i])) {
			if (PTR_ERR(gpio_ir->gpio_vibr[i]) != -EPROBE_DEFER)
				dev_err(&pdev->dev, "VIBR break failed to get gpio_%d (%ld)\n",i,PTR_ERR(gpio_ir->gpio_vibr[i]));
			break;
		
		}
		if (i==0)
			gpio_ir->gpio=devm_gpiod_get_index(&pdev->dev, "vibr", i, GPIOD_OUT_LOW);

		dev_info(&pdev->dev,"VIBR Multi gpio:%d pin %d",i,desc_to_gpio(gpio_ir->gpio_vibr[i]));
		gpio_ir->num_transmitters=i+1;
	}

	dev_info(&pdev->dev,"VIBR Mutli gpio, %d found",gpio_ir->num_transmitters);
	dev_info(&pdev->dev,"VIBR Mutli gpio,setting tx_mask to 0xFFFFFFFF (all transmitters active)");
	gpio_ir->tx_mask=0xFFFFFFFF;


	rcdev = devm_rc_allocate_device(&pdev->dev, RC_DRIVER_IR_RAW_TX);
	if (!rcdev)
		return -ENOMEM;

	gpio_ir->dev=&pdev->dev; // Add by VIB
	
	// gpio_ir->gpio = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
	// if (IS_ERR(gpio_ir->gpio)) {
	// 	if (PTR_ERR(gpio_ir->gpio) != -EPROBE_DEFER)
	// 		dev_err(&pdev->dev, "Failed to get gpio (%ld)\n",
	// 			PTR_ERR(gpio_ir->gpio));
	// 	return PTR_ERR(gpio_ir->gpio);
	// }

	rcdev->priv = gpio_ir;
	rcdev->driver_name = DRIVER_NAME;
	rcdev->device_name = DEVICE_NAME;
	rcdev->tx_ir = gpio_ir_tx;
	rcdev->s_tx_duty_cycle = gpio_ir_tx_set_duty_cycle;
	rcdev->s_tx_carrier = gpio_ir_tx_set_carrier;
	rcdev->s_tx_mask=gpio_ir_set_tx_mask; // add by VIB

	gpio_ir->carrier = 38000;
	gpio_ir->duty_cycle = 50;
	spin_lock_init(&gpio_ir->lock);

	rc = devm_rc_register_device(&pdev->dev, rcdev);
	if (rc < 0)
		dev_err(&pdev->dev, "failed to register rc device\n");
	dev_info(&pdev->dev,"driver registered");
	return rc;
}

static struct platform_driver gpio_ir_tx_driver = {
	.probe	= gpio_ir_tx_probe,
	.remove = gpio_ir_tx_remove,
	.driver = {
		.name	= DRIVER_NAME,
		.of_match_table = of_match_ptr(gpio_ir_tx_of_match),
	},
};
module_platform_driver(gpio_ir_tx_driver);

MODULE_DESCRIPTION("GPIO IR Multi GPIO Transmitters");
MODULE_AUTHOR("Vincent Besson VIBR <vincent@besson.be>");
MODULE_LICENSE("GPL");
