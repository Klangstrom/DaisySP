#include <vector>

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "Display.h"
#include "Draw.h"
#include "Encoder.h"
#include "Key.h"
#include "LED.h"
#include "SDCard.h"
#include "WAV.h"

#include "daisysp.h"

using namespace daisysp;

#define AUDIO_SAMPLE_RATE 48000
#define BLOCK_SIZE 128
AudioDevice* audiodevice;

Key* key_LEFT;
Key* key_RIGHT;

Encoder* encoder_LEFT;
Encoder* encoder_RIGHT;

GranularPlayer player;

int sample_buffer_size = 0;
float* sample_buffer = nullptr;
float speed = 1.0f;
float transposition = -300.0f;
float grain_size = 10.0f;

void setup() {
  system_init();

  // load sample
  sdcard_init();
  while (!sdcard_detected()) {
    console_println("SD card not detected");
    delay(1000);
  }
  sdcard_mount();
  wav_load_header("LINSE.WAV");  // WAV file should be present and MONO
  sample_buffer_size = wav_num_sample_frames();
  sample_buffer = system_external_memory_allocate_float_array(sample_buffer_size);
  wav_load_sample_frames(sample_buffer, WAV_ALL_SAMPLES);

  player.Init(sample_buffer, sample_buffer_size, AUDIO_SAMPLE_RATE);

  // init display
  display_init(true);
  display_enable_automatic_update(true);

  // init leds and turn 'em all on
  led_init();
  led_on(KLST_LED_ALL);

  // init audio
  system_init_audiocodec();

  // init keys and encoders
  key_LEFT = key_create(KEY_LEFT);
  key_RIGHT = key_create(KEY_RIGHT);
  encoder_LEFT = encoder_create(ENCODER_LEFT);
  encoder_RIGHT = encoder_create(ENCODER_RIGHT);
}

void loop() {
}

void display_update_event() {
}

void display_touch_event(const TouchEvent* touchevent) {
  for (int i = 0; i < touchevent->number_of_touches; i++) {
    int x = touchevent->x[i];
    int y = touchevent->y[i];
  }
}

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

void encoder_event(const Encoder* encoder, uint8_t event) {
  if (event == ENCODER_EVENT_ROTATION) {
    console_println("ENCODER_EVENT_ROTATION(%i): %i ", encoder->device_type, encoder->rotation);

    if (encoder->device_type == 0) {  // LEFT encoder
      int val = encoder->rotation;
      speed = (float)val / 50;
    }

    if (encoder->device_type == 1) {  // RIGHT encoder
      int val = encoder->rotation;
      transposition = abs(val);
    }
  }
}

//audio callback

void audioblock(const AudioBlock* audio_block) {

  for (int i = 0; i < audio_block->block_size; ++i) {

    /** Processes the granular player.
        \param speed playback speed. 1 is normal speed, 2 is double speed, 0.5 is half speed, etc. Negative values play the sample backwards.
        \param transposition transposition in cents. 100 cents is one semitone. Negative values transpose down, positive values transpose up.
        \param grain_size grain size in milliseconds. 1 is 1 millisecond, 1000 is 1 second. Does not accept negative values. Minimum value is 1.
    */
    float out = player.Process(speed, transposition, grain_size);

    audio_block->output[0][i] = out;
    audio_block->output[1][i] = out;
  }
}
