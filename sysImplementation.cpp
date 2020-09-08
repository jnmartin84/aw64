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
	virtual uint32_t getTimeStamp();
	virtual void startAudio(AudioCallback callback, void *param);
	virtual void stopAudio();
	virtual uint32_t getOutputSampleRate();
	virtual void *addTimer(uint32_t delay, TimerCallback callback, void *param);
	virtual void removeTimer(void *timerId);
	virtual void *createMutex();
	virtual void destroyMutex(void *mutex);
	virtual void lockMutex(void *mutex);
	virtual void unlockMutex(void *mutex);
	uint8_t* getOffScreenFramebuffer();

	void prepareGfxMode();
	void cleanupGfxMode();
	void switchGfxMode(bool fullscreen, uint8_t scaler);
};

/*/
struct SDLStub : System {
	typedef void (SDLStub::*ScaleProc)(uint16_t *dst, uint16_t dstPitch, const uint16_t *src, uint16_t srcPitch, uint16_t w, uint16_t h);

	enum {
		SCREEN_W = 320,
		SCREEN_H = 200,
		SOUND_SAMPLE_RATE = 22050
	};

	struct Scaler {
		const char *name;
		ScaleProc proc;
		uint8_t factor;
	};
	
	static const Scaler _scalers[];

	uint8_t *_offscreen;
	SDL_Surface *_screen;
	SDL_Surface *_sclscreen;
	bool _fullscreen;
	uint8_t _scaler;

	uint16_t palette[NUM_COLORS];

	virtual ~SDLStub() {}
	virtual void init(const char *title);
	virtual void destroy();
	virtual void setPalette(uint8_t s, uint8_t n, const uint8_t *buf);
	virtual void copyRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *buf, uint32_t pitch);
	virtual void processEvents();
	virtual void sleep(uint32_t duration);
	virtual uint32_t getTimeStamp();
	virtual void startAudio(AudioCallback callback, void *param);
	virtual void stopAudio();
	virtual uint32_t getOutputSampleRate();
	virtual void *addTimer(uint32_t delay, TimerCallback callback, void *param);
	virtual void removeTimer(void *timerId);
	virtual void *createMutex();
	virtual void destroyMutex(void *mutex);
	virtual void lockMutex(void *mutex);
	virtual void unlockMutex(void *mutex);
	uint8_t* getOffScreenFramebuffer();

	void prepareGfxMode();
	void cleanupGfxMode();
	void switchGfxMode(bool fullscreen, uint8_t scaler);

	
};*/
/*
const SDLStub::Scaler SDLStub::_scalers[] = {
	{ "Point1_tx", &SDLStub::point1_tx, 1 },
	{ "Point2_tx", &SDLStub::point2_tx, 2 },
	{ "Scale2x", &SDLStub::scale2x, 2 },
	{ "Point3_tx", &SDLStub::point3_tx, 3 },
	{ "Scale3x", &SDLStub::scale3x, 3 }
};
*/


void N64Stub::init(const char *title) {
/*	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption(title, NULL);

	int x, y; 

	SDL_GetMouseState( &x,&y ); 
	SDL_ShowCursor( SDL_ENABLE ); 
	SDL_WarpMouse( x, y ); 

	memset(&input, 0, sizeof(input));
*/	_offscreen = (uint8_t *)malloc(SCREEN_W * SCREEN_H * 2);
	if (!_offscreen) {
		error("Unable to allocate offscreen buffer");
	}
	_fullscreen = true;
/*	_scaler = 1;
	prepareGfxMode();*/
}

void N64Stub::destroy() {
//	cleanupGfxMode();
//	SDL_Quit();
}

uint32_t bigpal[256];

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
// handle pressed buttons that are mapped to keyboard event operations such as
// moving, shooting, opening doors, toggling run mode
// also handles out-of-band input operations like toggling GOD MODE, debug display,
// adjusting gamma correction
//
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
/**

case SDLK_c:
				input.code = true;
				break;
			case SDLK_p:
				input.pause = true;
				break;

**/				
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
// handle released buttons that are mapped to keyboard event operations such as
// moving, shooting, opening doors, etc
//
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
    struct controller_data keys_held = get_keys_held();
    struct controller_data keys_released = get_keys_up();

    pressed_key(keys_pressed);
    released_key(keys_released);

/*	SDL_Event ev;
	while(SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_QUIT:
			input.quit = true;
			break;
		case SDL_KEYUP:
			switch(ev.key.keysym.sym) {
			case SDLK_LEFT:
				input.dirMask &= ~PlayerInput::DIR_LEFT;
				break;
			case SDLK_RIGHT:
				input.dirMask &= ~PlayerInput::DIR_RIGHT;
				break;
			case SDLK_UP:
				input.dirMask &= ~PlayerInput::DIR_UP;
				break;
			case SDLK_DOWN:
				input.dirMask &= ~PlayerInput::DIR_DOWN;
				break;
			case SDLK_SPACE:
			case SDLK_RETURN:
				input.button = false;
				break;
			default:
				break;
			}
			break;
		case SDL_KEYDOWN:
			if (ev.key.keysym.mod & KMOD_ALT) {
				if (ev.key.keysym.sym == SDLK_RETURN) {
					switchGfxMode(!_fullscreen, _scaler);
				} else if (ev.key.keysym.sym == SDLK_KP_PLUS) {
					uint8_t s = _scaler + 1;
					if (s < ARRAYSIZE(_scalers)) {
						switchGfxMode(_fullscreen, s);
					}
				} else if (ev.key.keysym.sym == SDLK_KP_MINUS) {
					int8_t s = _scaler - 1;
					if (_scaler > 0) {
						switchGfxMode(_fullscreen, s);
					}
				} else if (ev.key.keysym.sym == SDLK_x) {
					input.quit = true;
				}
				break;
			} else if (ev.key.keysym.mod & KMOD_CTRL) {
				if (ev.key.keysym.sym == SDLK_s) {
					input.save = true;
				} else if (ev.key.keysym.sym == SDLK_l) {
					input.load = true;
				} else if (ev.key.keysym.sym == SDLK_f) {
					input.fastMode = true;
				} else if (ev.key.keysym.sym == SDLK_KP_PLUS) {
					input.stateSlot = 1;
				} else if (ev.key.keysym.sym == SDLK_KP_MINUS) {
					input.stateSlot = -1;
				}
				break;
			}
			input.lastChar = ev.key.keysym.sym;
			switch(ev.key.keysym.sym) {
			case SDLK_LEFT:
				input.dirMask |= PlayerInput::DIR_LEFT;
				break;
			case SDLK_RIGHT:
				input.dirMask |= PlayerInput::DIR_RIGHT;
				break;
			case SDLK_UP:
				input.dirMask |= PlayerInput::DIR_UP;
				break;
			case SDLK_DOWN:
				input.dirMask |= PlayerInput::DIR_DOWN;
				break;
			case SDLK_SPACE:
			case SDLK_RETURN:
				input.button = true;
				break;
			case SDLK_c:
				input.code = true;
				break;
			case SDLK_p:
				input.pause = true;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
*/}

void N64Stub::sleep(uint32_t duration) {
    unsigned long start = get_ticks_ms();

    while ((get_ticks_ms() - start) < duration)
    {
        ;
    }
}

uint32_t N64Stub::getTimeStamp() {
	return get_ticks_ms();	
}

void N64Stub::startAudio(AudioCallback callback, void *param) {
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
*/}

void N64Stub::stopAudio() {
//	SDL_CloseAudio();
}

uint32_t N64Stub::getOutputSampleRate() {
	return SOUND_SAMPLE_RATE;
}

void *N64Stub::addTimer(uint32_t delay, TimerCallback callback, void *param) {
	return (void *)0;//return SDL_AddTimer(delay, (SDL_NewTimerCallback)callback, param);
}

void N64Stub::removeTimer(void *timerId) {
	//SDL_RemoveTimer((SDL_TimerID)timerId);
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

