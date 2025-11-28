dims=$1
dtc -@ -I dts -O dtb -o overlays/ssd1306.dtbo "overlays/ssd1306-$1-overlay.dts"
sudo cp overlays/ssd1306.dtbo /boot/dtb/allwinner/overlay/sun50i-h616-ssd1306.dtbo
