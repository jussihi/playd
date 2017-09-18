/*
 * ALSAPlayer.hpp
 *
 *  Created on: Sep 13, 2017
 *      Author: jussi
 */

#ifndef ALSAPLAYER_HPP_
#define ALSAPLAYER_HPP_

#include <alsa/asoundlib.h>
#include <stdio.h>
#include "Types.hpp"

#define PCM_DEVICE "default"

struct ALSAConfig{
	uint32_t channels;
	uint32_t rate;
};

class ALSAPlayer{

public:

	int32_t initPlayer(ALSAConfig cfg);

	int writeAudio(byte* buffer, uint32_t buffSize);

	int closePlayer();

private:

	snd_pcm_t* pcm_handle;

	uint32_t pcm;

	snd_pcm_hw_params_t* params;

	uint32_t m_rate;
	uint32_t m_channels;

	snd_pcm_uframes_t m_frames;

}; // end class ALSAPlayer

#endif
