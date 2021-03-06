///// Includes
#include <Arduino.h>
#include "time.h"
// Webserver
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// SPIFFS
#include "SPIFFS.h"
// Own libraries
#include <Website.hpp>
#include <Neopixelstrip.hpp>
#include "definitions.h"
// Library is self included in /include and /source cause there was an error when the library was taken with platform.ini
#include "avdweb_Switch.h"

/* TODOS
- delay_between_animations aufloesen (evtl)
- evtl reset in die Neopixelstrip-Funktionen, sodass die static-Variablen nach einer Zeit zurueckgesetzt werden, damit bei
  erneuter Ausfuehrung einer Animation nicht der Stand vor zb 5h verwendet wird, sondern die Animation wirklich beim Initialzustand beginnt
*/

// Website
Website myWebsite = Website();

// Neopixel
const uint8_t number_leds = 143;
const uint8_t neopixel_pin = 4;
Neopixelstrip myStrip = Neopixelstrip(number_leds, neopixel_pin, &myWebsite);

// Wifi
unsigned long timestamp_wifi_connection = 0;
unsigned long interval_wifi_connection = 60000;

// Time
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
uint8_t month = 0;
uint8_t hour = 0;
unsigned long timestamp_time_update = 0;
unsigned long get_new_time_interval = 900000;
unsigned long duration_of_animation_before_turned_off = 3600000; // [ms]
const uint16_t delay_between_animations = 2;                    // 25;

// PIR Sensor - motion detector
const uint8_t pir_pin_Martin = 26;
const uint8_t pir_pin_Fussteil = 25;
const uint8_t pir_pin_Nadja = 33;
//PIRs implemented as buttons
Switch pir_martin = Switch(pir_pin_Martin, INPUT, HIGH, 10, 200, 250, 10);
Switch pir_fussteil = Switch(pir_pin_Fussteil, INPUT, HIGH, 10, 200, 250, 10);
Switch pir_nadja = Switch(pir_pin_Nadja, INPUT, HIGH, 10, 200, 250, 10);
// behavior variables for PIRs
uint8_t last_triggered_pir_module = 0; // 1=Martin, 2=Fuß, 3=Nadja
unsigned long timestamp_pir_module_triggered = millis();
#ifndef TEST_MODE
  unsigned long duration_lightning_pir_triggered = 45000; // [ms]
#else
  unsigned long duration_lightning_pir_triggered = 7000;
#endif
// more PIR flags 
bool blocked_pir_modules_because_it_is_day = false;
bool blocked_pir_modules_because_animation_is_active = false;
bool light_triggered_due_to_pirs = false;

// interruption due to PIR sensors occurred?
bool interruptionOccurred = false;
// first entry to a certain (= rgb, white or puff) state? ==> call function, otherwise do not keep calling the same function w/o any effect
bool first_entry_to_state = true;
// Flag that for clearing the strip ==> if true, strip will be cleared!
bool clear_strip = false;

// Buttons
const uint8_t pin_button_next_animation_martin = 15;
const uint8_t pin_button_white_light_martin = 17;
const uint8_t pin_button_next_animation_nadja = 14;
const uint8_t pin_button_white_light_nadja = 12;
// deglitch: make buttons more robust against noise pulses ==> during deglitchPeriod the input of a button has to be the same so that the callback is executed
const int deglitchPeriod = 20; // [ms]
Switch button_next_anim_martin = Switch(pin_button_next_animation_martin, INPUT_PULLUP, LOW, 50, 300, 250, deglitchPeriod);
Switch button_white_anim_martin = Switch(pin_button_white_light_martin, INPUT_PULLUP, LOW, 50, 300, 250, deglitchPeriod);
Switch button_next_anim_nadja = Switch(pin_button_next_animation_nadja, INPUT_PULLUP, LOW, 50, 300, 250, deglitchPeriod);
Switch button_white_anim_nadja = Switch(pin_button_white_light_nadja, INPUT_PULLUP, LOW, 50, 300, 250, deglitchPeriod);

///////////////////////////// Webserver
// Slider keyword
const char *PARAM_INPUT = "value";
// Switches keywords
const char *PARAM_INPUT_1 = "output";
const char *PARAM_INPUT_2 = "state";
// Wifi keys
const char *ssid = "FRITZ!Box 6490 Cable 2_4 GHz";
const char *password = "46782964961090263327";
// AsyncWebServer-object
AsyncWebServer server(80);

// function declarations
void actionsOnSwitchMoves(String, String);
void pir_triggered();
void callback_button_next_animation(void* s);
void callback_button_white_light(void* s);
#ifndef TEST_MODE
  void get_Local_Time();
#endif
void giveTimestampOfTriggeredPirModule();

// Replaces placeholder on webpage
String processor(const String &var)
{
  if (var == "SLIDERVALUE_ROT")
  {
    return myWebsite.get_sliderRot();
  }
  else if (var == "SLIDERVALUE_GRUEN")
  {
    return myWebsite.get_sliderGruen();
  }
  else if (var == "SLIDERVALUE_BLAU")
  {
    return myWebsite.get_sliderBlau();
  }
  else if (var == "BUTTONPLACEHOLDER")
  {
    String buttons = "";
    buttons += "<h4>Animation an/aus </h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"1\" " + myWebsite.StateOfSwitches("animation_active") + "><span class=\"slider2\"></span></label>";
    buttons += "<h4>Bewegungsmelder blockiert </h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"2\" " + myWebsite.StateOfSwitches("pir") + "><span class=\"slider2\"></span></label>";
    buttons += "<h5>PIR Modules triggered since last reboot: " + String(myWebsite.get_pirs_triggered_since_last_reboot()) + "</h5>";
    return buttons;
  }
  return String();
}

// Replaces placeholder with button section in web page
String processor2(const String &var)
{
  if (var == "LISTPLACEHOLDER")
  {
    return myWebsite.get_list_timestamps_pir_triggered();
  }
  return String();
}

///////////////////////////////////////////////////////////
// PIR 1 (Martin) triggered function
void lightning_pir_martin_moduletriggered(void* s)
{
    if (myWebsite.get_blocked_pir_modules() || blocked_pir_modules_because_it_is_day || blocked_pir_modules_because_animation_is_active || light_triggered_due_to_pirs)
    {
      // dont do anything
    }
    else
    {
      // Set bool variable to true ==> LED Strip is enabled in the loop method
      #ifndef BLOCK_PIR_MARTIN
        interruptionOccurred = true;
      #endif

      last_triggered_pir_module = 1;
    }
}

// PIR 2 (Fussteil) triggered function
void lightning_pir_fussteil_moduletriggered(void* s)
{
    if (myWebsite.get_blocked_pir_modules() || blocked_pir_modules_because_it_is_day || blocked_pir_modules_because_animation_is_active || light_triggered_due_to_pirs)
    {
      // dont do anything
    }
    else
    {
      // Set bool variable to true ==> LED Strip is enabled in the loop method
      #ifndef BLOCK_PIR_FUSSTEIL
        interruptionOccurred = true;
      #endif

      last_triggered_pir_module = 2;
    }
}

// PIR 3 (Nadja) triggered function
void lightning_pir_nadja_moduletriggered(void* s)
{
    if (myWebsite.get_blocked_pir_modules() || blocked_pir_modules_because_it_is_day || blocked_pir_modules_because_animation_is_active || light_triggered_due_to_pirs)
    {
      // dont do anything
    }
    else
    {
      // Set bool variable to true ==> LED Strip is enabled in the loop method
      #ifndef BLOCK_PIR_NADJA
        interruptionOccurred = true;
      #endif

      last_triggered_pir_module = 3;
    }
}

/*
button callback function to switch to next animation
1. if no animation is active ==> activate animation
2. else ==> switch to next animation
*/
void callback_button_next_animation(void *s)
{
  if (!myWebsite.get_animation_active())
  {
    // set animation to active
    actionsOnSwitchMoves("1", "1");
    // myWebsite.set_animation_active(true);
    myWebsite.update_selected_animation("rgb"); // if no animation is active, always start with rgb
    // blocked_pir_modules_because_animation_is_active = true;
  }
  else
  {
    myWebsite.select_next_animation();
    blocked_pir_modules_because_animation_is_active = true;
  }
  first_entry_to_state = true;
  #ifdef DEBUG_MODE
    Serial.println("  BUTTON - NEXT anim");
  #endif
}

/*
button callback function to enable white light
1. if another animation is active ==> change to white light
2. if white light is already active ==> deactive white light and clear strip
3. no animation active ==> activate white light
*/
void callback_button_white_light(void* s)
{
  #ifdef DEBUG_MODE
    Serial.println("  BUTTON - WHITE anim");
  #endif
  if (myWebsite.get_animation_active() && myWebsite.get_selected_animation() != WEISS) // Animation active but not white light
  {
    myWebsite.update_selected_animation("weiss"); // just change the selected animation so that white light will be activated in loop()
    first_entry_to_state = true;
  }
  else if (myWebsite.get_animation_active()) // white light active ==> clear the strip
  {
    actionsOnSwitchMoves("1", "0");
    clear_strip = true;
    blocked_pir_modules_because_animation_is_active = false;
  }
  else // no animation active ==> enable white light
  {
    myWebsite.update_selected_animation("weiss");
    actionsOnSwitchMoves("1", "1");
    first_entry_to_state = true;
  }
}


void setup()
{
  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
#ifdef DEBUG_MODE
    Serial.println("An Error has occurred while mounting SPIFFS");
#endif
    return;
  }

#if defined(DEBUG_MODE) || defined(LOOP_TIME)
  Serial.begin(115200);
#endif

  // Webserver
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
#ifdef DEBUG_MODE
    Serial.println("Connecting to WiFi..");
#endif
  }
#ifdef DEBUG_MODE
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());
#endif

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index_root.html", String(), false, processor); });

  // Route to load the javascript file
  server.on("/javascript.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/javascript.js", "text/javascript"); });
  // Route to load style_root.css file
  server.on("/style_root.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/style_root.css", "text/css"); });
  // Route to load style_timestamps_pir.css file
  server.on("/style_timestamps_pir.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/style_timestamps_pir.css", "text/css"); });

  // List of timestamps of triggered PIR modules
  server.on("/ListTriggeredPIRs", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index_timestamps_pir.html", String(), false, processor2); });

  // Send a GET request to <ESP_IP>/sliderRot?value=<inputMessage>
  server.on("/sliderRot", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String inputMessage;
              // GET input1 value on <ESP_IP>/sliderRot?value=<inputMessage>
              if (request->hasParam(PARAM_INPUT))
              {
                inputMessage = request->getParam(PARAM_INPUT)->value();
                myWebsite.set_sliderRot(inputMessage);
                myStrip.rgb_live_update();
              }
              else
              {
                inputMessage = "No message sent";
              }
              request->send(200, "text/plain", "OK"); });

  // Send a GET request to <ESP_IP>/sliderGruen?value=<inputMessage>
  server.on("/sliderGruen", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String inputMessage;
              // GET input1 value on <ESP_IP>/sliderGruen?value=<inputMessage>
              if (request->hasParam(PARAM_INPUT))
              {
                inputMessage = request->getParam(PARAM_INPUT)->value();
                myWebsite.set_sliderGruen(inputMessage);
                myStrip.rgb_live_update();
              }
              else
              {
                inputMessage = "No message sent";
              }
              request->send(200, "text/plain", "OK"); });

  // Send a GET request to <ESP_IP>/sliderBlau?value=<inputMessage>
  server.on("/sliderBlau", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String inputMessage;
              // GET input1 value on <ESP_IP>/sliderBlau?value=<inputMessage>
              if (request->hasParam(PARAM_INPUT))
              {
                inputMessage = request->getParam(PARAM_INPUT)->value();
                myWebsite.set_sliderBlau(inputMessage);
                myStrip.rgb_live_update();
              }
              else
              {
                inputMessage = "No message sent";
              }
              request->send(200, "text/plain", "OK"); });

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String inputMessage1;
              String inputMessage2;
              // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
              if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2))
              {
                inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
                inputMessage2 = request->getParam(PARAM_INPUT_2)->value();

                // function that performs the desired actions
                actionsOnSwitchMoves(inputMessage1, inputMessage2);
              }
              else
              {
                inputMessage1 = "No message sent";
                inputMessage2 = "No message sent";
              }
              request->send(200, "text/plain", "OK"); });

  server.on("/selection_list", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String inputMessage;
              if (request->hasParam(PARAM_INPUT))
              {
                inputMessage = request->getParam(PARAM_INPUT)->value();
                myWebsite.update_selected_animation(inputMessage);
                first_entry_to_state = true; // guarantee that animation is performed at least once
                myStrip.clear_show();  // clear strip, if a new animation is chosen
              }
              else
              {
                inputMessage = "No message sent";
              }
              request->send(200, "text/plain", "OK"); });

  // Reboot ESP32 if corresponding button is pushed
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              request->send(200, "text/plain", "ok");
#ifdef DEBUG_MODE
                Serial.println("ESP32 wird neugestartet.");
#endif
              ESP.restart(); });

  // Clear strip if corresponding button is pushed
  server.on("/clearstrip", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              request->send(200, "text/plain", "ok");
#ifdef DEBUG_MODE
                Serial.println("Strip wird gecleared!");
#endif
              myStrip.clear_show(); });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "ok"); });

  // Start server
  server.begin();

  // Neopixel
  myStrip.begin();
  myStrip.clear_show();
  myStrip.setBrightness(100);

  // Call function at the beginning so that time is known for blocking PIR-Modules
  #ifndef TEST_MODE
    get_Local_Time();
  #endif
  
  // PIR sensors
  pir_martin.setPushedCallback(&lightning_pir_martin_moduletriggered, nullptr);
  pir_fussteil.setPushedCallback(&lightning_pir_fussteil_moduletriggered, nullptr);
  pir_nadja.setPushedCallback(&lightning_pir_nadja_moduletriggered, nullptr);

  // setup buttons
  button_next_anim_martin.setPushedCallback(&callback_button_next_animation, nullptr);
  button_white_anim_martin.setPushedCallback(&callback_button_white_light, nullptr);
  button_next_anim_nadja.setPushedCallback(&callback_button_next_animation, nullptr);
  button_white_anim_nadja.setPushedCallback(&callback_button_white_light, nullptr);
}

#ifdef LOOP_TIME
  int counter = 0;
  const int num_iterations = 1000;
  unsigned long timestamp_start = millis();
#endif

// Loop function
void loop()
{
  #ifdef LOOP_TIME
    if (counter >= num_iterations)
    {
      unsigned long now = millis();
      unsigned long time_took = now - timestamp_start;
      timestamp_start = now;
      counter = 0;
      Serial.print("Average loop time in ms/1000cycles: ");
      Serial.println(time_took);
    }
    counter++;
  #endif

  // Read PIR states
  pir_martin.poll();
  pir_fussteil.poll();
  pir_nadja.poll();

  // Read button states
  button_next_anim_martin.poll();
  button_white_anim_martin.poll();
  button_next_anim_nadja.poll();
  button_white_anim_nadja.poll();


  // Clear strip if a new animation was chosen betwenn two loop iterations
  if (myWebsite.get_selected_animation() != myWebsite.get_previous_animation())
  {
    myWebsite.update_previous_animation();
    myStrip.clear_show();
    #ifdef DEBUG_MODE
      Serial.println("cleared strip cause selected animation changed between two loop interations");
    #endif
  }
  
  if ((unsigned long)(millis() - timestamp_wifi_connection) > interval_wifi_connection)
  {
    timestamp_wifi_connection = millis();
    if (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, password);
    }
  }

  // checks if interruptionOccurred is true and enables all leds in white when true
  pir_triggered();

  // check if new timestamp should be recorded
  #ifndef TEST_MODE
  if ((unsigned long)(millis() - timestamp_time_update) > get_new_time_interval)
  {
    timestamp_time_update = millis();
    get_Local_Time();
  }
  #endif

  // Main "state"-machine
  if (myWebsite.get_animation_active())
  {
    // Check if animation is active for too long and if so, disable animation and clear strip
    if ((unsigned long)(millis() - myWebsite.get_timestamp_animation_active()) > duration_of_animation_before_turned_off)
    {
      clear_strip = true;
      blocked_pir_modules_because_animation_is_active = false;
      myWebsite.set_animation_active(false);
    }

    else if (myWebsite.get_selected_animation() == RGB)
    {
      if (first_entry_to_state)
      {
        first_entry_to_state = false;
        myStrip.clear_show();
        myStrip.rgb_live_update();
      }
    }

    else if (myWebsite.get_selected_animation() == WEISS)
    {
      if (first_entry_to_state)
      {
        first_entry_to_state = false;
        myStrip.clear_show();
        myStrip.weisseBeleuchtung();
#ifdef DEBUG_MODE
        Serial.println("Weiss wurde gesetzt.");
#endif
      }
    }

    // case 4
    else if (myWebsite.get_selected_animation() == COLOR_WIPE)
    {
      myStrip.colorWipeAnimation();
#ifdef DEBUG_MODE
      Serial.println("Color Wipe einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    // case 5
    else if (myWebsite.get_selected_animation() == THEATER_CHASE)
    {
      myStrip.theaterChaseAnimation();
#ifdef DEBUG_MODE
      Serial.println("Theater Chase einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    // case 6
    else if (myWebsite.get_selected_animation() == RAINBOW)
    {
      myStrip.rainbow(20);
#ifdef DEBUG_MODE
      Serial.println("Rainbow einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    // case 7
    else if (myWebsite.get_selected_animation() == THEATER_CHASE_RAINBOW)
    {
      myStrip.theaterChaseRainbow(50);
#ifdef DEBUG_MODE
      Serial.println("Theater Chase Rainbow einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    // case 8
    else if (myWebsite.get_selected_animation() == PUFF_MODUS)
    {
      if (first_entry_to_state)
      {
#ifdef DEBUG_MODE
        Serial.println("Puff-Modus aktiviert.");
#endif
        first_entry_to_state = false;
        myStrip.clear_show();
        myStrip.puffModus();
      }
    }

    // case 9
    else if (myWebsite.get_selected_animation() == CYLON_BOUNCE)
    {
      myStrip.CylonBounce(0xff, 0, 0, 4, 10, 50);
#ifdef DEBUG_MODE
      Serial.println("Cylon Bounce einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    // case 10
    else if (myWebsite.get_selected_animation() == SPARKLE)
    {
      myStrip.Sparkle(0xff, 0xff, 0xff, 0);
#ifdef DEBUG_MODE
      Serial.println("Sparkle einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    // case 11
    else if (myWebsite.get_selected_animation() == RUNNING_LIGHTS)
    {
      myStrip.RunningLights(0xff, 0xff, 0x00, 50);
#ifdef DEBUG_MODE
      Serial.println("Running Lights einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    // case 12
    else if (myWebsite.get_selected_animation() == FIRE)
    {
      myStrip.Fire(55, 120, 15);
#ifdef DEBUG_MODE
      Serial.println("Fire einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    // case 13
    else if (myWebsite.get_selected_animation() == BOUNCING_BALLS)
    {
      myStrip.BouncingBalls(0, 0xff, 0);
#ifdef DEBUG_MODE
      Serial.println("Bouncing Balls einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    // case 14
    else if (myWebsite.get_selected_animation() == BOUNCING_BALLS_COLORED)
    {
      byte colors[3][3] = {{0xff, 0, 0},       // red
                           {0xff, 0xff, 0xff}, // white
                           {0, 0, 0xff}};      // blue
      myStrip.BouncingColoredBalls(colors);
#ifdef DEBUG_MODE
      Serial.println("Bouncing Balls Colored einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    // case 15
    else if (myWebsite.get_selected_animation() == METEOR_RAIN)
    {
      myStrip.meteorRain(0xff, 0xff, 0xff, 10, 64, true, 30);
#ifdef DEBUG_MODE
      Serial.println("Meteor Rain einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    // case 16
    else if (myWebsite.get_selected_animation() == FIRE_ANIMATION)
    {
      myStrip.fireAnimation();
#ifdef DEBUG_MODE
      Serial.println("Fire Animation einmal ausgefuehrt.");
#endif
      delay(delay_between_animations);
    }

    else
    {
#ifdef DEBUG_MODE
      Serial.println("Fail");
#endif
    }
  }

  // clear strip
  if (clear_strip)
  {
#ifdef DEBUG_MODE
    Serial.println("Strip wurde in loop() gecleared!");
#endif

    myStrip.clear_show();
    clear_strip = false;
    first_entry_to_state = true;
  }
}

/*
Function that performs the corresponding actions if a switch is activated/deactivated on the website
   function is also used for the physical buttons attached to the bed frame
*/
void actionsOnSwitchMoves(String switchIdString, String switchValueString)
{
  int switchId = switchIdString.toInt();
  int switchValue = switchValueString.toInt();

  myWebsite.updateWebsite(switchId);

  switch (switchId)
  {
  // animation active
  case 1:
    if (switchValue)
    {
#ifdef DEBUG_MODE
      Serial.println("Animation activated.");
#endif
      blocked_pir_modules_because_animation_is_active = true;
      light_triggered_due_to_pirs = false;
    }
    else
    {
#ifdef DEBUG_MODE
      Serial.println("Animation turned off.");
#endif
      clear_strip = true;
      blocked_pir_modules_because_animation_is_active = false;
    }
    break;

  // PIR-sensors are blocked
  case 2:
    if (switchValue)
    {
#ifdef DEBUG_MODE
      Serial.println("PIR blocked.");
#endif
    }
    else
    {
#ifdef DEBUG_MODE
      Serial.println("Pir blocked not any more.");
#endif
    }
    break;

  // Default - should never be called
  default:
#ifdef DEBUG_MODE
    Serial.println("Default in switch statement in Fkt. 'actionsOnSwitchMoves' wurde aufgerufen.");
#endif
    break;
  }
}

/*
Continiously checking the flag "interruptionOccurred" which is triggered if a PIR was triggered by a movement
*/
void pir_triggered()
{
  if (interruptionOccurred)
  {
#ifdef DEBUG_MODE
    Serial.println("PIR Modul wurde getriggered.");
#endif
    timestamp_pir_module_triggered = millis();

    myStrip.beleuchtungPIRtriggered();
    interruptionOccurred = false;

    light_triggered_due_to_pirs = true;
  }

  // clear strip if duration of active white light due to a PIR has passed and no other animation is active
  if (((unsigned long)(millis() - timestamp_pir_module_triggered) > duration_lightning_pir_triggered) && light_triggered_due_to_pirs)
  {
    interruptionOccurred = false;

#ifdef DEBUG_MODE
    Serial.println("PIR Modul wurde getriggered - vorbei.");
#endif

    giveTimestampOfTriggeredPirModule();
    myWebsite.increase_by_one_pirs_triggered_since_last_reboot();
    light_triggered_due_to_pirs = false;

    // dont clear strip if an animation/rgb light was enabled during the time the light is active because a pir module was triggered
    if (!blocked_pir_modules_because_animation_is_active)
    {
      myStrip.clear_show();
    }
  }
}

/*
get time in order to block pir modules when it is day
*/
#ifndef TEST_MODE
void get_Local_Time()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
#ifdef DEBUG_MODE
    Serial.println("Failed to obtain time");
#endif

    return;
  }
  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  int hour = atoi(timeHour);

  char monthint[3];
  strftime(monthint, 3, "%m", &timeinfo);
  int month = atoi(monthint);

  // summer months
  if ((month > 3) && (month < 9))
  {
    if ((hour >= 8) && (hour <= 17))
    {
      blocked_pir_modules_because_it_is_day = true;
    }
    else
    {
      blocked_pir_modules_because_it_is_day = false;
    }
  }
  // winter months
  else
  {
    if ((hour >= 9) && (hour <= 16))
    {
      blocked_pir_modules_because_it_is_day = true;
    }
    else
    {
      blocked_pir_modules_because_it_is_day = false;
    }
  }
}
#endif

/*
Get timestamp of last triggered PIR module and extend corresponding String for Website
*/
void giveTimestampOfTriggeredPirModule()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return;
  }
  char timestamp[40];
  strftime(timestamp, 40, "%A, %B %d %Y %H:%M:%S", &timeinfo);

  myWebsite.append_list_timestamps_pir_triggered("<p>" + String(timestamp) + " an Modul " + String(last_triggered_pir_module) + "</p>");
}