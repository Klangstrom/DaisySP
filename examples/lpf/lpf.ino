#include <vector>

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "Key.h"
#include "AudioDevice.h"
#include "Display.h"
#include "Draw.h"
#include "daisysp.h"

using namespace daisysp;

#define AUDIO_SAMPLE_RATE 48000
AudioDevice* audiodevice;

Oscillator osc_square, osc_triangle, lfo;
LadderFilter filter;

float filter_freq = 0, filter_resonance = 0;
int touch_x = display_get_width() / 2, touch_y = display_get_height() / 2;

void setup() {
  system_init();

  display_init(true);
  display_enable_automatic_update(true);

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
  osc_square.Init(AUDIO_SAMPLE_RATE);
  osc_square.SetAmp(.7f);
  osc_square.SetWaveform(osc_square.WAVE_SQUARE);
  osc_square.SetFreq(520.0);

  osc_triangle.Init(AUDIO_SAMPLE_RATE);
  osc_triangle.SetAmp(0.7f);
  osc_triangle.SetWaveform(osc_triangle.WAVE_TRI);
  osc_triangle.SetFreq(380.0);

  lfo.Init(AUDIO_SAMPLE_RATE);
  lfo.SetAmp(0.5);
  lfo.SetWaveform(lfo.WAVE_SAW);
  lfo.SetFreq(6.66);

  //init filter
  filter.Init(AUDIO_SAMPLE_RATE);
  filter.SetRes(0.7);
  filter.SetFreq(300.0);
  filter.SetFilterMode(LadderFilter::FilterMode::LP24);
}

void loop() {
}

void display_touch_event(const TouchEvent* touchevent) {
  for (int i = 0; i < 1; i++) {
    touch_x = touchevent->x[i];
    touch_y = touchevent->y[i];
    // touch_x = constrain(touch_x,40+1,display_get_width()-40);
    // touch_y = constrain(touch_y,40+1,display_get_height()-40);
    float freq = float(map(touch_x, 0, display_get_width(), 50, 890));
    float res = float(touch_y) / float(display_get_height() / 2);
    console_println("freq: %f", freq);
    console_println("res: %f", res);
    filter.SetFreq(freq);
    filter.SetRes(res);

    float lfo_freq = float(map (touch_x,0,display_get_width(),100,800))/100.0;
    lfo.SetFreq(lfo_freq);
  }
}

void clear_display(){
}

void display_update_event() {
  draw_line_horizontal(0, touch_y, display_get_width(), 0xFFFFFFFF);
  draw_line_vertical(touch_x, 0, display_get_height(), 0xFFFFFFFF);
  //background
  draw_rect_fill(0,0,touch_x,touch_y, color_from_gray(0.25));
  draw_rect_fill(touch_x,0,display_get_width()-touch_x,touch_y, color_from_gray(0.5));
  draw_rect_fill(0,touch_y,touch_x,display_get_height()-touch_y, color_from_gray(0.75));
  draw_rect_fill(touch_x,touch_y,display_get_width()-touch_x, display_get_height() - touch_y, color_from_gray(1.0));

  //circle color depending on x postion
  int r = 255;
  int g = map(touch_x, 0, display_get_width(), 255, 50);
  int b = map(touch_x, 0, display_get_width(), 255, 0);

  draw_circle_fill(touch_x, touch_y, 40, color_from_rgb(r, g, b));
}

void audioblock(const AudioBlock* audio_block) {
  for (int i = 0; i < audio_block->block_size; ++i) {
    float fmod = lfo.Process();
    fmod *= 222.0;
    fmod += 144.0;
    osc_square.SetFreq(fmod);

    float osc_signal = osc_square.Process() + osc_triangle.Process();
    float signal = filter.Process(osc_signal);
    audio_block->output[0][i] = signal;
    audio_block->output[1][i] = signal;
  }
}
