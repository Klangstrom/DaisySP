#include <vector>

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "Key.h"
#include "AudioDevice.h"
#include "daisysp.h"

using namespace daisysp;

#define AUDIO_SAMPLE_RATE 48000
AudioDevice* audiodevice;

Key* key_left;
Key* key_right;

Oscillator osc, lfo;
Adsr env;

float base_freq = 440.0f;
bool note_on = false;


void setup() {
  system_init();
  // init audio
  AudioInfo audioinfo;
  audioinfo.sample_rate = AUDIO_SAMPLE_RATE;
  audioinfo.output_channels = 2;
  audioinfo.input_channels = 2;
  audioinfo.block_size = 128;
  audioinfo.bit_depth = 16;
  audiodevice = audiodevice_init_audiocodec(&audioinfo);
  if (audiodevice->audioinfo->device_id == AUDIO_DEVICE_INIT_ERROR) {
    console_error("error initializing audio device");
  }
  audiodevice_resume(audiodevice);

  // init oscillator
  osc.Init(AUDIO_SAMPLE_RATE);
  osc.SetAmp(.95f);
  osc.SetFreq(440.0f);

  lfo.Init(AUDIO_SAMPLE_RATE);
  lfo.SetAmp(0.6f);
  lfo.SetWaveform(osc.WAVE_TRI);
  lfo.SetFreq(5.0f);

  // init adsr envelope

  // Set envelope parameters
  env.Init(AUDIO_SAMPLE_RATE);
  env.SetTime(ADSR_SEG_ATTACK, .1);
  env.SetTime(ADSR_SEG_DECAY, .5);
  env.SetTime(ADSR_SEG_RELEASE, .41);

  env.SetSustainLevel(1.0);

  // init keys
  key_left = key_create(KEY_LEFT);
  key_right = key_create(KEY_RIGHT);
}

void loop() {
  if (env.IsRunning()) {
    note_on = false;
  }
}



void key_event(const Key* key) {
  if (key->device_id == key_left->device_id) {
    console_println("left key  : %i", key->pressed);
    if (key->pressed) {
      // osc.SetFreq(440.0f);
      base_freq = 440.0f;
      note_on = true;
    }
  }
  if (key->device_id == key_right->device_id) {
    console_println("right key : %i", key->pressed);
    if (key->pressed) {
      // osc.SetFreq(880.0f);
      base_freq = 880.0f;
      note_on = true;
    }
  }
}


void audioblock(AudioBlock* audio_block) {
  for (int i = 0; i < audio_block->block_size; ++i) {

    float low = lfo.Process(); 
    low += 1;
    osc.SetFreq(low * base_freq + 220.0f);
    float envelope = env.Process(note_on);
    float signal = osc.Process() * envelope;
    audio_block->output[0][i] = signal;  //audio_block->input[0][i] + oscillator_left.process();
    audio_block->output[1][i] = signal;  //audio_block->input[1][i] + oscillator_right.process();
  }
}
