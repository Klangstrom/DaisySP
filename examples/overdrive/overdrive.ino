
#include <vector>

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "daisysp.h"


using namespace daisysp;

#define AUDIO_SAMPLE_RATE 48000
AudioDevice* audiodevice;

Overdrive drive;
Oscillator osc, lfo;

void audioblock(AudioBlock* audio_block) {
  for (size_t i = 0; i < audio_block->block_size; i++) {
    drive.SetDrive(fabsf(lfo.Process()));
    float sig = drive.Process(osc.Process());
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

  osc.Init(AUDIO_SAMPLE_RATE);
  lfo.Init(AUDIO_SAMPLE_RATE);
  lfo.SetAmp(.8f);
  lfo.SetWaveform(Oscillator::WAVE_TRI);
  lfo.SetFreq(.25f);
}

void loop() {}
