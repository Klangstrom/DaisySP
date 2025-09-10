
#include <vector>

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "AudioDevice.h"
#include "daisysp.h"

using namespace daisysp;

#define AUDIO_SAMPLE_RATE 48000
AudioDevice *audiodevice;
size_t num_channels;

// Set max delay time to 0.75 of samplerate.
#define MAX_DELAY static_cast<size_t>(48000 * 0.75f)

// Helper Modules
static AdEnv env;
static Oscillator osc;
static Metro tick;

// Declare a DelayLine of MAX_DELAY number of floats.
static DelayLine<float, MAX_DELAY> del;

void audioblock(AudioBlock* audio_block) {
  float osc_out, env_out, feedback, del_out, sig_out;
  for (size_t i = 0; i < audio_block->block_size; i++) {

    // When the Metro ticks:
    // trigger the envelope to start, and change freq of oscillator.
    if (tick.Process()) {
      float freq = rand() % 200;
      osc.SetFreq(freq + 100.0f);
      env.Trigger();
    }

    // Use envelope to control the amplitude of the oscillator.
    env_out = env.Process();
    osc.SetAmp(env_out);
    osc_out = osc.Process();

    // Read from delay line
    del_out = del.Read();
    // Calculate output and feedback
    sig_out = del_out + osc_out;
    feedback = (del_out * 0.75f) + osc_out;

    // Write to the delay
    del.Write(feedback);

    for (size_t chn = 0; chn < num_channels; chn++) {
      audio_block->output[chn][i] = sig_out;
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
  
  //init metronom 
  env.Init(AUDIO_SAMPLE_RATE);
  osc.Init(AUDIO_SAMPLE_RATE);
  del.Init();

  // Set up Metro to pulse every second
  tick.Init(1.0f, AUDIO_SAMPLE_RATE);

  // set adenv parameters
  env.SetTime(ADENV_SEG_ATTACK, 0.001);
  env.SetTime(ADENV_SEG_DECAY, 0.50);
  env.SetMin(0.0);
  env.SetMax(0.25);
  env.SetCurve(0);  // linear

  // Set parameters for oscillator
  osc.SetWaveform(osc.WAVE_TRI);
  osc.SetFreq(220);
  osc.SetAmp(0.25);

  // Set Delay time to 0.75 seconds
  del.SetDelay(AUDIO_SAMPLE_RATE * 2.0f);
}

void loop() {}
