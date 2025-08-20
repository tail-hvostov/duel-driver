# duel-driver
## Запуск
dtc -@ -I dts -O dtb -o ssd1306.dtbo ssd1306-overlay.dts

sudo cp ssd1306.dtbo /boot/dtb/allwinner/overlay/sun50i-h616-ssd1306.dtbo

sudo nano /boot/armbianEnv.txt

Отредактировать /boot/armbianEnv.txt: overlays=ssd1306

## Распиновка
- VCC - 3v3
- SCL - PH6
- SDA - PH7
- RES - PC10
- DC  - PC7
- CS  - PH9