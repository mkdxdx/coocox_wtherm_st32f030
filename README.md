This is a CooCox project for my weather station based on STM32F030F4P6 controller and ESP-01 wlan module.

Local weather data is acquired with BMP180 and DS18B20 sensors, while forecast is downloaded from OpenWeatherMap hence you need your own APPID for forecast to work.
You will also need to provide wireless AP connection credentials for unit to connect to internet, instructions inside main.h. 

Executable needs no less than 16k of memory on the controller (-Os option) and ~2k of RAM.
This is my first srs project on STM32 and that uses ESP-01.

inb4 "ESP-01 can do everything on its own": yes it can, but for sake of platform experience it is used solely as connectivity backend.

To-do:
 - polish code
