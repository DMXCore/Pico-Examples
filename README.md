# Pico-Examples
Example firmware for the DMX Core Pico and Pico 2.

### Examples ###

* Blink - Blinks the various LEDs and outputs
* DmxRepeater - Reads DMX frames from port A, outputs to port B
* DmxTriggerOutput - Reads DMX frames from port A, channel 1-4 drives digital output 1-4
* InputTriggerDmx - Outputs DMX frames on port A. Digital input 1 sets all DMX channels to either 255 or 0


### Build Instructions (Windows) ###

* Open the `Pico - Developer Command Prompt'. Navigate to the folder of the example.
* Run `build.bat`, which will make the example using CMake.
* Hold down BOOTSEL and power the DMX Core Pico/Pico2 to a power source/USB.
* Copy the xxxxxx.uf2 file from the `build` folder under the example to the RPi-Boot volume
* Enjoy

Note that you can also flash the Pico/Pico2 using a Raspberry Pi DebugProbe connected to the SWD debug port, however it's harder to reach with the enclosure on.

Documentation: https://docs.dmxcore.com/dmx-core-pico-2/
