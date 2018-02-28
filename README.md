# Simulator Collective Firmware

This repo contains firmware for a DIY simulator collective. The intent is not to commercialize, but rather to design something usable that an interested hobbyist with a relatively limited fabrication setup (ie: yours truly) can build and customize. Because I'm an aviation simulation enthusiast who likes physically correct (or as close as I can get for affordable prices) switchology, the "head" or "button box" component is designed to be interchangeable to support different airframes' controls. So far there are two software components:

## Collective Base

This firmware is intended to run on a Teensy 3.x, with the Joystick USB profile. It uses a contactless hall effect sensor to measure the collective arm's angle, and integrates input from the interchangeable button box. In future hardware/software revisions I hope to connect a twist throttle and collective brake as well.

## Collective Button Box

This firmware interfaces switches with the Base as an I2C slave. I probably could've used 74HC165 shift registers, like the Thrustmaster Cougar/Warthog, and just put all the brains in the Collective Base, but I like the idea of being able to support joystick axes (eg: for the spotlight slew control on some DCS helicopters) and for the box to be able to report things like its intended range of motion to the base.