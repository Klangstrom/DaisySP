#include <vector>

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "daisysp.h"


using namespace daisysp;

#define AUDIO_SAMPLE_RATE 48000
AudioDevice* audiodevice;


FractalRandomGenerator<ClockedNoise, 5> fract;
Oscillator lfo[2];

void audioblock(AudioBlock* audio_block) {
  for (size_t i = 0; i < audio_block->block_size; i++) {
    fract.SetFreq(fabsf(lfo[0].Process()));
    fract.SetColor(fabsf(lfo[1].Process()));
    float sig = fract.Process();
    audio_block->output[0][i] = sig;
    audio_block->output[1][i] = sig;
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


  fract.Init(AUDIO_SAMPLE_RATE);
  fract.SetFreq(AUDIO_SAMPLE_RATE / 10.f);

  lfo[0].Init(AUDIO_SAMPLE_RATE);
  lfo[1].Init(AUDIO_SAMPLE_RATE);

  lfo[0].SetFreq(.25f);
  lfo[0].SetAmp(AUDIO_SAMPLE_RATE / 3.f);
  lfo[1].SetFreq(.1f);
  lfo[1].SetAmp(1.f);
}

void loop() {}
