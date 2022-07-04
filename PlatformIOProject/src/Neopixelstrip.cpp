#include <Neopixelstrip.hpp>

Neopixelstrip::Neopixelstrip(const uint8_t _num_leds, const uint8_t _neopixel_pin) : num_leds(_num_leds), neopixel_pin(_neopixel_pin),
                                                                                     strip(this->num_leds, this->neopixel_pin, NEO_GRB + NEO_KHZ800)
{
}

Neopixelstrip::Neopixelstrip(const uint8_t _num_leds, const uint8_t _neopixel_pin, Website *website) : num_leds(_num_leds), neopixel_pin(_neopixel_pin),
                                                                                                       strip(this->num_leds, this->neopixel_pin, NEO_GRB + NEO_KHZ800),
                                                                                                       psite(website)
{
  this->fire_color = this->strip.Color(80, 35, 00);
  this->off_color = this->strip.Color(0, 0, 0);
}

Neopixelstrip::~Neopixelstrip()
{
}

// Auxiliary Methods
void Neopixelstrip::begin()
{
  this->strip.begin();
}

void Neopixelstrip::clear()
{
  this->strip.clear();
}

void Neopixelstrip::show()
{
  this->strip.show();
}

void Neopixelstrip::clear_show()
{
  this->clear();
  this->show();
}

void Neopixelstrip::fill(uint32_t c, uint16_t first, uint16_t count)
{
  this->strip.fill(c, first, count);
}

void Neopixelstrip::setBrightness(uint8_t val_brigthness)
{
  this->strip.setBrightness(val_brigthness);
}

void Neopixelstrip::weisseBeleuchtung()
{
  this->fill(this->strip.Color(this->weisse_belechtung_brightness, this->weisse_belechtung_brightness, this->weisse_belechtung_brightness), 0, 0);
  this->show();
}

void Neopixelstrip::puffModus()
{
  this->fill(this->strip.Color(230, 0, 0), 0, 0);
  this->show();
}

void Neopixelstrip::beleuchtungPIRtriggered()
{
  this->fill(this->strip.Color(this->brightness_pir_triggered, this->brightness_pir_triggered, this->brightness_pir_triggered), 0, 0);
  this->show();
}

void Neopixelstrip::rgb_live_update()
{
  if (this->psite->get_selected_animation() == RGB && this->psite->get_animation_active())
  {
#ifdef DEBUG_MODE
    Serial.printf("Live Update RGB-Werte -- Rot: %d, Gruen: %d, Blau: %d\n", this->psite->get_sliderRot_int(), this->psite->get_sliderGruen_int(), this->psite->get_sliderBlau_int());
#endif

    this->fill(this->strip.Color(psite->get_sliderRot_int(), psite->get_sliderGruen_int(), psite->get_sliderBlau_int()), 0, 0);
    this->show();
  }
}

/// Animation Methods

bool Neopixelstrip::colorWipe(uint32_t color, int wait)
{
  
  static unsigned long lastUpdate = 0;
  static uint16_t i = 0;
  unsigned long now = millis();

  // clear strip if it is the first time that the function is called after a "long" time (2 seconds)
  if ((unsigned long)(now - lastUpdate) > 2000)
  {
    this->clear_show();
  }

  if ((unsigned long)(now - lastUpdate) > wait)
  {
    lastUpdate = now;
    this->strip.setPixelColor(i, color); //  Set pixel's color (in RAM)
    this->strip.show();                  //  Update strip to match
    i++;
    if (i >= this->strip.numPixels())
    {
      i = 0;
      return true;
    }
  }
  return false;
}

void Neopixelstrip::colorWipeAnimation()
{
  static uint8_t state = 1;
  static uint32_t current_color = this->strip.Color(0, 127, 0);
  
  bool go_on;
  go_on = this->colorWipe(current_color, 50);
  if (go_on)
  {
    state++;
    switch (state)
    {
    case 1:
      current_color = this->strip.Color(0, 127, 0);
      break;
    
    case 2:
      current_color = strip.Color(127, 0, 0);
      break;
    
    case 3:
      current_color = strip.Color(0, 0, 127);
      break;
    
    default:
      break;
    }
    if (state >= 3) state = 1;
  }
}

// Theater-marquee-style chasing lights
bool Neopixelstrip::theaterChase(uint32_t color, int wait)
{
  static uint8_t b = 0;
  static uint8_t c = 0;
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  
  if ((unsigned long)(now - lastUpdate) > wait)
  {
    lastUpdate = now;
    this->strip.clear();
    this->strip.setPixelColor(c, color);
    this->strip.show();
    c += 3;
    if (c >= this->strip.numPixels())
    {
      b++;
      c = b;
    }

    if (b >= 3)
    {
      b = 0;
      c = 0;
      return true;
    }
  }
  return false;
}

void Neopixelstrip::theaterChaseAnimation()
{
  static uint8_t state = 1;
  static uint32_t current_color = this->strip.Color(0, 127, 0);
  static uint8_t counter = 0;
  
  bool go_on;
  go_on = this->theaterChase(current_color, 50);
  if (go_on)
  {
    counter++;
    if (counter >= 10)
    {
      counter = 0;
      state++;
    }

    switch (state)
    {
    case 1:
      current_color = this->strip.Color(0, 127, 0);
      break;
    
    case 2:
      current_color = strip.Color(127, 0, 0);
      break;
    
    case 3:
      current_color = strip.Color(0, 0, 127);
      break;
    
    default:
      break;
    }
    if (state >= 3) state = 1;
  }
}

// Rainbow cycle along whole strip
void Neopixelstrip::rainbow(int wait)
{
  static unsigned long lastUpdate = 0;
  static long firstPixelHue = 0;
  unsigned long now = millis();

  if ((unsigned long)(now - lastUpdate) > wait)
  {
    lastUpdate = now;
    for (int i = 0; i < this->strip.numPixels(); i++)
    { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / this->strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      this->strip.setPixelColor(i, this->strip.gamma32(this->strip.ColorHSV(pixelHue)));
    }

    this->strip.show(); // Update strip with new contents
    
    firstPixelHue += 256;
    if (firstPixelHue >= 5*65536)
    {
      firstPixelHue = 0;
    }
  }
}

// Rainbow-enhanced theater marquee
void Neopixelstrip::theaterChaseRainbow(int wait)
{
  static unsigned long lastUpdate = 0;
  static long firstPixelHue = 0;
  unsigned long now = millis();
  static uint8_t b = 0;

  if ((unsigned long)(now - lastUpdate) > wait)
  {
    lastUpdate = now;
    this->strip.clear(); //   Set all pixels in RAM to 0 (off)
    // 'c' counts up from 'b' to end of strip in increments of 3...
    for (int c = b; c < this->strip.numPixels(); c += 3)
    {
      // hue of pixel 'c' is offset by an amount to make one full
      // revolution of the color wheel (range 65536) along the length
      // of the strip (strip.numPixels() steps):
      int hue = firstPixelHue + c * 65536L / this->strip.numPixels();
      uint32_t color = this->strip.gamma32(this->strip.ColorHSV(hue)); // hue -> RGB
      this->strip.setPixelColor(c, color);                             // Set pixel 'c' to value 'color'
    }

    this->strip.show();          // Update strip with new contents
    firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    
    b++;
    if (b >= 3) b=0;
  }
}

//////////////////////////////////
// Functions, so that the codes from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/ work
void Neopixelstrip::showStrip()
{
  // NeoPixel
  this->strip.show();
}

void Neopixelstrip::setPixel(int Pixel, byte red, byte green, byte blue)
{
  // NeoPixel
  this->strip.setPixelColor(Pixel, this->strip.Color(red, green, blue));
}

void Neopixelstrip::setAll(byte red, byte green, byte blue)
{
  for (int i = 0; i < this->num_leds; i++)
  {
    this->setPixel(i, red, green, blue);
  }
  this->showStrip();
}

void Neopixelstrip::CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay)
{
  static unsigned long lastUpdate = 0;
  static int i = 0;
  unsigned long now = millis();
  static uint8_t state = 1;

  switch (state)
  {
  case 1:
    if ((unsigned long)(now - lastUpdate) > SpeedDelay)
    {
      lastUpdate = now;
      this->setAll(0, 0, 0);
      this->setPixel(i, red / 10, green / 10, blue / 10);
      for (int j = 1; j <= EyeSize; j++)
      {
        this->setPixel(i + j, red, green, blue);
      }
      this->setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
      this->showStrip();

      i++;
      if (i >= this->num_leds - EyeSize - 2)
      {
        state++;
        i = this->num_leds - EyeSize - 2;
      }
    }
    break;
  
  case 2:
    if ((unsigned long)(now - lastUpdate) > ReturnDelay)
    {
      state++;
      lastUpdate = now;
    }
    break;
  
  case 3:
    if ((unsigned long)(now - lastUpdate) > SpeedDelay)
    {
      lastUpdate = now;
      this->setAll(0, 0, 0);
      this->setPixel(i, red / 10, green / 10, blue / 10);
      for (int j = 1; j <= EyeSize; j++)
      {
        this->setPixel(i + j, red, green, blue);
      }
      this->setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
      this->showStrip();

      i--;
      if (i <= 0)
      {
        i = 0;
        state++;
      }
    }
    break;

    case 4:
      if ((unsigned long)(now - lastUpdate) > ReturnDelay)
      {
        state = 1;
        lastUpdate = now;
      }
      break;

  default:
    break;
  }
}

void Neopixelstrip::Sparkle(byte red, byte green, byte blue, int SpeedDelay)
{
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if ((unsigned long)(now - lastUpdate) > SpeedDelay)
  {
    lastUpdate = now;
  
    int Pixel = random(this->num_leds);
    this->setPixel(Pixel, red, green, blue);
    this->showStrip();

    this->setPixel(Pixel, 0, 0, 0);
  }
}

void Neopixelstrip::RunningLights(byte red, byte green, byte blue, int WaveDelay)
{
  static int Position = 0;
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  static int j = 0;

  if ((unsigned long)(now - lastUpdate) > WaveDelay)
  {
    lastUpdate = now;
    Position++; // = 0; //Position + Rate;
    for (int i = 0; i < this->num_leds; i++)
    {
      // sine wave, 3 offset waves make a rainbow!
      // float level = sin(i+Position) * 127 + 128;
      // setPixel(i,level,0,0);
      // float level = sin(i+Position) * 127 + 128;
      this->setPixel(i, ((sin(i + Position) * 127 + 128) / 255) * red,
                    ((sin(i + Position) * 127 + 128) / 255) * green,
                    ((sin(i + Position) * 127 + 128) / 255) * blue);
    }
    this->showStrip();

    j++;
    if (j >= this->num_leds * 2)
    {
      j = 0;
      Position = 0;
    }
  }
}

void Neopixelstrip::Fire(int Cooling, int Sparking, int SpeedDelay)
{
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  byte heat[this->num_leds];
  int cooldown;

  if ((unsigned long) (now - lastUpdate) > SpeedDelay)
  {
    lastUpdate = now;
    // Step 1.  Cool down every cell a little
    for (int i = 0; i < this->num_leds; i++)
    {
      cooldown = random(0, ((Cooling * 10) / this->num_leds) + 2);

      if (cooldown > heat[i])
      {
        heat[i] = 0;
      }
      else
      {
        heat[i] = heat[i] - cooldown;
      }
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = this->num_leds - 1; k >= 2; k--)
    {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' near the bottom
    if (random(255) < Sparking)
    {
      int y = random(7);
      heat[y] = heat[y] + random(160, 255);
      // heat[y] = random(160,255);
    }

    // Step 4.  Convert heat to LED colors
    for (int j = 0; j < this->num_leds; j++)
    {
      this->setPixelHeatColor(j, heat[j]);
    }
    this->showStrip();
  }
}

void Neopixelstrip::setPixelHeatColor(int Pixel, byte temperature)
{
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature / 255.0) * 191);

  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2;              // scale up to 0..252

  // figure out which third of the spectrum we're in:
  if (t192 > 0x80)
  { // hottest
    this->setPixel(Pixel, 255, 255, heatramp);
  }
  else if (t192 > 0x40)
  { // middle
    this->setPixel(Pixel, 255, heatramp, 0);
  }
  else
  { // coolest
    this->setPixel(Pixel, heatramp, 0, 0);
  }
}

void Neopixelstrip::BouncingBalls(byte red, byte green, byte blue)
{
  float Gravity = -9.81;
  int StartHeight = 1;

  static bool first_entry = true;
  static float Height[BALLCOUNT];
  static float ImpactVelocityStart = sqrt(-2 * Gravity * StartHeight);
  static float ImpactVelocity[BALLCOUNT];
  static float TimeSinceLastBounce[BALLCOUNT];
  static int Position[BALLCOUNT];
  static long ClockTimeSinceLastBounce[BALLCOUNT];
  static float Dampening[BALLCOUNT];


  if (first_entry)
  {
    for (int i = 0; i < BALLCOUNT; i++)
    {
      ClockTimeSinceLastBounce[i] = millis();
      Height[i] = StartHeight;
      Position[i] = 0;
      ImpactVelocity[i] = ImpactVelocityStart;
      TimeSinceLastBounce[i] = 0;
      Dampening[i] = 0.90 - float(i) / pow(BALLCOUNT, 2);
    }
    first_entry = false;
  }


  for (int i = 0; i < BALLCOUNT; i++)
  {
    TimeSinceLastBounce[i] = millis() - ClockTimeSinceLastBounce[i];
    Height[i] = 0.5 * Gravity * pow(TimeSinceLastBounce[i] / 1000, 2.0) + ImpactVelocity[i] * TimeSinceLastBounce[i] / 1000;

    if (Height[i] < 0)
    {
      Height[i] = 0;
      ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
      ClockTimeSinceLastBounce[i] = millis();

      if (ImpactVelocity[i] < 0.01)
      {
        ImpactVelocity[i] = ImpactVelocityStart;
      }
    }
    Position[i] = round(Height[i] * (this->num_leds - 1) / StartHeight);
  }

  for (int i = 0; i < BALLCOUNT; i++)
  {
    this->setPixel(Position[i], red, green, blue);
  }

  this->showStrip();
  this->setAll(0, 0, 0);
}

void Neopixelstrip::BouncingColoredBalls(byte colors[][3])
{
  float Gravity = -9.81;
  int StartHeight = 1;

  static bool first_entry = true;

  static float Height[BALLCOUNT];
  static float ImpactVelocityStart = sqrt(-2 * Gravity * StartHeight);
  static float ImpactVelocity[BALLCOUNT];
  static float TimeSinceLastBounce[BALLCOUNT];
  static int Position[BALLCOUNT];
  static long ClockTimeSinceLastBounce[BALLCOUNT];
  static float Dampening[BALLCOUNT];

  if (first_entry)
  {
    for (int i = 0; i < BALLCOUNT; i++)
    {
      ClockTimeSinceLastBounce[i] = millis();
      Height[i] = StartHeight;
      Position[i] = 0;
      ImpactVelocity[i] = ImpactVelocityStart;
      TimeSinceLastBounce[i] = 0;
      Dampening[i] = 0.90 - float(i) / pow(BALLCOUNT, 2);
    }
    first_entry = false;
  }

  for (int i = 0; i < BALLCOUNT; i++)
  {
    TimeSinceLastBounce[i] = millis() - ClockTimeSinceLastBounce[i];
    Height[i] = 0.5 * Gravity * pow(TimeSinceLastBounce[i] / 1000, 2.0) + ImpactVelocity[i] * TimeSinceLastBounce[i] / 1000;

    if (Height[i] < 0)
    {
      Height[i] = 0;
      ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
      ClockTimeSinceLastBounce[i] = millis();

      if (ImpactVelocity[i] < 0.01)
      {
        ImpactVelocity[i] = ImpactVelocityStart;
      }
    }
    Position[i] = round(Height[i] * (this->num_leds - 1) / StartHeight);
  }

  for (int i = 0; i < BALLCOUNT; i++)
  {
    this->setPixel(Position[i], colors[i][0], colors[i][1], colors[i][2]);
  }

  this->showStrip();
  this->setAll(0, 0, 0);
}

void Neopixelstrip::meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay)
{
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  static int i = 0;

  if ((unsigned long) (now - lastUpdate) > SpeedDelay)
  {
    lastUpdate = now;
  
    this->setAll(0, 0, 0);

    // fade brightness all LEDs one step
    for (int j = 0; j < this->num_leds; j++)
    {
      if ((!meteorRandomDecay) || (random(10) > 5))
      {
        fadeToBlack(j, meteorTrailDecay);
      }
    }

    // draw meteor
    for (int j = 0; j < meteorSize; j++)
    {
      if ((i - j < this->num_leds) && (i - j >= 0))
      {
        this->setPixel(i - j, red, green, blue);
      }
    }
    this->showStrip();

    i++;
    if (i >= this->num_leds + this->num_leds) i = 0;
  }
}

void Neopixelstrip::fadeToBlack(int ledNo, byte fadeValue)
{
  uint32_t oldColor;
  uint8_t r, g, b;
  // int value;

  oldColor = this->strip.getPixelColor(ledNo);
  r = (oldColor & 0x00ff0000UL) >> 16;
  g = (oldColor & 0x0000ff00UL) >> 8;
  b = (oldColor & 0x000000ffUL);

  r = (r <= 10) ? 0 : (int)r - (r * fadeValue / 256);
  g = (g <= 10) ? 0 : (int)g - (g * fadeValue / 256);
  b = (b <= 10) ? 0 : (int)b - (b * fadeValue / 256);

  this->strip.setPixelColor(ledNo, r, g, b);
}

// Fire Animation Methods
// taken from https://github.com/RoboUlbricht/arduinoslovakia/blob/master/neopixel/neopixel_fire01/neopixel_fire01.ino
///

void Neopixelstrip::fireAnimation()
{
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();
  
  static int random_delay = random(50, 150);
  if ((unsigned long) (now - lastUpdate) > random_delay)
  {
    lastUpdate = now;
    random_delay = random(50, 150);
    this->fireAnimationDraw();
  }
}

/// Set all colors
///
void Neopixelstrip::fireAnimationDraw()
{
  this->clear_show();

  for (int i = 0; i < this->num_leds; i++)
  {
    this->fireAnimationAddColor(i, this->fire_color);
    int r = random(80);
    uint32_t diff_color = this->strip.Color(r, r / 2, r / 2);
    this->fireAnimationSubstractColor(i, diff_color);
  }

  this->show();
}

/// Set color of LED
///
void Neopixelstrip::fireAnimationAddColor(uint8_t position, uint32_t color)
{
  uint32_t blended_color = this->fireAnimationBlend(this->strip.getPixelColor(position), color);
  this->strip.setPixelColor(position, blended_color);
}

/// Set color of LED
///
void Neopixelstrip::fireAnimationSubstractColor(uint8_t position, uint32_t color)
{
  uint32_t blended_color = this->fireAnimationSubstract(this->strip.getPixelColor(position), color);
  this->strip.setPixelColor(position, blended_color);
}

/// Color blending
///
uint32_t Neopixelstrip::fireAnimationBlend(uint32_t color1, uint32_t color2)
{
  uint8_t r1, g1, b1;
  uint8_t r2, g2, b2;
  // uint8_t r3,g3,b3;

  r1 = (uint8_t)(color1 >> 16),
  g1 = (uint8_t)(color1 >> 8),
  b1 = (uint8_t)(color1 >> 0);

  r2 = (uint8_t)(color2 >> 16),
  g2 = (uint8_t)(color2 >> 8),
  b2 = (uint8_t)(color2 >> 0);

  return this->strip.Color(constrain(r1 + r2, 0, 255), constrain(g1 + g2, 0, 255), constrain(b1 + b2, 0, 255));
}

/// Color blending
///
uint32_t Neopixelstrip::fireAnimationSubstract(uint32_t color1, uint32_t color2)
{
  uint8_t r1, g1, b1;
  uint8_t r2, g2, b2;
  // uint8_t r3,g3,b3;
  int16_t r, g, b;

  r1 = (uint8_t)(color1 >> 16),
  g1 = (uint8_t)(color1 >> 8),
  b1 = (uint8_t)(color1 >> 0);

  r2 = (uint8_t)(color2 >> 16),
  g2 = (uint8_t)(color2 >> 8),
  b2 = (uint8_t)(color2 >> 0);

  r = (int16_t)r1 - (int16_t)r2;
  g = (int16_t)g1 - (int16_t)g2;
  b = (int16_t)b1 - (int16_t)b2;
  if (r < 0)
    r = 0;
  if (g < 0)
    g = 0;
  if (b < 0)
    b = 0;

  return this->strip.Color(r, g, b);
}