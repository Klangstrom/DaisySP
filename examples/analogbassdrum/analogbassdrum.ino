#include <vector>

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "daisysp.h"

using namespace daisysp;

#define AUDIO_SAMPLE_RATE 48000
AudioDevice* audiodevice;

DaisyHardware hw;
AnalogBassDrum bd;
Metro tick;

void audioblock(AudioBlock* audio_block) { {
  for (size_t i = 0; i < audio_block->block_size; i++) {
    bool t = tick.Process();
    if (t) {
      bd.SetTone(.7f * random() / (float)RAND_MAX);
      bd.SetDecay(random() / (float)RAND_MAX);
      bd.SetSelfFmAmount(random() / (float)RAND_MAX);
    }
    float signal = bd.Process(t);
    audio_block->output[0][i] = signal;
    audio_block->output[1][i] = signal;
  }
}

void setup() {
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

  bd.Init(AUDIO_SAMPLE_RATE);
  bd.SetFreq(50.f);

  tick.Init(2.f, AUDIO_SAMPLE_RATE);

  DAISY.begin(AudioCallback);
}

void loop() {}
