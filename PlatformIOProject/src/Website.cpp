#include <Website.hpp>

Website::Website()
{
}

Website::~Website()
{
}

// getter and setter-methods
bool Website::get_blocked_pir_modules()
{
  return this->blocked_pir_modules;
}

void Website::set_blocked_pir_modules(bool new_state)
{
  this->blocked_pir_modules = new_state;
}

void Website::toggle_blocked_pir_modules()
{
  this->blocked_pir_modules = !this->blocked_pir_modules;
}

bool Website::get_animation_active()
{
  return this->animation_active;
}

void Website::set_animation_active(bool new_state)
{
  this->animation_active = new_state;
}

void Website::toggle_animation_active()
{
  this->animation_active = !this->animation_active;
}


String Website::get_sliderRot()
{
  return this->sliderRot;
}

int Website::get_sliderRot_int()
{
  return this->sliderRot.toInt();
}

void Website::set_sliderRot(String new_value)
{
  this->sliderRot = new_value;
}

String Website::get_sliderGruen()
{
  return this->sliderGruen;
}

int Website::get_sliderGruen_int()
{
  return this->sliderGruen.toInt();
}

void Website::set_sliderGruen(String new_value)
{
  this->sliderGruen = new_value;
}

String Website::get_sliderBlau()
{
  return this->sliderBlau;
}

int Website::get_sliderBlau_int()
{
  return this->sliderBlau.toInt();
}

void Website::set_sliderBlau(String new_value)
{
  this->sliderBlau = new_value;
}

String Website::get_list_timestamps_pir_triggered()
{
  return this->list_timestamps_pir_triggered;
}

void Website::append_list_timestamps_pir_triggered(String to_append)
{
  this->list_timestamps_pir_triggered += to_append;
}

int Website::get_pirs_triggered_since_last_reboot()
{
  return this->pirs_triggered_since_last_reboot;
}

void Website::increase_by_one_pirs_triggered_since_last_reboot()
{
  this->pirs_triggered_since_last_reboot++;
}

unsigned long Website::get_timestamp_animation_active()
{
  return this->timestamp_animation_active;
}

void Website::set_timestamp_animation_active(unsigned long new_value)
{
  this->timestamp_animation_active = new_value;
}

SELECTED_ANIMATION Website::get_selected_animation()
{
  return this->selected_animation;
}

SELECTED_ANIMATION Website::get_previous_animation()
{
  return this->previous_animation;
}


// methods
void Website::updateWebsite(uint8_t case_id)
{
  switch (case_id)
  {
    // animation active
    case 1:
      this->animation_active = !this->animation_active;
      if(this->animation_active)  this->timestamp_animation_active = millis(); // track timestamp when animation was activated
      break;

    // pir state changed
    case 2:
      this->blocked_pir_modules = !this->blocked_pir_modules;
      break;
        
    default:
      break;
  }
}

void Website::update_selected_animation(String new_animation)
{
  if (new_animation == "rgb")
  {
    this->selected_animation = RGB;
  }
  else if (new_animation == "weiss")
  {
    this->selected_animation = WEISS;
  }
  else if (new_animation == "color_wipe")
  {
    this->selected_animation = COLOR_WIPE;
  }
  else if (new_animation == "theater_chase")
  {
    this->selected_animation = THEATER_CHASE;
  }
  else if (new_animation == "rainbow")
  {
    this->selected_animation = RAINBOW;
  }
  else if (new_animation == "theater_chase_rainbow")
  {
    this->selected_animation = THEATER_CHASE_RAINBOW;
  }
  else if (new_animation == "puff_modus")
  {
    this->selected_animation = PUFF_MODUS;
  }
  else if (new_animation == "cylon_bounce")
  {
    this->selected_animation = CYLON_BOUNCE;
  }
  else if (new_animation == "sparkle")
  {
    this->selected_animation = SPARKLE;
  }
  else if (new_animation == "runnings_lights")
  {
    this->selected_animation = RUNNING_LIGHTS;
  }
  else if (new_animation == "fire")
  {
    this->selected_animation = FIRE;
  }
  else if (new_animation == "bouncing_balls")
  {
    this->selected_animation = BOUNCING_BALLS;
  }
  else if (new_animation == "bouncing_balls_colored")
  {
    this->selected_animation = BOUNCING_BALLS_COLORED;
  }
  else if (new_animation == "meteor_rain")
  {
    this->selected_animation = METEOR_RAIN;
  }
  else if (new_animation == "fire_animation")
  {
    this->selected_animation = FIRE_ANIMATION;
  }

  if (this->animation_active) this->timestamp_animation_active = millis();
}

void Website::update_previous_animation()
{
  this->previous_animation = this->get_selected_animation();
}


void Website::select_next_animation()
{
  if (this->selected_animation == RGB)
  {
    this->selected_animation = WEISS;
  }
  else if (this->selected_animation == WEISS)
  {
    this->selected_animation = COLOR_WIPE;
  }
  else if (this->selected_animation == COLOR_WIPE)
  {
    this->selected_animation = THEATER_CHASE;
  }
  else if (this->selected_animation == THEATER_CHASE)
  {
    this->selected_animation = RAINBOW;
  }
  else if (this->selected_animation == RAINBOW)
  {
    this->selected_animation = THEATER_CHASE_RAINBOW;
  }
  else if (this->selected_animation == THEATER_CHASE_RAINBOW)
  {
    this->selected_animation = PUFF_MODUS;
  }
  else if (this->selected_animation == PUFF_MODUS)
  {
    this->selected_animation = CYLON_BOUNCE;
  }
  else if (this->selected_animation == CYLON_BOUNCE)
  {
    this->selected_animation = SPARKLE;
  }
  else if (this->selected_animation == SPARKLE)
  {
    this->selected_animation = RUNNING_LIGHTS;
  }
  else if (this->selected_animation == RUNNING_LIGHTS)
  {
    this->selected_animation = FIRE;
  }
  else if (this->selected_animation == FIRE)
  {
    this->selected_animation = BOUNCING_BALLS;
  }
  else if (this->selected_animation == BOUNCING_BALLS)
  {
    this->selected_animation = BOUNCING_BALLS_COLORED;
  }
  else if (this->selected_animation == BOUNCING_BALLS_COLORED)
  {
    this->selected_animation = METEOR_RAIN;
  }
  else if (this->selected_animation == METEOR_RAIN)
  {
    this->selected_animation = FIRE_ANIMATION;
  }
  else if (this->selected_animation == FIRE_ANIMATION)
  {
    this->selected_animation = RGB;
  }
  else
  {

  }
  #ifdef DEBUG_MODE
    Serial.print("next animation=");
    Serial.println(this->selected_animation);
  #endif

  if (this->animation_active) this->timestamp_animation_active = millis();
}


String Website::StateOfSwitches(String mode)
{
  if (mode == "animation_active" && this->animation_active)
  {
    return "checked";
  }
  else if (mode == "pir" && this->blocked_pir_modules)
  {
    return "checked";
  }
  else
  {
    return "";
  }
}