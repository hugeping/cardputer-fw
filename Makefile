all: compile upload

compile:
	arduino-cli compile -v -b esp32:esp32:stamp-s3:CDCOnBoot=cdc,PartitionScheme=default_8MB,FlashSize=8M --build-property compiler.cpp.extra_flags="-Wall"

upload:
	arduino-cli upload -v -b esp32:esp32:stamp-s3:CDCOnBoot=cdc,PartitionScheme=default_8MB,FlashSize=8M -p /dev/ttyACM0
