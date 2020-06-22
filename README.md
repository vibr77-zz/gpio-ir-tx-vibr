# gpio-ir-tx-vibr
Linux Kernel module GPIO-IR with multi GPIO management.
The new gpio-ir-tx that replace the lirc_rpi is not managing multiple transmitters (via SET_TRANSMITTERS). this proposes version enable the multi gpio with tx_mask.

This has been successfully testing on
Rasperry 3B+ & 4B

Note that I am struggling with Raspberry pi zero, where the original and this new version are not working (pulse issue)

How to make it works

first update to the last linux kernel version

apt-get update
apt-get upgrade
rpi-update

Then get the linux source
cd /home/pi
sudo apt-get install git bc bison flex libssl-dev
sudo wget https://raw.githubusercontent.com/notro/rpi-source/master/rpi-source -O /usr/local/bin/rpi-source && sudo chmod +x /usr/local/bin/rpi-source && /usr/local/bin/rpi-source -q --tag-update
rpi-source

compile the new driver 
cd gpio-ir-tx
make

Modify your /boot/config
sudo vi /boot/config

and add at the end this line

dtoverlay=gpio-ir-tx-vibr,gpio_pin=13

Note : gpio_pin=13, is not manage since gpio_pin is setup in the dts file 

Edit the DTS file, the section below is where to put your pin configuration,

vibr-gpios = 	<&gpio 21 0>,
							<&gpio 23 0>,
							<&gpio 24 0>,
							<&gpio 25 0>;
              
First number is for the pin number, the second is for the state 0 => LOW => Output

Note: The MAX gpio is 8 but feel free to increase the Define MAX_GPIO in the source

Compile the DTS 

sudo  dtc -@ -I dts -O dtb -o gpio-ir-tx-vibr.dtbo gpio-ir-tx-vibr-overlay.dts

Install the Compiled DTS

sudo cp gpio-ir-tx-vibr.dtbo /boot/overlays/
sudo reboot 

After reboot check the dts is loaded

sudo vcdbg log msg

You should see the log regarding gpio-ir-tx-vibr

Now load the new kernel driver in the kernel 

sudo rmmod gpio-ir-tx
sudo insmod gpio-ir-tx

tail -f /var/log/kern

You should see the message of load

then we your lirc configured

irsend SET_TRANSMITTERS 1 2 // to send to First transmitter and second
irsend SET_TRANSMITTERS 2 // for the second transmitter

Note there is a bug in the current version, when sending to multiple transmitters there is a lock issue and irsend is not giving back hand, will manage that later.

No issue with 1 transmitter at a time

Enjoy the new module




