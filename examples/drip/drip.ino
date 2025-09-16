#include <vector>

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "daisysp.h"

using namespace daisysp;

#define AUDIO_SAMPLE_RATE 48000
AudioDevice *audiodevice;

size_t num_channels = 2;

static Drip drip;
static Metro tick;


void audioblock(AudioBlock* audio_block) {
  float output;
  bool trig;
  for (size_t i = 0; i < audio_block->block_size; i++) {
    // When the metro ticks, trigger the envelope to start.
    if (tick.Process()) {
      trig = true;
    } else {
      trig = false;
    }

    output = drip.Process(trig);

	    for (size_t chn = 0; chn < num_channels; chn++) {
      audio_block->output[chn][i] = output;
    }
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

  num_channels = 2;

  // Set up metro to pulse every second
  tick.Init(1.0f, AUDIO_SAMPLE_RATE);

  drip.Init(AUDIO_SAMPLE_RATE, .1);

}

void loop() {}
