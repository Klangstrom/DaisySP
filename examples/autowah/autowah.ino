#include <vector>

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "daisysp.h"

using namespace daisysp;

#define AUDIO_SAMPLE_RATE 48000
AudioDevice* audiodevice;

size_t num_channels;

static AdEnv env;
static Oscillator osc;
static Metro tick;
static Autowah autowah;

void audioblock(AudioBlock* audio_block) {
  float osc_out, env_out;
  for (size_t i = 0; i < audio_block->block_size; i++) {

    // When the metro ticks, trigger the envelope to start.
    if (tick.Process()) {
      env.Trigger();
      osc.SetFreq(float(random(100,240)));
    }

    // Use envelope to control the amplitude of the oscillator.
    // Apply autowah on the signal.
    env_out = env.Process();
    osc.SetAmp(env_out);
    osc_out = osc.Process();
    osc_out = autowah.Process(osc_out);

    for (size_t chn = 0; chn < num_channels; chn++) {
      audio_block->output[chn][i] = osc_out;
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

  env.Init(AUDIO_SAMPLE_RATE);
  osc.Init(AUDIO_SAMPLE_RATE);
  autowah.Init(AUDIO_SAMPLE_RATE);

  // Set up metro to pulse every second
  tick.Init(1.0f, AUDIO_SAMPLE_RATE);

  // set adenv parameters
  env.SetTime(ADENV_SEG_ATTACK, 0.01);
  env.SetTime(ADENV_SEG_DECAY, 0.5);
  env.SetMin(0.0);
  env.SetMax(0.5);
  env.SetCurve(0);  // linear

  // Set parameters for oscillator
  osc.SetWaveform(osc.WAVE_SAW);
  osc.SetFreq(220);
  osc.SetAmp(0.25);

  // set autowah parameters
  autowah.SetLevel(.1);
  autowah.SetDryWet(100);
  autowah.SetWah(1);


}

void loop() {}
