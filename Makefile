.DEFAULT_GOAL := upload
BUILD_NAME := "/tmp/arduino_build"
CACHE_NAME := "/tmp/arduino_cache"
ARDUINO_DIR := "/home/$(USER)/arduino-1.8.12-linux64/arduino-1.8.12/"
ARDUINO_CONF_DIR := "/home/$(USER)/.arduino15"
SKETCH_NAME := "main.cpp"
TTY_USB := "/dev/ttyUSB0"
BAUDRATE := "115200"

create_dirs: clean
	mkdir $(BUILD_NAME)
	mkdir $(CACHE_NAME)

dump_prefs: create_dirs
	$(ARDUINO_DIR)/arduino-builder -dump-prefs -logger=machine -hardware $(ARDUINO_DIR)/hardware -hardware $(ARDUINO_CONF_DIR)/packages -tools $(ARDUINO_DIR)/tools-builder -tools $(ARDUINO_DIR)/hardware/tools/avr -tools $(ARDUINO_CONF_DIR)/packages -built-in-libraries $(ARDUINO_DIR)/libraries -libraries ./libraries -fqbn=adafruit:nrf52:feather52832:softdevice=s132v6,debug=l0 -ide-version=10807 -build-path $(BUILD_NAME) -warnings=none -build-cache $(CACHE_NAME) -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.gcc-arm-none-eabi.path=$(ARDUINO_CONF_DIR)/packages/adafruit/tools/gcc-arm-none-eabi/5_2-2015q4 -prefs=runtime.tools.gcc-arm-none-eabi-5_2-2015q4.path=$(ARDUINO_CONF_DIR)/packages/adafruit/tools/gcc-arm-none-eabi/5_2-2015q4 -prefs=runtime.tools.nrfjprog.path=$(ARDUINO_CONF_DIR)/packages/adafruit/tools/nrfjprog/9.4.0 -prefs=runtime.tools.nrfjprog-9.4.0.path=$(ARDUINO_CONF_DIR)/packages/adafruit/tools/nrfjprog/9.4.0 -verbose $(SKETCH_NAME) > /dev/null

compile: dump_prefs
	$(ARDUINO_DIR)/arduino-builder -compile -logger=machine -hardware $(ARDUINO_DIR)/hardware -hardware $(ARDUINO_CONF_DIR)/packages -tools $(ARDUINO_DIR)/tools-builder -tools $(ARDUINO_DIR)/hardware/tools/avr -tools $(ARDUINO_CONF_DIR)/packages -built-in-libraries $(ARDUINO_DIR)/libraries -libraries ./libraries -fqbn=adafruit:nrf52:feather52832:softdevice=s132v6,debug=l0 -ide-version=10807 -build-path $(BUILD_NAME) -warnings=none -build-cache $(CACHE_NAME) -prefs=build.warn_data_percentage=75 -prefs=runtime.tools.gcc-arm-none-eabi.path=$(ARDUINO_CONF_DIR)/packages/adafruit/tools/gcc-arm-none-eabi/5_2-2015q4 -prefs=runtime.tools.gcc-arm-none-eabi-5_2-2015q4.path=$(ARDUINO_CONF_DIR)/packages/adafruit/tools/gcc-arm-none-eabi/5_2-2015q4 -prefs=runtime.tools.nrfjprog.path=$(ARDUINO_CONF_DIR)/packages/adafruit/tools/nrfjprog/9.4.0 -prefs=runtime.tools.nrfjprog-9.4.0.path=$(ARDUINO_CONF_DIR)/packages/adafruit/tools/nrfjprog/9.4.0 -verbose $(SKETCH_NAME) > /dev/null

upload: compile
	adafruit-nrfutil --verbose dfu serial -pkg $(BUILD_NAME)/$(SKETCH_NAME).zip -p $(TTY_USB) -b $(BAUDRATE) --singlebank

clean:
	rm -rf $(BUILD_NAME) $(CACHE_NAME)
