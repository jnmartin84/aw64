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

//#include <SDL.h>
#include <libdragon.h>
#include "sys.h"
#include "util.h"
#include <map>
#include "sfxplayer.h"

static volatile uint64_t timekeeping=0;

struct N64Stub : System {
	
	enum {
		SCREEN_W = 320,
		SCREEN_H = 200,
		SOUND_SAMPLE_RATE = 11025
	};
	uint8_t *_offscreen;
	bool _fullscreen;
	uint8_t _scaler;

	uint16_t palette[NUM_COLORS];
	
	virtual ~N64Stub() {};
	virtual void init(const char *title);
	virtual void destroy();
	virtual void setPalette(uint8_t s, uint8_t n, const uint8_t *buf);
	virtual void copyRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *buf, uint32_t pitch);
	virtual void processEvents();
	virtual void pressed_key(struct controller_data pressed_data);
	virtual void released_key(struct controller_data pressed_data);
	virtual void sleep(uint32_t duration);
	virtual volatile uint32_t getTimeStamp();
	virtual void startAudio(void *param);
	virtual void stopAudio();
	
	virtual uint32_t getOutputSampleRate();
	virtual void *addTimer(uint32_t delay, TimerCallback callback, void *param);
	virtual void removeTimer(void *timerId);
	virtual void *createMutex();
	virtual void destroyMutex(void *mutex);
	virtual void lockMutex(void *mutex);
	virtual void unlockMutex(void *mutex);
	uint8_t* getOffScreenFramebuffer();

//	static void audio_callback();
	
	void prepareGfxMode();
	void cleanupGfxMode();
	void switchGfxMode(bool fullscreen, uint8_t scaler);
};


static void tickercb(int o, int a, int b, int c) {
	timekeeping++; // 1 tick == 
}

void N64Stub::init(const char *title) {
	memset(&input, 0, sizeof(input));

	_offscreen = (uint8_t *)malloc(SCREEN_W * SCREEN_H * 2);
	if (!_offscreen) {
		error("Unable to allocate offscreen buffer");
	}
	_fullscreen = true;

	timer_init();
	timekeeping = 0;
		new_timer(
		1171875,
		TF_CONTINUOUS, 
		0, 0, 0, 
		tickercb
	);
}

void N64Stub::destroy() {
}

static uint32_t bigpal[256];

void N64Stub::setPalette(uint8_t start, uint8_t numEnties, const uint8_t *buf) {

	assert(start + numEnties <= 16);

	for (int i = start; i < start + numEnties; ++i) {

		uint8_t c[3];
		for (int j = 0; j < 3; j++) {
			uint8_t col = buf[i * 3 + j];
			c[j] =  (col << 2) | (col & 3);
		}

		bigpal[i] = graphics_make_color(c[0], c[1], c[2], 255);
	}

}
extern "C" void *__n64_memcpy_ASM(void *d, const void *s, size_t n);
// special case, always fills with zero
extern "C" void *__n64_memset_ZERO_ASM(void *ptr, int value, size_t num);
// the non-special case version that accepts arbitrary fill value
extern "C" void *__n64_memset_ASM(void *ptr, int value, size_t num);

extern "C" void *__safe_buffer[];
display_context_t _dc;
display_context_t lockVideo(int wait)
{
    display_context_t dc;

    if (wait)
    {
        while (!(dc = display_lock()));
    }
    else
    {
        dc = display_lock();
    }

    return dc;
}


void unlockVideo(display_context_t dc)
{
    if (dc)
    {
        display_show(dc);
    }
}

void N64Stub::copyRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *buf, uint32_t pitch) {
    _dc = lockVideo(1);

	//buf += y * pitch + x;
	uint16_t *p = ((uint16_t *)__safe_buffer[(_dc)-1]);;

	//For each line
	while (height--) {

		//One byte gives us two pixels, we only need to iterate w/2 times.
		for (int i = 0; i < width / 2; ++i) {

			//Extract two palette indices from upper byte and lower byte.
			uint8_t p1 = *(buf + i) >> 4;
			uint8_t p2 = *(buf + i) & 0xF;

			//Get the pixel value from the palette and write in in offScreen.
			//*(p + i * 2 + 0) = palette[p1];
			//*(p + i * 2 + 1) = palette[p2];
			p[(i*2)] = bigpal[p1];
			p[(i*2)+1] = bigpal[p2];
		}

		p += SCREEN_W;
		buf += pitch;
	}
unlockVideo(_dc);
}
//
// pressed_key
// handle pressed buttons that are mapped to keyboard events
#define KEY_RETURN '\r'
#define KEY_c 'c'
#define KEY_p 'p'
#define KEY_right 79
#define KEY_left 80
#define KEY_down 81
#define KEY_up 82
void N64Stub::pressed_key(struct controller_data pressed_data)
{
    struct SI_condat pressed = pressed_data.c[0];

    if (pressed.A)
    {
		input.lastChar = KEY_RETURN;
		input.button = true;    
	}
    if (pressed.B)
    {
    }
    if (pressed.L)
    {
    }
    if (pressed.R)
    {
	}
    if (pressed.C_up)
    {
		input.lastChar = KEY_c;
		input.code = true;
	}
    if (pressed.C_down)
    {
    }
    if (pressed.C_left)
    {
    }
    if (pressed.C_right)
    {
    }
    if (pressed.up)
    {
		input.lastChar = KEY_up;
		input.dirMask |= PlayerInput::DIR_UP;
    }
    if (pressed.down)
    {
		input.lastChar = KEY_down;
		input.dirMask |= PlayerInput::DIR_DOWN;
    }
    if (pressed.left)
    {
		input.lastChar = KEY_left;
		input.dirMask |= PlayerInput::DIR_LEFT;
    }
    if (pressed.right)
    {
		input.lastChar = KEY_right;
		input.dirMask |= PlayerInput::DIR_RIGHT;
    }
    if (pressed.start)
    {
		input.lastChar = KEY_p;
		input.pause = true;
    }
}


//
// released_key
// handle released buttons that are mapped to keyboard eventsvoid
void N64Stub::released_key(struct controller_data pressed_data)
{
    struct SI_condat pressed = pressed_data.c[0];

    if (pressed.A)
    {
		input.button = false;    
	}
    if (pressed.B)
    {
    }
    if (pressed.L)
    {
    }
    if (pressed.R)
    {
    }
    if (pressed.C_up)
    {
    }
    if (pressed.C_down)
    {
    }
    if (pressed.C_left)
    {
    }
    if (pressed.C_right)
    {
    }
    if (pressed.up)
    {
		input.dirMask &= ~PlayerInput::DIR_UP;
    }
    if (pressed.down)
    {
		input.dirMask &= ~PlayerInput::DIR_DOWN;
    }
    if (pressed.left)
    {
		input.dirMask &= ~PlayerInput::DIR_LEFT;
    }
    if (pressed.right)
    {
		input.dirMask &= ~PlayerInput::DIR_RIGHT;
    }
    if (pressed.start)
    {
    }
}

void N64Stub::processEvents() {
    controller_scan();

    struct controller_data keys_pressed = get_keys_down();
    struct controller_data keys_released = get_keys_up();

    pressed_key(keys_pressed);
    released_key(keys_released);
}

__attribute__((noinline))void N64Stub::sleep(uint32_t duration) {
    const uint64_t start = getTimeStamp();

    while ((getTimeStamp() - start) < (uint64_t)duration)
    {
        ;
    }
}

volatile __attribute__((noinline)) uint32_t N64Stub::getTimeStamp() {
	return (timekeeping * 26);	
}

extern "C" volatile struct AI_regs_s *AI_regs;// = (struct AI_regs_s *)0xa4500000;

static int16_t __attribute__((aligned(8))) pcmout1[512*2] = {0}; // 1260 stereo samples
static int16_t __attribute__((aligned(8))) pcmout2[512*2] = {0};
int pcmflip = 0;
int16_t* pcmout[2] = {pcmout1,pcmout2};
int16_t* pcmbuf = pcmout1;
extern void mix(Mixer *mxr);
Mixer* System::mxr = 0;
static void the_audio_callback(int o, int a, int b, int c) {
	 
//	disable_interrupts();
	
	//callback
	
	mix(System::mxr);

	if(!(AI_regs->status & AI_STATUS_FULL)) {
		//disable_interrupts();
		AI_regs->address = (volatile void *)pcmbuf;//(((uint32_t)(pcmbuf) & 0x0FFFFFFF) | (uint32_t)0x80000000);//(uint32_t)0xA0000000);
		AI_regs->length = 512*2*2;
		AI_regs->control = 1;
		//enable_interrupts();
		pcmflip ^= 1;
		pcmbuf = pcmout[pcmflip];//pcmflip ? pcmout2 : pcmout1;
	};	
	
//	enable_interrupts();

//	printf("tac\n"); 
}



void N64Stub::startAudio(void *param) {
//	printf("startAudio\n");
//	while(1) {}
	System::mxr = (Mixer*)param;
	audio_init(SOUND_SAMPLE_RATE, 0);
	pcmout[0] = pcmout1;
	pcmout[1] = pcmout2;
	pcmbuf = pcmout[pcmflip];
    
	//printf("%02X %08X %08X %08X\n", pcmflip, pcmout[0], pcmout[1], pcmbuf);
	//sleep(5000);
	//register_AI_handler(the_audio_callback);
    //set_AI_interrupt(1);
	timer_link_t* akeeper = new_timer(
	2180232,
	TF_CONTINUOUS, 0, 0, 0, the_audio_callback);
	
/*	SDL_AudioSpec desired;
	memset(&desired, 0, sizeof(desired));

	desired.freq = SOUND_SAMPLE_RATE;
	desired.format = AUDIO_U8;
	desired.channels = 1;
	desired.samples = 2048;
	desired.callback = callback;
	desired.userdata = param;
	if (SDL_OpenAudio(&desired, NULL) == 0) {
		SDL_PauseAudio(0);
	} else {
		error("SDLStub::startAudio() unable to open sound device");
	}
*/
}

void N64Stub::stopAudio() {
//	SDL_CloseAudio();
}

uint32_t N64Stub::getOutputSampleRate() {
	return SOUND_SAMPLE_RATE;
}

static void timer_callback(int ovfl, int param1, int param2, int param3) {
	SfxPlayer* p = (SfxPlayer *)param3;
//	printf("it happens\n");
	p->handleEvents();
	//return param1;
}

void *N64Stub::addTimer(uint32_t delay, TimerCallback callback, void *param) {
	timer_link_t* timer = new_timer(46875*delay, TF_CONTINUOUS, delay, 0, (uintptr_t)param, timer_callback);
	return (void *)timer;
	//return (void*)0;
}

void N64Stub::removeTimer(void *timerId) {
	delete_timer((timer_link_t*)timerId);
}

void *N64Stub::createMutex() {
	return (void *)0;//return SDL_CreateMutex();
}

void N64Stub::destroyMutex(void *mutex) {
	//SDL_DestroyMutex((SDL_mutex *)mutex);
}

void N64Stub::lockMutex(void *mutex) {
	//SDL_mutexP((SDL_mutex *)mutex);
}

void N64Stub::unlockMutex(void *mutex) {
//	SDL_mutexV((SDL_mutex *)mutex);
}

void N64Stub::prepareGfxMode() {
/*	int w = SCREEN_W * _scalers[_scaler].factor;
	int h = SCREEN_H * _scalers[_scaler].factor;
	_screen = SDL_SetVideoMode(w, h, 16, _fullscreen ? (SDL_FULLSCREEN | SDL_HWSURFACE) : SDL_HWSURFACE);
	if (!_screen) {
		error("SDLStub::prepareGfxMode() unable to allocate _screen buffer");
	}
	_sclscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 16,
						_screen->format->Rmask,
						_screen->format->Gmask,
						_screen->format->Bmask,
						_screen->format->Amask);
	if (!_sclscreen) {
		error("SDLStub::prepareGfxMode() unable to allocate _sclscreen buffer");
	}
*/
//display_close();
//   display_init(RESOLUTION_320x240, DEPTH_16_BPP, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE);
}

void N64Stub::cleanupGfxMode() {
/*	if (_offscreen) {
		free(_offscreen);
		_offscreen = 0;
	}
	if (_sclscreen) {
		SDL_FreeSurface(_sclscreen);
		_sclscreen = 0;
	}
	if (_screen) {
		SDL_FreeSurface(_screen);
		_screen = 0;
	}
*/}

void N64Stub::switchGfxMode(bool fullscreen, uint8_t scaler) {
/*	SDL_Surface *prev_sclscreen = _sclscreen;
	SDL_FreeSurface(_screen); 	
*/	_fullscreen = fullscreen;
/*	_scaler = scaler;
*/	prepareGfxMode();
/*	SDL_BlitSurface(prev_sclscreen, NULL, _sclscreen, NULL);
	SDL_FreeSurface(prev_sclscreen);
*/}
/*
void N64Stub::point1_tx(uint16_t *dst, uint16_t dstPitch, const uint16_t *src, uint16_t srcPitch, uint16_t w, uint16_t h) {
	dstPitch >>= 1;
	while (h--) {
		__n64_memcpy_ASM(dst, src, w * 2);
		dst += dstPitch;
		src += dstPitch;
	}
}

void N64Stub::point2_tx(uint16_t *dst, uint16_t dstPitch, const uint16_t *src, uint16_t srcPitch, uint16_t w, uint16_t h) {
	dstPitch >>= 1;
	while (h--) {
		uint16_t *p = dst;
		for (int i = 0; i < w; ++i, p += 2) {
			uint16_t c = *(src + i);
			*(p + 0) = c;
			*(p + 1) = c;
			*(p + 0 + dstPitch) = c;
			*(p + 1 + dstPitch) = c;
		}
		dst += dstPitch * 2;
		src += srcPitch;
	}
}

void N64Stub::point3_tx(uint16_t *dst, uint16_t dstPitch, const uint16_t *src, uint16_t srcPitch, uint16_t w, uint16_t h) {
	dstPitch >>= 1;
	while (h--) {
		uint16_t *p = dst;
		for (int i = 0; i < w; ++i, p += 3) {
			uint16_t c = *(src + i);
			*(p + 0) = c;
			*(p + 1) = c;
			*(p + 2) = c;
			*(p + 0 + dstPitch) = c;
			*(p + 1 + dstPitch) = c;
			*(p + 2 + dstPitch) = c;
			*(p + 0 + dstPitch * 2) = c;
			*(p + 1 + dstPitch * 2) = c;
			*(p + 2 + dstPitch * 2) = c;
		}
		dst += dstPitch * 3;
		src += srcPitch;
	}
}

void N64Stub::scale2x(uint16_t *dst, uint16_t dstPitch, const uint16_t *src, uint16_t srcPitch, uint16_t w, uint16_t h) {
	dstPitch >>= 1;
	while (h--) {
		uint16_t *p = dst;
		for (int i = 0; i < w; ++i, p += 2) {
			uint16_t B = *(src + i - srcPitch);
			uint16_t D = *(src + i - 1);
			uint16_t E = *(src + i);
			uint16_t F = *(src + i + 1);
			uint16_t H = *(src + i + srcPitch);
			if (B != H && D != F) {
				*(p) = D == B ? D : E;
				*(p + 1) = B == F ? F : E;
				*(p + dstPitch) = D == H ? D : E;
				*(p + dstPitch + 1) = H == F ? F : E;
			} else {
				*(p) = E;
				*(p + 1) = E;
				*(p + dstPitch) = E;
				*(p + dstPitch + 1) = E;
			}
		}
		dst += dstPitch * 2;
		src += srcPitch;
	}
}

void N64Stub::scale3x(uint16_t *dst, uint16_t dstPitch, const uint16_t *src, uint16_t srcPitch, uint16_t w, uint16_t h) {
	dstPitch >>= 1;
	while (h--) {
		uint16_t *p = dst;
		for (int i = 0; i < w; ++i, p += 3) {
			uint16_t A = *(src + i - srcPitch - 1);
			uint16_t B = *(src + i - srcPitch);
			uint16_t C = *(src + i - srcPitch + 1);
			uint16_t D = *(src + i - 1);
			uint16_t E = *(src + i);
			uint16_t F = *(src + i + 1);
			uint16_t G = *(src + i + srcPitch - 1);
			uint16_t H = *(src + i + srcPitch);
			uint16_t I = *(src + i + srcPitch + 1);
			if (B != H && D != F) {
				*(p) = D == B ? D : E;
				*(p + 1) = (D == B && E != C) || (B == F && E != A) ? B : E;
				*(p + 2) = B == F ? F : E;
				*(p + dstPitch) = (D == B && E != G) || (D == B && E != A) ? D : E;
				*(p + dstPitch + 1) = E;
				*(p + dstPitch + 2) = (B == F && E != I) || (H == F && E != C) ? F : E;
				*(p + 2 * dstPitch) = D == H ? D : E;
				*(p + 2 * dstPitch + 1) = (D == H && E != I) || (H == F && E != G) ? H : E;
				*(p + 2 * dstPitch + 2) = H == F ? F : E;
			} else {
				*(p) = E;
				*(p + 1) = E;
				*(p + 2) = E;
				*(p + dstPitch) = E;
				*(p + dstPitch + 1) = E;
				*(p + dstPitch + 2) = E;
				*(p + 2 * dstPitch) = E;
				*(p + 2 * dstPitch + 1) = E;
				*(p + 2 * dstPitch + 2) = E;
			}
		}
		dst += dstPitch * 3;
		src += srcPitch;
	}
}
*/

uint8_t* N64Stub::getOffScreenFramebuffer()
{
	return _offscreen;
}


N64Stub sysImplementation;
System *stub = &sysImplementation;

