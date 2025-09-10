#include <vector>

#include "Arduino.h"
#include "System.h"
#include "Console.h"
#include "Key.h"
#include "Encoder.h"
#include "Display.h"
#include "Draw.h"
#include "AudioDevice.h"


#define AUDIO_SAMPLE_RATE 48000
#define BLOCK_SIZE 128
AudioDevice* audiodevice;


//inputs
Key* key_left;
Key* key_right;

Encoder* encoder_LEFT;
Encoder* encoder_RIGHT;


// visuals
uint32_t draw_sample_buff[128];
int disp_w, disp_h;
int off_x = 0, off_y = 8;
int pix_w = 17, pix_h = 17;


//byte beat
uint32_t t_unscaled = 0;
uint32_t t = 0;
uint8_t var_l = 0;
uint8_t var_r = 0;
uint8_t incr = 1;


/*  
#################################### SETUP 
*/

void setup() {
  system_init();

  //init display
  display_init(true);
  display_enable_automatic_update(true);
  display_set_backlight(1.0);

    disp_w = display_get_width();
  disp_h = display_get_height();
  console_println("display width: %i", disp_w);
  console_println("display height: %i", disp_h);

  // init audio
  AudioInfo audioinfo;
  audioinfo.sample_rate = AUDIO_SAMPLE_RATE;
  audioinfo.output_channels = 2;
  audioinfo.input_channels = 2;
  audioinfo.block_size = BLOCK_SIZE;
  audioinfo.bit_depth = 16;
  audiodevice = audiodevice_init_audiocodec(&audioinfo);
  if (audiodevice->audioinfo->device_id == AUDIO_DEVICE_INIT_ERROR) {
    console_error("error initializing audio device");
  }
  audiodevice_resume(audiodevice);

  // init keys
  key_left = key_create(KEY_LEFT);
  key_right = key_create(KEY_RIGHT);

  //init encoders
  encoder_LEFT = encoder_create(ENCODER_LEFT);
  encoder_RIGHT = encoder_create(ENCODER_RIGHT);
}

/*  
#################################### LOOP 
*/


void loop() {
}


/*  
#################################### DISPLAY
*/

void display_update_event() {

  for (int x = 0, i = 0; x < (BLOCK_SIZE / 16); x++) {
    for (int y = 0; y < 15; y++, i++) {
      uint32_t c = draw_sample_buff[i];
      int pos_x = ((off_x + x) * pix_w);
      pos_x %= disp_w;
      int pos_y = off_y + (y * pix_h);
      draw_rect_fill(pos_x, pos_y, pix_w, pix_h, c);
    }
  }

  off_x += (BLOCK_SIZE / 16);
  off_x %= disp_w;
}



/*  
#################################### KEY
*/

void key_event(const Key* key) {
  if (key->device_id == key_left->device_id) {
    console_println("left key  : %i", key->pressed);
    if (key->pressed) {
      incr = 2;
    } else {
      incr = 1;
    }
  }
  if (key->device_id == key_right->device_id) {
    console_println("right key : %i", key->pressed);
    if (key->pressed) {
      incr = 3;
    } else {
      incr = 1;
    }
  }
}

/*  
#################################### ENCODER
*/

void encoder_event(Encoder* encoder, uint8_t event) {
  if (event == ENCODER_EVENT_ROTATION) {
    console_println("ENCODER_EVENT_ROTATION(%i): %i ", encoder->device_type, encoder->rotation);

    if (encoder->device_type == 0) {  // LEFT encoder
      var_l = 5 + (((encoder->rotation) >> 4) % 12);
    }

    if (encoder->device_type == 1) {  // RIGHT encoder
      var_r = 1 + (((encoder->rotation) >> 2) % 64);
    }
  }
}

/*  
#################################### AUDIO 
*/


void audioblock(AudioBlock* audio_block) {
  for (int i = 0; i < audio_block->block_size; ++i) {
    // increment and scale counter t
    t_unscaled += incr;
    t = t_unscaled >> 2;
    // compute sample using a magic formula / byte beat
    uint32_t out = (t >> var_l) | (t << 8) | (t >> 4) | (t & var_r) & (t << (t & 222)) | (t << 7) ^ (t >> 1);
    draw_sample_buff[i] = (out * out) | 0xFF000000;  // shift color space a bit and set alpha to 100%
    // scale sample value from [0,255] to [-1.0,1.0]
    float out_f = float(out & 0xFF);
    out_f /= 128.0;
    out_f -= 1.0;

    audio_block->output[0][i] = out_f;
    audio_block->output[1][i] = out_f;
  }
}