# NeopixelBett
## General information
Nepixels are mounted on the bed frame such that the Neopixels are facing the floor. This results in a passive illumination.
The Neopixles are controlled by an ESP32 which has the following interfaces:
1. PIR-modules which are triggered by living creatures,
2. physical buttons which are mounted on the bedframe and
3. a webinterface which can be accessed by smartphone, tablet or PC.

## Explanations for interacting with the illumination system
If a PIR module is triggered, a white illumination is activated so that one can see the room (useful at night).
The physical buttons allow the user to physically control the Neopixels. Due to the limited number of buttons, it is not possible to fully control the Neopixels compared to controlling them via the webinterface. However, every animation can be activated by using these buttons.
The webinterface enables full control of the Neopixels, including mixing your own color by adjusting the RGB values or activating one of 10+ animations.

## Wiring diagram
The wiring diagram can be found in ./Fritzing/.

## 3D-printed components
These files can be found in ./3D-Drucker/.
## Code
The source code can be found in ./BeleuchtungBettPlatformIOProjekt!
