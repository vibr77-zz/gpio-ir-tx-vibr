#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xe338ec46, "module_layout" },
	{ 0x7ee9f129, "platform_driver_unregister" },
	{ 0xb1a6baa3, "__platform_driver_register" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0xd697e69a, "trace_hardirqs_on" },
	{ 0x365a79dc, "gpiod_set_value" },
	{ 0x12a38747, "usleep_range" },
	{ 0xb43f9365, "ktime_get" },
	{ 0xec3d2e1b, "trace_hardirqs_off" },
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0x881991d7, "devm_rc_register_device" },
	{ 0x84323789, "devm_gpiod_get" },
	{ 0x8db7edce, "devm_rc_allocate_device" },
	{ 0x3fe4d2a, "desc_to_gpio" },
	{ 0xce2f7cb4, "_dev_err" },
	{ 0xae3ddfa, "gpiod_get_index" },
	{ 0x35e882d1, "devm_kmalloc" },
	{ 0xb3a83540, "_dev_info" },
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cgpio-ir-tx-vibr");
MODULE_ALIAS("of:N*T*Cgpio-ir-tx-vibrC*");

MODULE_INFO(srcversion, "E489B55CF7B61475EA8CEA1");
