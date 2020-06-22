#!/bin/bash
make clean 
make
sudo rmmod gpio-ir-tx
sudo insmod ./gpio-ir-tx.ko
sudo  dtc -@ -I dts -O dtb -o gpio-ir-tx-vibr-overlay.dtb gpio-ir-tx-vibr-overlay.dts
sudo cp gpio-ir-tx-vibr-overlay.dtb /boot/overlays/gpio-ir-tx-vibr.dtbo
