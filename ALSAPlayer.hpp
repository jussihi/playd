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
#include <vector>

#define PCM_DEVICE "default"

struct ALSAConfig{
    uint32_t channels;
    uint32_t rate;
};

class ALSAPlayer{

public:
    ALSAPlayer() : pcm_handle(nullptr), pcm(0), params(nullptr), m_rate(0), m_channels(0), m_buffSize(0), m_frames(0), m_init(false) {}

    ~ALSAPlayer();

    int32_t initPlayer(ALSAConfig cfg);

    int writeAudio(byte* buffer, uint32_t buffSize);

    int closePlayer();

    uint32_t getChannelCount();

    uint32_t getBuffSize();

private:

    snd_pcm_t* pcm_handle;

    int32_t pcm;

    snd_pcm_hw_params_t* params;

    uint32_t m_rate;
    uint32_t m_channels;
    uint32_t m_buffSize;

    snd_pcm_uframes_t m_frames;

    bool m_init;

}; // end class ALSAPlayer

#endif
