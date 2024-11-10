# ESP8266 Controller

## Overview

This is a simple ESP8266 controller that receives temperature measurements from a DS18B20 temperature sensor on another
ESP8266 over ESP-NOW and sends them to a PocketBase server over HTTP. You can find the client code
in [here](https://github.com/ingui-n/thermometer-client).

## Components

- ESP8266 New NodeMCU v3

## Code setup

1. Clone this repository
2. Copy `include/env.h.template` to `include/env.h`
3. Fill in the values in `include/env.h` based on comments in the file
4. Set up static IP address and other network settings for your ESP8266 in `src/main.cpp`
5. Use PlatformIO to compile and upload the code
