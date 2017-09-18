/*
 * ALSAPlayer.cpp
 *
 *  Created on: Sep 13, 2017
 *      Author: jussi
 */

#include "ALSAPlayer.hpp"
#include <iostream>


int32_t ALSAPlayer::initPlayer(ALSAConfig cfg)
{
	std::cout << "=== INITIALIZING ALSA ===" << std::endl;

	if(!cfg.channels || !cfg.rate)
	{
		std::cout << "ERROR: player config was bad" << std::endl;
		return -1;
	}

	m_channels = cfg.channels;

	m_rate = cfg.rate;

	m_frames = 1024;

	uint32_t tmp;
	uint32_t buff_size;

	int dir = 0;

	/* Open the PCM device in playback mode */
	if ((pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		printf("ERROR: Can't open \"%s\" PCM device. %s\n", PCM_DEVICE, snd_strerror(pcm));
	}
	snd_pcm_hw_params_alloca(&params);

	snd_pcm_hw_params_any(pcm_handle, params);

	if ((pcm = snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));
	}

	if ((pcm = snd_pcm_hw_params_set_format(pcm_handle, params, SND_PCM_FORMAT_S8)) < 0)
	{
		printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));
	}

	if ((pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, m_channels)) < 0)
	{
		printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));
	}

	if ((pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &m_rate, &dir)) < 0)
	{
		printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));
	}

	// force the ALSA interface to use exactly *m_frames* number of frames

	snd_pcm_hw_params_set_period_size(pcm_handle, params, m_frames, dir);

	/* Write parameters */
	if ((pcm = snd_pcm_hw_params(pcm_handle, params)) < 0)
	{
		printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));
	}

	std::cout << "ALSA output device name:        " << snd_pcm_name(pcm_handle) << std::endl;
	std::cout << "ALSA output device state:       " << snd_pcm_state_name(snd_pcm_state(pcm_handle)) << std::endl;

	snd_pcm_hw_params_get_channels(params, &tmp);
	std::cout << "ALSA output device channels:    " << tmp << std::endl;

	snd_pcm_hw_params_get_rate(params, &tmp, 0);
	std::cout << "ALSA output device rate:        " << tmp << std::endl;

	snd_pcm_hw_params_get_period_size(params, &m_frames, &dir);

	buff_size = m_frames * m_channels;

	std::cout << "ALSA output device frames size: " << m_frames << std::endl;

	std::cout << "ALSA output device buffer size: " << buff_size << "(should be 1024)" << std::endl;

	return 0;
}

int ALSAPlayer::writeAudio(byte* buffer, uint32_t buffSize)
{
	int pcmRetVal;
	if(buffSize == 0)
	{
		snd_pcm_drain(pcm_handle);
		snd_pcm_close(pcm_handle);
		return -1;
	}

	if((pcmRetVal = snd_pcm_writei(pcm_handle, buffer, m_frames)) == -EPIPE)
	{
		snd_pcm_prepare(pcm_handle);
	}
	else if(pcm < 0)
	{
		std::cout << "ERROR: could not write to audio interface" << std::endl;
	}
	return 0;
}

int ALSAPlayer::closePlayer()
{
	return 0;
}
