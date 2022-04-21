#pragma once

#include <Arduino.h>
#include <Website.hpp>
#include "definitions.h"

// Neopixel
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif


class Neopixelstrip
{
    private:
    // Attributes
        const uint8_t num_leds;
        const uint8_t neopixel_pin;
        uint8_t weisse_belechtung_brightness = 175;
        uint8_t brightness_pir_triggered = 100;

        // Fire animation attributes
        uint32_t fire_color   = 0;
        uint32_t off_color    = 0;


    public:
    // Attributes
        Adafruit_NeoPixel strip;
        // Adafruit_NeoPixel& strip;


        // Website instance
        Website *psite;


    // Constructors
        Neopixelstrip(const uint8_t, const uint8_t);
        // Neopixelstrip(Adafruit_NeoPixel& init_strip);
        Neopixelstrip(const uint8_t, const uint8_t, Website*);
        ~Neopixelstrip();

    // Auxiliary Methods
        void begin();
        void clear();
        void show();
        void clear_show();
        void fill(uint32_t, uint16_t, uint16_t);
        void setBrightness(uint8_t);
        void weisseBeleuchtung();
        void puffModus();
        void beleuchtungPIRtriggered();

    // Animation Methods
        void rgb_live_update();
        bool colorWipe(uint32_t, int);
        void colorWipeAnimation();
        bool theaterChase(uint32_t, int);
        void theaterChaseAnimation();
        void rainbow(int);
        void theaterChaseRainbow(int);
        void CylonBounce(byte, byte, byte, int, int, int);
        void Sparkle(byte, byte, byte, int);
        void showStrip();
        void setPixel(int, byte, byte, byte);
        void setAll(byte, byte, byte);
        void RunningLights(byte, byte, byte, int);
        void Fire(int, int, int);
        void BouncingBalls(byte, byte, byte, int);
        void setPixelHeatColor(int, byte);
        void BouncingColoredBalls(int, byte[][3]);
        void meteorRain(byte, byte, byte, byte, byte, boolean, int);
        void fadeToBlack(int, byte);
        void fireAnimation();
        void fireAnimationDraw();
        void fireAnimationAddColor(uint8_t position, uint32_t color);
        void fireAnimationSubstractColor(uint8_t position, uint32_t color);
        uint32_t fireAnimationBlend(uint32_t color1, uint32_t color2);
        uint32_t fireAnimationSubstract(uint32_t color1, uint32_t color2);
        
};  