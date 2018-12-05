Cryptoclock ESP8266
===================

Cryptoclock is a device based on ESP8266 chip, designed for displaying prices, messages, tickers, or any other arbitrary data on a compact display.

Hardware:
----------
In it's basic form it consists of stock ESP8266 development board (NodeMCU, WeMOS, SparkFun, etc.) connected
to a compatible display without any additional electronic components.
By default, 32x8 LED matrix with MAX7219 driver chips is used, along with optional gyroscope module (for automatic display orientation).

Currently supported displays:
-----------------------------
* Any graphical display configuration supported by U8g2 library (see https://github.com/olikraus/u8g2/wiki/u8g2setupcpp for details)
* TM1637 7-segment driver
* Lixie
* Neopixel (Work in Progress)

Peripherials
-------------
* MPU6050 gyroscope/accelerometer module via I2C interface
  * SDA - D2(GPIO4)
  * SCL - D1(GPIO5)
* Piezoelectric buzzer
  * D8(GPIO15)
* MAX7219 chainable led matrix
  * DATA - D5(GPIO14)
  * CS - D6(GPIO12)
  * CLK - D7(GPIO13))

Mode of Operation
------------------
On startup, the firmware connectes to configured WiFi. (If no WiFI is available, or not yet configured, it switches the device into AP mode.)
When WiFi is successfully connected, it connects to a set websocket server, and starts continuously reading and displaying any numbers
that are sent by the server. The server can be a www.cryptoclock.net server, or any custom websocket server, using the same protocol.
(See protocol section for details).

Pressing the menu button (by default mapped to boot/flash on dev boards) brings out menu which can be navigated using
short and long presses of the button.
Holding the menu button down for more than 1 sec switches the device into AP mode.
Holding the menu button down for more than 10 secs will erase all the stored device settings, bringing it back to factory defaults.

AP mode
--------
In AP mode, the device acts as a WiFi Access Point (SSID shown on display) with captive portal.
You can use e.g. smartphone/notebook to connect to it and using your device browser to configure both WiFi credentials and device settings.
If not automatically redirected, you can point your browser to http://192.168.4.1 to access the configuration page.

Menu
-----
Use short button presses to switch between items, and long press to select/confirm.

* OTP - sends OTP request to server (see section OTP for details)
* Info - displays info about current firmware
* Clock - sets if clock should be periodically displayed in place of data (Cl On/Cl off), or if clock is the only thing displayed (Cl only)
* ERASE - erase all the stored device settings, bringing it back to factory defaults

OTP
----
One-Time Password function is used to register(pair) a device with a server account. (As by default, server connections are anonymous).
Activating the OTP on the device will send an OTP request to a server. Server responds with numeric password which is then shown on display.
You can then enter the displayed number into server's web interface to pair the device with your account.

Note that each device is identified to the server using an unique number (UUID), which is randomly generated after device ERASE,
 requiring you to re-add the device if you choose so.

Protocol
---------
The protocol is implemented using standard websocket messages (ws:// or wss://).
Any received message not starting with a semicolon (';') is treated as a numeric value, and the display will show rolling number
animation from previously received value.

Any message starting with a semicolon followed by space ('; xxx') is discarded

Any message starting with a semicolon (';') is treated as a command:

### Commands sent by server

__UPDATE__
  sent by server to trigger OTA firmware update. Firmware is also automatically updated on device startup.
  Firmware update server url can be set in device settings, or set to empty ('') to disable firmware updates.

__RESET__
  sent by server to trigger device restart

__ATH=X__
  sent by server to set All-Time-High threshold. When the price/number displayed reaches this threshold,
  the display will flash for a short time.

__MSG some message__
  display the message specified as a rolling text on the display

__STATICMSG X some message__
__MSGSTATIC X some message__
  display the message specified as a static text on the display, for X amount of seconds
  (or indefinitely, if X is 0)

__PARAM name value__
  sent by server to set the device parameter 'name' to 'value'

__OTP password__
  server sends pairing password to be shown on display (see section OTP for details)

__OTP_ACK__
  server notifies the device about successfull pairing

__DATA_TIMEOUT X__
  sets the timeout for not receiving an update, upon which reconnect is forced

__GET_PARAMS__
  server asks the device to send all the settings (excluding WiFi credentials)

__NEW_SETTINGS_LOADED__
  server notifies the device about device settings being changed (via web interface or other means)

__HB__
  heartbeat, periodically sent by server to keep the connection alive

### Commands sent by client

__HELLO modelnumber uuid version firmwareMD5__
  greets the server and sends hardware model, uuid, firmware version and MD5 checksum

__OTP_REQ__
  client requests OTP pairing

__PARAM name value__
  sends to server 'value' of parameter 'name'

__DIAG x y__
  diagnostics data, currently sendiong 'last_reset_reason', 'last_reset_info', 'data_timeout_received'

Example of typical communication ('S:' is server, 'C:' is client):
(Device connected to url: wss://ticker.cryptoclock.net:443/?uuid=e589bc6c-41c5-49f1-935f-e05cf28a6103)

```
S: ; Welcome client e589bc6c-41c5-49f1-935f-e05cf28a6103
C: ;HELLO 3DA0100 e589bc6c-41c5-49f1-935f-e05cf28a6103 1.0.0 831457cc7f8787cd9d1d1af8fe47a250
S: ; Ticker subscribed to bitfinex/btc/usd
S: ;ATH=11775.0
S: 4554.4
C: ;PARAM brightness 1
C: ;PARAM clock_interval 30
C: ;PARAM clock_mode 0
C: ;PARAM font 0
C: ;PARAM rotate_display 1
C: ;PARAM ticker_url wss://ticker.cryptoclock.net:443/
C: ;PARAM timezone 0
C: ;PARAM update_url update.cryptoclock.net
C: ;DIAG last_reset_reason External System
C: ;DIAG last_reset_info Fatal exception:0 flag:6 (EXT_SYS_RST) epc1:0x00000000 epc2:0x00000000 epc3:0x00000000 excvaddr:0x00000000 depc:0x00000000
S: 4567.1
...
S: 4565.0
C: ;HB
S: 4565.1
S: ;HB
S: ;MSG Greetings to all cryptoclock users!
S: 4565.7
...
```

Firmware update
----------------
Upon startup (or when triggered by a server command), the device will check the specified update server for a firmware update.
When update is found, it will start downloading and verifying the new firmware, during which the device may become unresponsive for few minutes.
It is recommended to not disconnect or power-off the device during update, however it should be safe to do so.
Upon sucessfull update, the device is restarted.

The official firmware server is available at http://update.cryptoclock.net.

Providing custom firmware update server:
-----------------------------------------
Make standard http server that will respond to url _/esp/update_.
(Update over https is not available due to ESP8266 RAM limitations.)

The device will connect to the url provided with parameters 'md5' and 'model' specifying current firmware checksum and model number,
e.g. 'http://someserver.org/esp/update?md5=e4d909c290d0fb1ca068ffaddf22cbd0&model=3DA0100'.
The server should respond with http code either '304' if no update is required, or '200' followed by firmware image as http data.

Setup and Building
-------------------
The firmware is using Arduino framework and libraries.
The build system is preconfigured for Platformio, which exists as a plugin into VScode and Atom editors.

1. Install VScode or Atom editor
2. Install Platformio extension into the editor
3. Point the editor to this folder
4. Customize configuration (see below)
5. Start Platformio build command
6. Start Platformio upload command to upload the firmware over USB into the device
7. Start Platfomio monitor command to see debug output from the connected device

Configuration
--------------
First, check file _platformio.ini_ for any configuration related to the device (COM port speed etc.),
build flags etc. You may enable additional debugging output from various arduino library systems using
the provided -DDEBUG_* build flags, however note that this may cause the device to run out of available RAM
and restart.

Depending on the display attached, pins used and other hardware configuration, when building, set the environment variable _X\_MODEL\_NUMBER_ to
one of the models listed in src/config_model.hpp, or create or customize your own.

The default model when not specified is _3DA0100_, consisting of 32x8 LED matrix with MAX7219 driver, connected to pins D5-D7, and optional
gyroscope module MPU6050 connected via I2C on pins D1/D2.

Extending the firmware
-----------------------
Adding new display driver library to the firmware can be done by making class inheriting from class Display::DisplayT,
overloading the defined virtual methods with custom ones, and adding initialization code into setupDisplay() function in main.cpp.
Check all setup*() functions in main.cpp for customizing the default values, menu items and all other aspects of the firmware.

Used Libraries from repository
-------------------------------
* [WebSockets](https://travis-ci.org/Links2004/arduinoWebSockets)
* [NtpClientLib](https://github.com/gmag11/NtpClient)
* [ESP8266TrueRandom](https://github.com/marvinroger/ESP8266TrueRandom) - UUID generation
* [I2Cdevlib-MPU6050](https://github.com/jrowberg/i2cdevlib.git) - i2c library for controlling MPU6050 gyroscope/accelerometer
* [TM1637](https://github.com/avishorp/TM1637) - driver for TM1637 7-segment LED displays
* [Time](http://playground.arduino.cc/code/time) - dependency of NtpClientLib
* [FastLED](http://fastled.io) - dependency of Lixie-Arduino

Modified Libraries used
------------------------
* [U8g2](https://github.com/olikraus/u8g2) - driver library for multiple displays
    changes: support for 6-panel max7219 display configuration, waiting for upstream addition

* [WiFiManager](https://github.com/tzapu/WiFiManager) - AP captive portal to configure device over WiFi
    changes: customized html and css for captive portal, fixes to storing and deleting AP credentials

* [Lixie-Arduino](https://github.com/connornishijima/Lixie-arduino) - driver for Lixie displays
    changes: changed template instantiation of the underlying FastLED library so we don't run out of RAM

Licence
--------
Licensed under GNU GPL (version 2 or any later vrsion) license (see file LICENSE for details)

Copyright (C) 2018 www.cryptoclock.net <info@cryptoclock.net>
