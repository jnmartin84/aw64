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

#include "mixer.h"
#include "serializer.h"
#include "sys.h"

//extern "C" void* __n64_memset_ASM(void *a, uint8_t v, size_t s);
//extern "C" void* __n64_memset_ZERO_ASM(void *a, uint8_t v, size_t s);

extern int16_t* pcmbuf; 

Mixer::Mixer(System *stub) 
	: sys(stub) {
}

void Mixer::init() {
	memset(_channels, 0, sizeof(_channels));
	sys->startAudio(this);
}

void Mixer::free() {
	stopAll();
	sys->stopAudio();
}

void Mixer::playChannel(uint8_t channel, const MixerChunk *mc, uint16_t freq, uint8_t volume) {
	//debug(DBG_SND, "Mixer::playChannel(%d, %d, %d)", channel, freq, volume);
	//assert(channel < AUDIO_NUM_CHANNELS);
	MixerChannel *ch = &_channels[channel];
	ch->active = true;
	ch->volume = volume;
	ch->chunk = *mc;
	ch->chunkPos = 0;
	ch->chunkInc = (freq << 8) / sys->getOutputSampleRate();
}

void Mixer::stopChannel(uint8_t channel) {
	//debug(DBG_SND, "Mixer::stopChannel(%d)", channel);
	//assert(channel < AUDIO_NUM_CHANNELS);
	_channels[channel].active = false;
}

void Mixer::setChannelVolume(uint8_t channel, uint8_t volume) {
	//debug(DBG_SND, "Mixer::setChannelVolume(%d, %d)", channel, volume);
	//assert(channel < AUDIO_NUM_CHANNELS);
	_channels[channel].volume = volume;
}

void Mixer::stopAll() {
	//debug(DBG_SND, "Mixer::stopAll()");
	for (uint8_t i = 0; i < AUDIO_NUM_CHANNELS; ++i) {
		_channels[i].active = false;		
	}
}

// Called in order to populate the buf with len bytes.  
// The mixer iterates through all active channels and combine all sounds.
void mix(Mixer* mxr) {
	int16_t *pBuf = pcmbuf;

	//Clear the buffer since nothing garanty we are receiving clean memory.
	memset(pBuf, 0, NUM_BYTES_IN_SAMPLE_BUFFER);

	for (uint8_t i = 0; i < AUDIO_NUM_CHANNELS; ++i) {
		MixerChannel *ch = &(mxr->_channels[i]);
		if (!ch->active) 
			continue;

		//pBuf = pcmbuf;
		for (int j = 0; j < NUM_SAMPLES * STEREO_MUL; j+=2) {
#if 0
		uint16_t p1;
			p1 = ch->chunkPos >> 8;
			ch->chunkPos += ch->chunkInc;

			if (ch->chunk.loopLen != 0) {
				if (p1 == ch->chunk.loopPos + ch->chunk.loopLen - 1) {
					//debug(DBG_SND, "Looping sample on channel %d", i);
					ch->chunkPos = ch->chunk.loopPos;
				}
			} else {
				if (p1 == ch->chunk.len - 1) {
					//debug(DBG_SND, "Stopping sample on channel %d", i);
					ch->active = false;
					break;
				}
			}
			
			int8_t b1 = *(int8_t *)(ch->chunk.data + p1);
#endif
			uint16_t p1, p2;
			uint16_t ilc = (ch->chunkPos & 0xFF);
			p1 = ch->chunkPos >> 8;
			ch->chunkPos += ch->chunkInc;

			if (ch->chunk.loopLen != 0) {
				if (p1 == ch->chunk.loopPos + ch->chunk.loopLen - 1) {
					//debug(DBG_SND, "Looping sample on channel %d", i);
					ch->chunkPos = p2 = ch->chunk.loopPos;
				} else {
					p2 = p1 + 1;
				}
			} else {
				if (p1 == ch->chunk.len - 1) {
					//debug(DBG_SND, "Stopping sample on channel %d", i);
					ch->active = false;
					break;
				} else {
					p2 = p1 + 1;
				}
			}
			// interpolate
			int8_t b1 = *(int8_t *)(ch->chunk.data + p1);
			int8_t b2 = *(int8_t *)(ch->chunk.data + p2);
			int8_t b = (int8_t)((b1 * (0xFF - ilc) + b2 * ilc) >> 8);
			
			// set volume and clamp
			pBuf[j] = (int)pBuf[j] + (((int)(b/*1*/) * (int)ch->volume)/0x40);
		}
	}

	// the above mixes everything in mono
	// ai expects stereo interleaved so first we rescale every final mixed sample
	// and then duplicate them
	// result is clear clipping-free audio much unlike the original code in this function
	//pBuf = pcmbuf;
	for (int j = 0; j < NUM_SAMPLES * STEREO_MUL; j+=2) {
		// 4 channels of mixed audio
		// after summing
		// [-128,127]*4 == [-512,511]
		// int16 [-32768,32767)
		// [-512,511]*64 == [-32768,32767]
		int16_t pBufJ = pBuf[j] << 5;
		*(uint32_t*)(&pBuf[j]) = (pBufJ<<16) | (pBufJ);
	}
}

void Mixer::saveOrLoad(Serializer &ser) {
	for (int i = 0; i < AUDIO_NUM_CHANNELS; ++i) {
		MixerChannel *ch = &_channels[i];
		Serializer::Entry entries[] = {
			SE_INT(&ch->active, Serializer::SES_BOOL, VER(2)),
			SE_INT(&ch->volume, Serializer::SES_INT8, VER(2)),
			SE_INT(&ch->chunkPos, Serializer::SES_INT32, VER(2)),
			SE_INT(&ch->chunkInc, Serializer::SES_INT32, VER(2)),
			SE_PTR(&ch->chunk.data, VER(2)),
			SE_INT(&ch->chunk.len, Serializer::SES_INT16, VER(2)),
			SE_INT(&ch->chunk.loopPos, Serializer::SES_INT16, VER(2)),
			SE_INT(&ch->chunk.loopLen, Serializer::SES_INT16, VER(2)),
			SE_END()
		};
		ser.saveOrLoadEntries(entries);
	}
};
