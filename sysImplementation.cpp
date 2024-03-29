/* Raw - Another World Interpreter
 * Copyright (C) 2004 Gregory Montoir
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <libdragon.h>
#include "sys.h"
#include "util.h"
#include "sfxplayer.h"

#define KEY_RETURN '\r'
#define KEY_c 'c'
#define KEY_p 'p'
#define KEY_right 79
#define KEY_left 80
#define KEY_down 81
#define KEY_up 82

static volatile struct AI_regs_s *AI_regs = (struct AI_regs_s *)0xA4500000;

static int16_t __attribute__((aligned(8))) pcmout1[NUM_SAMPLES*STEREO_MUL] = {0};
static int16_t __attribute__((aligned(8))) pcmout2[NUM_SAMPLES*STEREO_MUL] = {0};
int pcmflip = 0;
int16_t* pcmout[2] = {pcmout1,pcmout2};
int16_t* pcmbuf = pcmout1;
extern void mix(Mixer *mxr);
Mixer* System::mxr = 0;
extern "C" void *__safe_buffer[];

static uint16_t __attribute__((aligned(8))) bigpal[256];
static uint32_t __attribute__((aligned(8))) twocolorpal[256];
static display_context_t _dc;
static volatile uint64_t timekeeping = 0;


extern "C" display_context_t lockVideo(int wait) {
	display_context_t dc;

	if (wait) {
		while (!(dc = display_lock()));
	}
	else {
		dc = display_lock();
	}

	return dc;
}

extern "C" void unlockVideo(display_context_t dc) {
	if (dc) {
		display_show(dc);
	}
}

extern "C" void tickercb(int o) { //, int a, int b, int c) {
	timekeeping+=10;
}

extern "C" void the_audio_callback(int o) { //, int a, int b, int c) {
	if(!(AI_regs->status & AI_STATUS_FULL)) {
		mix(System::mxr);

		AI_regs->address = (volatile void *)pcmbuf;
		AI_regs->length = NUM_BYTES_IN_SAMPLE_BUFFER;
		AI_regs->control = 1;
		pcmflip ^= 1;
		pcmbuf = pcmout[pcmflip];
	};
}

struct N64Stub : System {
	enum {
		SCREEN_W = 320,
		SCREEN_H = 200,
		SOUND_SAMPLE_RATE = 11025
	};
	uint8_t* _offscreen;

	virtual ~N64Stub() {};
	virtual void init(const char *title);
	virtual void destroy();
	virtual void setPalette(uint8_t s, uint8_t n, const uint8_t* buf);
	virtual void copyRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t* buf, uint32_t pitch);
	virtual void processEvents();
	virtual void pressed_key(struct controller_data pressed_data);
	virtual void released_key(struct controller_data pressed_data);
	virtual void sleep(uint32_t duration);
	virtual uint32_t getTimeStamp();
	virtual void startAudio(void* param);
	virtual void stopAudio();

	virtual uint32_t getOutputSampleRate();
	virtual void *addTimer(uint32_t delay, TimerCallback callback, void* param);
	virtual void removeTimer(void* timerId);

	uint8_t* getOffScreenFramebuffer();

	void prepareGfxMode();
	void cleanupGfxMode();
	void switchGfxMode(bool fullscreen, uint8_t scaler);
};


void N64Stub::init(const char* title) {
		console_init();
	console_set_render_mode(RENDER_AUTOMATIC);
	controller_init();
	if (dfs_init( DFS_DEFAULT_LOCATION ) != DFS_ESUCCESS)
	{
		printf("Could not initialize filesystem!\n");
		while(1);
	}
			display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);


	memset(&input, 0, sizeof(input));

	// w*h*4bpp*4
	_offscreen = (uint8_t*)malloc(SCREEN_W * SCREEN_H * 2);
	if (!_offscreen) {
		error("Unable to allocate offscreen buffer");
	}

	timer_init();
	timekeeping = 0;
	/* timer_link_t* tick_timer = */
	new_timer(
	// 1 millisecond tics
		46875*10,
		TF_CONTINUOUS,
		tickercb
	);
}

void N64Stub::destroy() {
}

void N64Stub::setPalette(uint8_t start, uint8_t numEnties, const uint8_t* buf) {
	//assert(start + numEnties <= 16);
	for (int i = start; i < start + numEnties; ++i) {
		uint8_t c[3];
		for (int j = 0; j < 3; j++) {
			uint8_t col = buf[i * 3 + j];
			c[j] =  (col << 2) | (col & 3);
		}
		bigpal[i] = graphics_make_color(c[0], c[1], c[2], 255);
	}

	// map 8 bit value to 32 bit value and skip intermediate lookups later
	for(int i=0;i<16;i++) {
		for(int j=0;j<16;j++) {
			twocolorpal[((i&0xF)<<4) | (j&0xF)] = ((bigpal[i] & 0xffff) << 16) | (bigpal[j] & 0xffff);
		}
	}
}

void N64Stub::copyRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t* buf, uint32_t pitch) {
	const int w = width >> 1;
	const uint32_t SW = SCREEN_W >> 1;
	uint32_t* p;

	_dc = lockVideo(1);
	p = &((uint32_t*)__safe_buffer[(_dc)-1])[SW * 20];
	//For each line
	while (height--) {
		//One byte gives us two pixels, we only need to iterate w/2 times.
		for (int i = 0; i < w; i+=4) {
			//Extract two palette indices from upper and lower 4 bits
			// 8 pixels from this read
			uint32_t fourpixels = *((uint32_t*)((uintptr_t)buf + i));
			uint8_t a,b,c,d;
			a = (fourpixels >> 24)&0xff;
			b = (fourpixels >> 16)&0xff;
			c = (fourpixels >> 8) &0xff;
			d = (fourpixels >> 0) &0xff;
			// 2 pixels with this write
			p[i+0] = twocolorpal[a];
			// 2 pixels with this write
			p[i+1] = twocolorpal[b];
			// 2 pixels with this write
			p[i+2] = twocolorpal[c];
			// 2 pixels with this write
			p[i+3] = twocolorpal[d];
		}

		// skip SCREEN_W 16-bit pixels
		// SCREEN_W 16-bit == (SCREEN_W/2) 32-bit
		p += SW;
		buf += pitch;
	}

	unlockVideo(_dc);
}

// pressed_key
// handle pressed buttons that are mapped to keyboard events
void N64Stub::pressed_key(struct controller_data pressed_data) {
	struct SI_condat pressed = pressed_data.c[0];

	if (pressed.A) {
		input.lastChar = KEY_RETURN;
		input.button = true;
	}
	if (pressed.C_up) {
		input.lastChar = KEY_c;
		input.code = true;
	}
	if (pressed.up) {
		input.lastChar = KEY_up;
		input.dirMask |= PlayerInput::DIR_UP;
	}
	if (pressed.down) {
		input.lastChar = KEY_down;
		input.dirMask |= PlayerInput::DIR_DOWN;
	}
	if (pressed.left) {
		input.lastChar = KEY_left;
		input.dirMask |= PlayerInput::DIR_LEFT;
	}
	if (pressed.right) {
		input.lastChar = KEY_right;
		input.dirMask |= PlayerInput::DIR_RIGHT;
	}
	if (pressed.start) {
		input.lastChar = KEY_p;
		input.pause = true;
	}
}


// released_key
// handle released buttons that are mapped to keyboard events
void N64Stub::released_key(struct controller_data pressed_data) {
	struct SI_condat pressed = pressed_data.c[0];

	if (pressed.A) {
		input.button = false;
	}
	if (pressed.up) {
		input.dirMask &= ~PlayerInput::DIR_UP;
	}
	if (pressed.down) {
		input.dirMask &= ~PlayerInput::DIR_DOWN;
	}
	if (pressed.left) {
		input.dirMask &= ~PlayerInput::DIR_LEFT;
	}
	if (pressed.right) {
		input.dirMask &= ~PlayerInput::DIR_RIGHT;
	}
}

void N64Stub::processEvents() {
	struct controller_data keys_pressed;
	struct controller_data keys_released;

	controller_scan();

	keys_pressed = get_keys_down();
	keys_released = get_keys_up();

	pressed_key(keys_pressed);
	released_key(keys_released);
}

__attribute__ ((optimize(0))) void N64Stub::sleep(uint32_t duration) {
	// everything is single millseconds now
	const uint64_t start = timekeeping;
	const uint64_t durationtk = duration;
	while ( ((timekeeping) - start) < durationtk ) {
		continue;
	}
}

uint32_t N64Stub::getTimeStamp() {
	// 1 tick == 1 ms
	return (timekeeping);
}

void N64Stub::startAudio(void *param) {
	System::mxr = (Mixer*)param;
	audio_init(SOUND_SAMPLE_RATE, 0);
	pcmout[0] = pcmout1;
	pcmout[1] = pcmout2;
	pcmbuf = pcmout[pcmflip];

	/* timer_link_t* audio_timer = */
	new_timer(
		// double the number of times per second that samples get generated
		// to smooth out clicks and pops and allow for the flag to clear
		// for writing more sample data to AI
		46875*124,//62,//820312*3,
		TF_CONTINUOUS,
		the_audio_callback);
}

void N64Stub::stopAudio() {
}

uint32_t N64Stub::getOutputSampleRate() {
	return SOUND_SAMPLE_RATE;
}

typedef struct sfx_timer_ctx_s {
	uint32_t param1;
	uint32_t param3;
} sfx_timer_ctx_t;

static void timer_callback(int ovfl, void *ctx) {
	SfxPlayer* p = (SfxPlayer*)(((struct sfx_timer_ctx_s*)ctx)->param3);
	p->handleEvents();
}

void *N64Stub::addTimer(uint32_t delay, TimerCallback callback, void* param) {
	struct sfx_timer_ctx_s *tctx = (struct sfx_timer_ctx_s *)malloc(sizeof(struct sfx_timer_ctx_s));
	tctx->param1 = delay;
	tctx->param3 = (uint32_t)param;
	timer_link_t* timer = new_timer_context(46875*delay, TF_CONTINUOUS, timer_callback, tctx);
	return (void*)timer;
}

void N64Stub::removeTimer(void* timerId) {
    free(((timer_link_t*)timerId)->ctx);
//	stop_timer((timer_link_t*)timerId);
	delete_timer((timer_link_t*)timerId);
}

void N64Stub::prepareGfxMode() {
}

void N64Stub::cleanupGfxMode() {
	if (_offscreen) {
		free(_offscreen);
		_offscreen = 0;
	}
}

void N64Stub::switchGfxMode(bool fullscreen, uint8_t scaler) {
	prepareGfxMode();

}

uint8_t* N64Stub::getOffScreenFramebuffer() {
	return _offscreen;
}

N64Stub sysImplementation;
System* stub = &sysImplementation;
