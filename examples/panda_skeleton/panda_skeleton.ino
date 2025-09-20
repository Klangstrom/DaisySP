#include <vector>

#include "Arduino.h"
#include "AudioDevice.h"
#include "Console.h"
#include "Display.h"
#include "Draw.h"
#include "Encoder.h"
#include "Key.h"
#include "LED.h"
#include "System.h"


// audio config & device
#define AUDIO_SAMPLE_RATE 48000
#define BLOCK_SIZE 128
AudioDevice* audiodevice;

//inputs
Key* key_LEFT;
Key* key_RIGHT;

Encoder* encoder_LEFT;
Encoder* encoder_RIGHT;



void setup() {

  //start the computarrrr
  system_init();

  // init display
  display_init(true);
  display_enable_automatic_update(true);


  // init leds and turn 'em all on
  led_init();
  led_on(KLST_LED_ALL);


  // init audio
  AudioInfo audioinfo;
  audioinfo.sample_rate = AUDIO_SAMPLE_RATE;
  audioinfo.output_channels = 2;
  audioinfo.input_channels = 2;
  audioinfo.block_size = BLOCK_SIZE;
  audioinfo.bit_depth = 16;
  audiodevice = audiodevice_init_audiocodec(&audioinfo);
  if (audiodevice->audioinfo->device_id == AUDIO_DEVICE_INIT_ERROR) {
    console_error("error initializing audio device");
  }
  audiodevice_resume(audiodevice);


  //init keys and encoders

  key_LEFT = key_create(KEY_LEFT);
  key_RIGHT = key_create(KEY_RIGHT);

  encoder_LEFT = encoder_create(ENCODER_LEFT);
  encoder_RIGHT = encoder_create(ENCODER_RIGHT);
}



void loop() {
 
}



//display draw callback

void display_update_event() {
}


//display touch callback

void display_touch_event(TouchEvent* touchevent) {
  for (int i = 0; i < touchevent->number_of_touches; i++) {
    int x = touchevent->x;
    int y = touchevent->y;
  }
}

//key callback

void key_event(const Key* key) {
  if (key->device_id == key_LEFT->device_id) {
    console_println("left key  : %i", key->pressed);
    if (key->pressed) {
    } else {
    }
  }
  if (key->device_id == key_RIGHT->device_id) {
    console_println("right key : %i", key->pressed);
    if (key->pressed) {
    } else {
    }
  }
}


//encoder callback

void encoder_event(Encoder* encoder, uint8_t event) {
  if (event == ENCODER_EVENT_ROTATION) {
    console_println("ENCODER_EVENT_ROTATION(%i): %i ", encoder->device_type, encoder->rotation);

    if (encoder->device_type == 0) {  // LEFT encoder
      int val = encoder->rotation;
    }

    if (encoder->device_type == 1) {  // RIGHT encoder
      int val = encoder->rotation;
    }
  }
}

//audio callback

void audioblock(AudioBlock* audio_block) {

  for (int i = 0; i < audio_block->block_size; ++i) {

    float out = 0.0;

    audio_block->output[0][i] = out;
    audio_block->output[1][i] = out;
  }
}
