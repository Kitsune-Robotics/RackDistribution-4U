# Rack Distribution 4U firmware

Built with the [Pico SDK](https://github.com/raspberrypi/pico-sdk) for an RP2040.
FreeRTOS plus the WS2812 PIO driver for the light panel.

## To build

`pico_sdk_import.cmake` will pull the SDK if you don't already have `PICO_SDK_PATH`.

```shell
git submodule update --init Firmware/lib/FreeRTOS-Kernel
mkdir build
cd build
cmake ..
make
```

## To load

```shell
picotool load -f -x rack_distribution.uf2
```

Or drop the uf2 on the Pico in BOOTSEL.
