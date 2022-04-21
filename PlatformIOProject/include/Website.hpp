#pragma once
#include <Arduino.h>
#include "definitions.h"

class Website
{
    private:
    // Attributes
        bool blocked_pir_modules = true;
        bool animation_active = false;

        String sliderRot = "136";
        String sliderGruen = "0";
        String sliderBlau = "255";
        
        String list_timestamps_pir_triggered = "";
        int pirs_triggered_since_last_reboot = 0;
        
        unsigned long timestamp_animation_active = 0;   // Timestamp to check whether animation is running for too long ==> if so, strip is cleared in main.cpp

        SELECTED_ANIMATION selected_animation = RGB;

    public:
        // con-/destructor
        Website();
        ~Website();

        // getter and setter-methods
        bool get_blocked_pir_modules();
        void set_blocked_pir_modules(bool);
        void toggle_blocked_pir_modules();
        bool get_animation_active();
        void set_animation_active(bool);
        void toggle_animation_active();

        String get_sliderRot();
        int get_sliderRot_int();
        void set_sliderRot(String);
        String get_sliderGruen();
        int get_sliderGruen_int();
        void set_sliderGruen(String);
        String get_sliderBlau();
        int get_sliderBlau_int();
        void set_sliderBlau(String);

        String get_list_timestamps_pir_triggered();
        void append_list_timestamps_pir_triggered(String to_append);
        int get_pirs_triggered_since_last_reboot();
        void increase_by_one_pirs_triggered_since_last_reboot();
        unsigned long get_timestamp_animation_active();
        void set_timestamp_animation_active(unsigned long);

        SELECTED_ANIMATION get_selected_animation();

        // methods
        void updateWebsite(uint8_t);
        void update_selected_animation(String new_animation);
        void select_next_animation();
        String StateOfSwitches(String mode);
};