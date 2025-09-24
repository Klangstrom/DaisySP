#include <vector>

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "daisysp.h"


using namespace daisysp;

#define AUDIO_SAMPLE_RATE 48000
AudioDevice* audiodevice;


static Metro timer;
static Oscillator osc_sine;

void audioblock(AudioBlock* audio_block) {
  float sine, freq;
  uint8_t tic;
  for (size_t i = 0; i < audio_block->block_size; i++) {
    tic = timer.Process();
    if (tic) {
      freq = rand() % 500;
      osc_sine.SetFreq(freq);
    }

    sine = osc_sine.Process();

    audio_block->output[0][i] = sine;
    audio_block->output[1][i] = sine;
  }
}

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

  // initialize Metro object at 2 hz
  timer.Init(2, AUDIO_SAMPLE_RATE);

  // set parameters for sine oscillator object
  osc_sine.Init(AUDIO_SAMPLE_RATE);
  osc_sine.SetWaveform(Oscillator::WAVE_SIN);
  osc_sine.SetFreq(100);
  osc_sine.SetAmp(0.25);
}

void loop() {}
