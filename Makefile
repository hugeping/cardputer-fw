all:
	arduino-cli compile -v -b esp32:esp32:stamp-s3:CDCOnBoot=cdc,PartitionScheme=default_8MB,FlashSize=8M
upload:
	arduino-cli upload -v -b esp32:esp32:stamp-s3 -p /dev/ttyACM0
