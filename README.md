#Presentation

WazaBee is an attack allowing to transmit and receive 802.15.4 packets by diverting Bluetooth Low Energy chips. This software is a basic proof of concept implementing the two communication primitives used by this attack. This source code is not dedicated to an offensive usage, and it is only provided for research purposes. It is released as an open-source project under MIT license.

This repository contains the WazaBee PoC firmware for the following development board: **Adafruit nRF52832 Feather**. This firmware exposes a basic API that can be manipulated using the WazaBee Command Line Interface.

#Building and flashing this firmware

* Download and install Arduino IDE: <https://www.arduino.cc/en/Main/Software>

* Go into **File -> Preferences**.

* In the *Additional Board Manager URL* field, enter the following URL: *https://www.adafruit.com/package_adafruit_index.json*

* Restart Arduino IDE, then go into **Tools -> Board -> Board Manager**.

* Install *"Adafruit nRF52 by Adafruit"*.

* Select *"Adafruit Bluefruit nRF52832 Feather"* from the **Tools -> Board** menu.

* If needed, use the following instructions to update the bootloader: <https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide/updating-the-bootloader>.

* Enter the following command to install *adafruit-nrfutil*:
```
$ pip3 install --user adafruit-nrfutil
```

* Add the *adafruit-nrfutil* directory in your *PATH*:
```
$ export PATH=$PATH:/home/$USER/.local/bin 
```

* Go to *wazabee_nrf52* directory, then modify the Makefile to match your build environment if needed:
```
BUILD_NAME := "/tmp/arduino_build"
CACHE_NAME := "/tmp/arduino_cache"
ARDUINO_DIR := "/home/$(USER)/arduino-1.8.12-linux64/arduino-1.8.12/"
ARDUINO_CONF_DIR := "/home/$(USER)/.arduino15"
```

* Plug the board, then type the following command to build and flash the firmware:
```
$ make
```
