### Purpose
UI Speed Runner Quickly review UI screens in development on target device

### Configuring targets
- Set parameters in secrets.h (see config.h for list of required parameters)
- Set parameters in config.h
- Switch screen types in SCREEN routines in ui_speed_runner.ino

### Software Dependencies
- add library for appropriate sensor from known, working BOM
	- Adafruit LC709203F library (#define BATTERY)
	- Adafruit EPD library (MagTag)
- include all dependencies to these libraries

### known, working BOM
- MCU
	- ESP32S2. ESP8266
	- ARM(m0,m4)
		- deepSleep() not implemented
- Ethernet
	- Particle Ethernet Featherwing: https://www.adafruit.com/product/4003
	- Silicognition PoE Featherwing: https://www.crowdsupply.com/silicognition/poe-featherwing
- WiFi
	- esp32s2 boards
- battery monitor
	- LC709203F battery voltage monitor: https://www.adafruit.com/product/4712
- screen
	- Adafruit MagTag (EPD): https://www.adafruit.com/product/4800
	- LCD and OLED screens -> see code history before 02/2022
- battery
	- Adafruit 2000mA battery: https://www.adafruit.com/product/2011

### Information Sources
- NTP
	- https://github.com/PaulStoffregen/Time/tree/master/examples/TimeNTP
- Ethernet
	- https://docs.particle.io/datasheets/accessories/gen3-accessories/
	- https://www.adafruit.com/product/4003#:~:text=Description%2D-,Description,along%20with%20a%20Feather%20accessory
	- https://learn.adafruit.com/adafruit-wiz5500-wiznet-ethernet-featherwing/usage
	- https://www.arduino.cc/en/reference/ethernet
	- https://store.arduino.cc/usa/arduino-ethernet-rev3-without-poe
- Display
	- https://cdn-learn.adafruit.com/downloads/pdf/adafruit-gfx-graphics-library.pdf

### Issues
- See GitHub Issues for project

### Feature Requests
- See GitHub Issues for project

### Questions