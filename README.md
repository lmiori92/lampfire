# lampfire
A simple but effective fire effect with incadescent lamps

## Docs
The doc folder contains the circuit schematic and some pictures.

## Source
The src folder contains the C source code to be compiled with avr-gcc.

## Power supply
+5V (any USB charger should be fine)

## Output specification
The output PWM frequency is set to 31Hz. The flicker of the low frequency helps to enhance the effect.

The output swings from a minimum duty cycle (~20%) to full-on (100%).

Outputs are mapped to PB0 and PB1. They are independently set with a different random duty cycle at each application cycle.

## Microcontroller
The attiny85 microcontroller is used but I think all attinyXX are compatible or compatible with minor changes.

The clock is set to internal 8MHz

### Fuse configuration
-U lfuse:w:0xe2:m -U hfuse:w:0xd4:m -U efuse:w:0xff:m

Using usbasp programmer:

    avrdude -p attiny85 -c usbasp -U lfuse:w:0xe2:m -U hfuse:w:0xd4:m -U efuse:w:0xff:m -B 10
    Note: -B switch is used to allow working commnication on slow internal default clock (1MHz)

## Compiler
avr-gcc
using your IDE of choice; I have used Eclipse with the AVR plugins.

## Changelog
28.11.2020 - Initial version with 2 channels

## Schematic
![Alt text](/doc/Schematic.jpg | width=350)
