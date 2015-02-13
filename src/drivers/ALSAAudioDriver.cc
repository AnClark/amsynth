/*
 *  ALSAAudioDriver.cc
 *
 *  Copyright (c) 2001-2015 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "ALSAAudioDriver.h"

#include "../Config.h"

#include <iostream>

#ifdef WITH_ALSA
#include <alsa/asoundlib.h>
#endif


int
ALSAAudioDriver::write(float *buffer, int nsamples)
{
	if (!_handle) {
		return -1;
	}

#ifdef WITH_ALSA

	assert(nsamples <= kMaxWriteFrames);
	for (int i = 0; i < nsamples; i++) {
		short s16 = buffer[i] * 32767;
		((unsigned char *)_buffer)[i * 2 + 0] = ( s16       & 0xff);
		((unsigned char *)_buffer)[i * 2 + 1] = ((s16 >> 8) & 0xff);
	}

	snd_pcm_t *pcm = (snd_pcm_t *)_handle;
	snd_pcm_sframes_t err = snd_pcm_writei(pcm, _buffer, nsamples / _channels);
	if (err < 0) {
		err = snd_pcm_recover(pcm, err, 1);
	}
	if (err < 0) {
		return -1;
	}
	return 0;

#else

	UNUSED_PARAM(buffer);
	UNUSED_PARAM(nsamples);
	return -1;

#endif
}

int 
ALSAAudioDriver::open( Config & config )
{
	if (_handle != NULL) {
		return 0;
	}

#ifdef WITH_ALSA


#define ALSA_CALL(expr) \
	if ((err = (expr)) < 0) { \
		std::cerr << #expr << " failed with error: " << snd_strerror(err) << std::endl; \
		if (pcm) { snd_pcm_close(pcm); } \
		return -1; \
	}

	int err = 0;

	snd_pcm_t *pcm = NULL;
	ALSA_CALL(snd_pcm_open(&pcm, config.alsa_audio_device.c_str(), SND_PCM_STREAM_PLAYBACK, 0));

	unsigned int latency = 10 * 1000;
	ALSA_CALL(snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED, config.channels, config.sample_rate, 0, latency));

#if DEBUG
	snd_pcm_uframes_t period_size = 0;
	snd_pcm_uframes_t buffer_size = 0;
	ALSA_CALL(snd_pcm_get_params(pcm, &buffer_size, &period_size));
	std::cout << "Opened ALSA device \"" << config.alsa_audio_device<< "\" @ " << config.sample_rate << "Hz, period_size = " << period_size << " buffer_size = " << buffer_size << std::endl;
#endif

	_handle = pcm;
	_channels = config.channels;
	_buffer = (short *)malloc(kMaxWriteFrames * sizeof(short));

	config.current_audio_driver = "ALSA";
#ifdef ENABLE_REALTIME
	config.current_audio_driver_wants_realtime = 1;
#endif

	return 0;

#else

	UNUSED_PARAM(config);
	return -1;

#endif
}

void ALSAAudioDriver::close()
{
#ifdef WITH_ALSA

	if (_handle != NULL) {
		snd_pcm_close((snd_pcm_t *)_handle);
		_handle = NULL;
	}

	free(_buffer);
	_buffer = NULL;

#endif
}
