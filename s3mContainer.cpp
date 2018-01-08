/*
 * s3mContainer.cpp
 *
 *  Created on: Sep 11, 2017
 *      Author: jussi
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <iostream>
#include <cstdlib>
#include <arpa/inet.h>
#include "s3mContainer.hpp"
#include "ALSAPlayer.hpp"

static const uint32_t SamplingRate = 48000;

s3mContainer::s3mContainer()
:
m_name("")
{
	m_player = new ALSAPlayer;
	ALSAConfig cfg;
	cfg.channels = 2;
	cfg.rate = SamplingRate;
	m_player->initPlayer(cfg);
	m_audioBufferSize = m_player->getBuffSize();
	m_MBuffer = new MultiBuffer(1, m_audioBufferSize);
}

s3mContainer::~s3mContainer()
{
}

double s3mContainer::loadSample(const Instrument& ins, double& s, double incRate)
{
	if(s >= ins.length)
	{
		return 0.0;
	}
	double retVal = ins.sampleData[(uint32_t)s] - 128.0;

	s += incRate;

	if((ins.flags & 1) && s >= ins.loopEnd)
	{
		s = ins.loopBegin + fmod(s - ins.loopBegin, ins.loopEnd - ins.loopBegin);
	}

	return retVal;
}

bool s3mContainer::loadStereoSample(const Instrument& ins, double& s, double incRate, double& retL, double& retR)
{
	if(s >= ins.length)
	{
		return false;
	}

	retL = ins.sampleDataL[(uint32_t)s] - 128.0;
	retR = ins.sampleDataR[(uint32_t)s] - 128.0;

	s += incRate;

	if((ins.flags & 1) && s >= ins.loopEnd)
	{
		s = ins.loopBegin + fmod(s - ins.loopBegin, ins.loopEnd - ins.loopBegin);
	}

	return true;
}

static int periodic = 0;

Slot s3mContainer::readSlot(const byte*& pSlot)
{
	byte curByte = *pSlot++;
	Slot ret = {(uint32_t)curByte & 0x1F, 255, 0, 255, 0, 0};
	if(curByte & 0x20)
	{
		ret.note = *pSlot++;
		ret.instrument = *pSlot++;
	}
	if(curByte & 0x40)
	{
		ret.volume = *pSlot++;
	}
	if(curByte & 0x80)
	{
		ret.command = *pSlot++;
		ret.info = *pSlot++;
	}
	return ret;
}

void s3mContainer::loadSong(const char* filename)
{
	if(!filename)
	{
		std::cout << "Invalid filename. Exiting" << std::endl;
		std::exit(-1);
	}
	FILE *fp = NULL;
	fp = fopen(filename, "rb");
	if(!fp)
	{
		std::cout << "Could not open file. Exiting" << std::endl;
		std::exit(-1);
	}
	//setbuf(fp, NULL);

	/*
	 * Read the stuff to the class member variables
	 * This is a very ugly way to do it, but come up with better yourself
	 */
	std::cout << "\n=== LOADING TRACK ===\n" << std::endl;
	fread(m_name, 1, 29, fp);
	m_name[28] = 0;

	std::cout << "Track name: " << m_name << std::endl;
	fread(&m_type, 1, 1, fp);

	fseek(fp, 2, SEEK_CUR);

	fread(&m_ordersNum, 2, 1, fp);
	fread(&m_instrumentNum, 2, 1, fp);
	fread(&m_patternNum, 2, 1, fp);
	fread(&m_flags, 2, 1, fp);
	fread(&m_cwtv, 2, 1, fp);
	fread(&m_ffv, 2, 1, fp);
	fread(&m_scrm, 1, 4, fp);
	fread(&m_globalVolume, 1, 1, fp);
	fread(&m_initialSpeed, 1, 1, fp);
	fread(&m_initialTempo, 1, 1, fp);
	fread(&m_masterVolume, 1, 1, fp);

	fseek(fp, 10, SEEK_CUR);

	fread(&m_special, 2, 1, fp);
	fread(m_channelSettings, 1, 32, fp);

	// sanity check
	assert(memcmp(m_scrm, "SCRM", 4) == 0);
	assert(m_ordersNum <= 256);
	assert(m_instrumentNum <= 99);
	assert(m_patternNum <= 100);

	// make room for orders and read them from the file pointer initialized before
	// also set every order to idle in the beginning
	m_orders = std::make_unique<byte[]>(m_ordersNum);
	memset(m_orders.get(), 255, m_ordersNum);
	fread(m_orders.get(), 1, m_ordersNum, fp);

	// make room for parapointers and read them from the same file pointer
	std::cout << "Total " << m_instrumentNum << " instruments read:\n" << std::endl;
	m_instrumentPPs = std::make_unique<uint16_t[]>(m_instrumentNum);
	m_patternPPs = std::make_unique<uint16_t[]>(m_patternNum);

	fread(m_instrumentPPs.get(), 2, m_instrumentNum, fp);
	fread(m_patternPPs.get(), 2, m_patternNum, fp);

	m_instruments = std::make_unique<Instrument[]>(m_instrumentNum);

	// read instruments to memory
	for(uint32_t i=0; i < m_instrumentNum; i++)
	{
		Instrument& ins = m_instruments[i];
		fseek(fp, m_instrumentPPs.get()[i] *16 , SEEK_SET);

		// might still not work
		// works at least some way ...
		fread(&ins.type, 1 , 1, fp);
		fread(ins.filename, 1, 12, fp);
		fread(ins.memSeg, 3, 1, fp);
		fread(&ins.length, 2, 1, fp);
		fread(&ins.HIleng, 2, 1, fp);
		fread(&ins.loopBegin, 2, 1, fp);
		fread(&ins.HILbeg, 2, 1, fp);
		fread(&ins.loopEnd, 2, 1, fp);
		fread(&ins.HILend, 2, 1, fp);
		fread(&ins.volume, 1, 1, fp);

		fseek(fp, 1, SEEK_CUR);

		fread(&ins.packFlag, 1, 1, fp);
		fread(&ins.flags, 1, 1, fp);

		fread(&ins.c4Speed, 1, 2, fp);
		fread(&ins.c4SpeedFactor, 1, 2, fp);


		fseek(fp, 12, SEEK_CUR);

		fread(ins.name, 1, 28, fp);
		fread(ins.scrs, 1, 4, fp);

		// bad assertion; not every S3M files have this SCRS
		// included ...
		// assert(memcmp(ins.scrs, "SCRS", 4) == 0);

		if(ins.type != 1)
		{
			continue;
		}
		if(ins.length > 64000)
		{
			ins.length = 64000;
		}
		if(ins.flags & 1)
		{
			assert(ins.loopBegin < ins.length);
			assert(ins.loopEnd <= ins.length);
		}

		if(ins.flags & 2)
		{
			std::cout << ins.name << "stereo == TRUE" << std::endl;
			ins.stereo = true;
			uint32_t smppos = ins.memSeg[1]*0x10 + ins.memSeg[2]*0x1000 + ins.memSeg[0]*0x100000;
			fseek(fp, smppos, SEEK_SET);
			fseek(fp, ins.length, SEEK_CUR);
			ins.sampleDataL = new byte[ins.length];
			assert(ins.sampleDataL != NULL);
			fread(ins.sampleDataL, 1, ins.length, fp);
			ins.sampleDataR = new byte[ins.length];
			assert(ins.sampleDataR != NULL);
			fread(ins.sampleDataR, 1, ins.length, fp);
		}
		else
		{
			uint32_t smppos = ins.memSeg[1]*0x10 + ins.memSeg[2]*0x1000 + ins.memSeg[0]*0x100000;
			fseek(fp, smppos, SEEK_SET);
			ins.sampleData = new byte[ins.length];
			assert(ins.sampleData != NULL);
			fread(ins.sampleData, 1, ins.length, fp);
		}


		ins.c4SpeedFactor = (229079296.0 / ins.c4Speed);

		std::cout << ins.name << std::endl;
	}

	m_patternData = new byte*[m_patternNum];
	m_patternEnd = new byte*[m_patternNum];

	for(uint32_t i=0; i < m_patternNum; i++)
	{
		uint16_t length = 2;
		fseek(fp, m_patternPPs[i] * 16UL, SEEK_SET);
		if(m_patternPPs[i])
		{
			fread(&length, 1, 2, fp);
		}
		m_patternData[i] = new byte[length];
		assert(m_patternData[i] != NULL);
		fread(m_patternData[i], 1, length-2, fp);
		m_patternEnd[i] = m_patternData[i] + length-2;
	}
	fclose(fp);
}


void s3mContainer::playSong()
{

	// make channel a class instead of a struct, include the idle base not setting in the constructor of the class

	Channel channel[32];

	double arpeggioInterval = SamplingRate / 50.0;
	double frameDuration = 2.5 * SamplingRate / m_initialTempo;
	//double hertzRatio =  14317056.0 / (double)SamplingRate;
	double hertzRatio =  14317056.0 / (double)SamplingRate / 2;
	double VolumeNormalizer = (m_masterVolume & 127) * m_globalVolume / 1048576.0;
	double currTime = 0.0;
	double NoteHzTable[16];
	double inverseNoteHzTable[195];

	for(int i = 0; i < 195; ++i)
	{
		inverseNoteHzTable[i] = exp(i * -0.0577622650466621);
	}
	for(int i = 0; i < 16; ++i)
	{
		NoteHzTable[i] = exp(i * 0.0577622650466621);
	}

	uint32_t currentRow = 0;
	uint32_t nextOrder = 0;

	// loop until the song is complete
	while (1)
	{
		/* Load the pattern pointer */ /* Load the pattern pointer */ /* Load the pattern pointer */
		if(nextOrder >= m_ordersNum)
		{
			nextOrder = 0;
			continue;
		}
		uint32_t currentPattern = m_orders[nextOrder++];
		if(currentPattern == 255)
		{
			nextOrder = 0;
			continue;
		}
		if(currentPattern == 254)
		{
			continue;
		}
		const byte* pCurrPattern = m_patternData[currentPattern];
		assert(pCurrPattern != NULL);
		/* Skip to the current row */
		// printf("Skipping to row %u...¥n", row);
		for(uint32_t rowskip=0; rowskip < currentRow; ++rowskip, ++pCurrPattern)
		{
			while(*pCurrPattern != 0)
			{
				readSlot(pCurrPattern);
			}
		}

		const byte* patternLoop = pCurrPattern;
		uint32_t loopsRemain = 0;
		uint32_t loopRow = 0;
		/* Play the pattern until its end */
		int rowFinishStyle = 0;
		while(pCurrPattern < m_patternEnd[currentPattern])
		{
			uint32_t rowRepeatCount = 0;


			for(uint32_t repeat = 0; repeat <= rowRepeatCount; repeat++)
			{
				for(uint32_t frame = 0; frame < m_initialSpeed; frame++)
				{
					const byte* rowptr = pCurrPattern;
					while( *rowptr != 0)
					{
						Slot currSlot = readSlot(rowptr);

						Channel& ch = channel[currSlot.channel];

						uint32_t noteOnFrame = 0;
						uint32_t noteCutFrame = 999;
						uint32_t sampleOffset = 0;
						uint32_t volumeSlide=0;
						uint32_t pitchSlide=0;
						uint32_t vibrato=0;
						uint32_t portamento=0;
						uint32_t arpeggio=0;

						switch(currSlot.command + 64) // Parse the command
						{
						case 'A':
							m_initialSpeed = currSlot.info;
							break;
						case 'B':
							rowFinishStyle = 2;
							currentRow = 0;
							nextOrder = currSlot.info;
							break;
						case 'C':
							rowFinishStyle = 2;
							currentRow = (currSlot.info >> 4)*10 + (currSlot.info & 0x0F);
							break;
						case 'T':
							m_initialTempo = currSlot.info;
							frameDuration = 2.5* SamplingRate / m_initialTempo;
							break;
						case 'V':
							m_globalVolume = currSlot.info;
							VolumeNormalizer = (m_masterVolume & 127) * m_globalVolume / 1048576.0;
							break;
						case 'U':
								currSlot.info |= 0x100;
						case 'H':
							if(currSlot.info & 0x0F)
							{
								ch.cacheVibratoLo = currSlot.info & 0x0F;
							}
							else
							{
								currSlot.info |= ch.cacheVibratoLo;
							}
							if(currSlot.info & 0xF0)
							{
								ch.cacheVibratoHi = currSlot.info & 0xF0;
							}
							else
							{
								currSlot.info |= ch.cacheVibratoHi;
							}
							vibrato = currSlot.info;
							break;
						case 'R':
							if(currSlot.info & 0x0F)
							{
								ch.cacheVibratoLo = currSlot.info & 0x0F;
							}
							else
							{
								currSlot.info |= ch.cacheVibratoLo;
							}
							if(currSlot.info & 0xF0)
							{
								ch.cacheTremoloHi = currSlot.info & 0xF0;
							}
							else
							{
								currSlot.info |= ch.cacheTremoloHi;
							}
							break;
						case 'O': // Oxx: Set sampledata offset
							sampleOffset = currSlot.info * 0x100;
							break;
						case 'G': // Gxx: Tone portamento
							if(!currSlot.info)
							{
								portamento = ch.cachePortamento;
							}
							else
							{
								portamento = ch.cachePortamento = currSlot.info;
							}
							break;
						case 'K': // Kxy: H00 + Dxy
							vibrato = ch.cacheVibratoLo | ch.cacheVibratoHi;
							goto Dcommand;
						case 'L': // Lxy: G00 + Dxy
							portamento = ch.cachePortamento;
							goto Dcommand;
						case 'D': Dcommand: // Dxy: Volume slides
							if(!currSlot.info)
							{
								volumeSlide = ch.cacheVolumeSlide;
							}
							else
							{
								volumeSlide = ch.cacheVolumeSlide = currSlot.info;
							}
							break;
						case 'E': // Exy: Pitch slides down
							if(!currSlot.info)
							{
								pitchSlide = ch.cachePitchSlide;
							}
							else
							{
								pitchSlide = ch.cachePitchSlide = currSlot.info;
							}
							break;
						case 'F': // Fxy: Pitch slides up
							if(!currSlot.info)
							{
								pitchSlide = ch.cachePitchSlide;
							}
							else
							{
								pitchSlide = ch.cachePitchSlide = currSlot.info;
							}
							pitchSlide |= 0x100;
							break;
						case 'J': // Jxy: Arpeggio 50Hz { n, n+x, n+y }
							if(!currSlot.info)
							{
								arpeggio = ch.cacheArpeggio;
							}
							else
							{
								arpeggio = ch.cacheArpeggio = currSlot.info;
							}
							ch.arpeggio = arpeggio;
							break;
						case 'S':
							if(frame != 0)
							{
								break;
							}
							switch(currSlot.info & 0xF0)
							{
							case 0xB0: // SBx: Pattern loop
								if(currSlot.info == 0xB0)
								{
									patternLoop = pCurrPattern;
									loopRow = currentRow;
								}
								else if(loopsRemain == 0)
								{
									rowFinishStyle = 1;
									loopsRemain = currSlot.info & 0x0F;
								}
								else if(--loopsRemain > 0)
								{
									rowFinishStyle = 1;
								}
								break;
							case 0xC0:
								noteCutFrame = currSlot.info & 0x0F;
								break;
							case 0xD0:
								noteOnFrame = currSlot.info & 0x0F;
								break;
							case 0xE0:
								rowRepeatCount = currSlot.info & 0x0F;
								break;
							}
						}
						// jatkuu tänne

						if(frame == noteOnFrame)
						{
							if(currSlot.note == 254)
							{
								noteCutFrame = frame;
							}
							else if(currSlot.note != 255 || currSlot.instrument != 0)
							{
								if(currSlot.note != 255)
								{
									ch.baseNote = (currSlot.note >> 4) * 12 + (currSlot.note & 0x0F);
								}
								if(currSlot.instrument)
								{
									ch.lastInstrument = currSlot.instrument - 1;
								}
								if(currSlot.instrument && currSlot.volume == 255)
								{
									ch.volume = m_instruments[ch.lastInstrument].volume;
								}

								ch.slideToPeriod = inverseNoteHzTable[ch.baseNote] * m_instruments[ch.lastInstrument].c4SpeedFactor;

								if(!portamento)
								{
									ch.stablePeriod = ch.slideToPeriod;
									ch.sampleOffset = sampleOffset;
								}
							}
							if(currSlot.volume != 255)
							{
								ch.volume = currSlot.volume;
							}
						}

						if(frame == noteCutFrame)
						{
							ch.baseNote = 255;
						}

						if(portamento)
						{
							if(ch.stablePeriod < ch.slideToPeriod)
							{
								if((ch.stablePeriod += portamento * 4) >= ch.slideToPeriod)
								{
									ch.stablePeriod = ch.slideToPeriod;
								}
							}
							else if(ch.stablePeriod > ch.slideToPeriod)
							{
								if((ch.stablePeriod -= portamento * 4) <= ch.slideToPeriod)
								{
									ch.stablePeriod = ch.slideToPeriod;
								}
							}
						}

						ch.livePeriod = ch.stablePeriod;

						if(vibrato)
						{
							ch.vibratoOffset += vibrato >> 4;
							uint32_t vibratoStrength = vibrato & 0x0F;
							if(vibrato < 0x100)
							{
								vibratoStrength *= 4;
							}
							// sin ? cos ? the multiplier is just rnd
							double vibratoSkew = sin(ch.vibratoOffset * .0491);
							vibratoSkew *= vibratoStrength;
							ch.livePeriod = ch.stablePeriod + vibratoSkew;
						}

						if(pitchSlide)
						{
							int magnitude = pitchSlide & 0xFF;
							int fine = 0;
							if(magnitude >= 0xF0)
							{
								fine = 2;
								magnitude &= 0x0F;
							}
							if(magnitude >= 0xE0)
							{
								fine = 1;
								magnitude &= 0x0F;
							}
							if(fine < 2)
							{
								magnitude *=4;
							}
							if(pitchSlide & 0x100)
							{
								magnitude = -magnitude;
							}
							if(!fine || frame == noteOnFrame)
							{
								ch.livePeriod = ch.stablePeriod += magnitude;
							}
						}

						if(volumeSlide)
						{
							int trig = (m_cwtv == 0x1300 || (m_flags & 0x40)) || frame >= noteOnFrame;
							if((volumeSlide & 0xF0) == 0)
							{
								if(trig)
								{
									ch.volume -= volumeSlide & 0x0F;
								}
							}
							else if((volumeSlide & 0x0F) == 0)
							{
								if(trig)
								{
									ch.volume += volumeSlide >> 4;
								}
							}
							else if(frame == noteOnFrame)
							{
								if(volumeSlide & 0xF0)
								{
									ch.volume -= volumeSlide & 0x0F;
								}
								else if(volumeSlide & 0x0F)
								{
									ch.volume += volumeSlide >> 4;
								}
							}
							if(ch.volume < 0)
							{
								ch.volume = 0;
							}
							if(ch.volume > 64)
							{
								ch.volume = 64;
							}
						}

						if(ch.livePeriod != 0.0)
						{
							ch.liveHz = hertzRatio / ch.livePeriod;
						}

					} /* end of rowptr loop */

					double frameEndsAt = currTime + frameDuration;
					uint32_t numSamplesToMix = frameEndsAt - currTime;

					static uint32_t mixBuffPos = 0;
					//std::vector<byte> mixBuff;
					//mixBuff.resize(m_audioBufferSize, 128);
					byte* mixBuff = m_MBuffer->getNextBuffer();
					//std::cout << m_audioBufferSize << std::endl;

					for(uint32_t i = 0; i < numSamplesToMix; ++i)
					{
						double resL = 0.0;
						double resR = 0.0;
						for(int j = 0; j < 32; ++j)
						{
							Channel& ch = channel[j];

							// sanity check if channel is idle
							if(ch.baseNote == 255)
							{
								continue;
							}

							const Instrument& ins = m_instruments[ch.lastInstrument];

							if(ch.sampleOffset >= ins.length)
							{
								ch.baseNote = 255;
								continue;
							}

							double currHz = ch.liveHz;

							if(ch.arpeggio)
							{
								uint32_t position = fmod(currTime / arpeggioInterval, 3.0);
								currHz *= NoteHzTable[(ch.arpeggio >> (position * 4)) & 0x0F];
							}
							//resL += ch.volume * loadSample(ins, ch.sampleOffset, currHz);
							//resR += ch.volume * loadSample(ins, ch.sampleOffset, currHz);
							if(ins.stereo)
							{
								double tempL = 0.0;
								double tempR = 0.0;
								if(loadStereoSample(ins, ch.sampleOffset, currHz, tempL, tempR))
								{
									std::cout << "loaded stereo sample" << std::endl;
									resL += ch.volume * tempL;
									resR += ch.volume * tempR;
								}

							}
							else
							{
								resL += ch.volume * loadSample(ins, ch.sampleOffset, currHz);
								resR += ch.volume * loadSample(ins, ch.sampleOffset, currHz);
							}

						}
						int normalizedL = resL * VolumeNormalizer;
						int normalizedR = resR * VolumeNormalizer;

						//int retVal = res;

						//std::cout << "VolumeNormalizer: " << VolumeNormalizer << std::endl;

						// force retVal to be unsigned 0-255 (8bit)
						//if(retVal < -128 || retVal > 127)
						//{
						//	std::cout << retVal << std::endl;
						//}
						normalizedL = (normalizedL) < (-128) ? (-128) : (normalizedL);
						normalizedL = (normalizedL) > (127) ? (127) : (normalizedL);
						normalizedR = (normalizedR) < (-128) ? (-128) : (normalizedR);
						normalizedR = (normalizedR) > (127) ? (127) : (normalizedR);
						mixBuff[mixBuffPos*2] = normalizedL + 128;
						mixBuff[mixBuffPos*2+1] = normalizedR + 128;
						//std::cout << retVal + 128 << std::endl;
						mixBuffPos++;
						if(mixBuffPos >= m_audioBufferSize/2)
						{
							int luku = 0;
							//std::cout << mixBuff.size() << std::endl;
							/*if(mixBuff.size() > 0)
							{
								for(std::vector<byte>::iterator i = mixBuff.begin(); i != mixBuff.end(); i++)
								{
									std::cout << luku++ << "\t\t" << (uint32_t)*i << std::endl;
								}
							}*/
							//std::cout << mixBuffPos << std::endl;
							m_player->writeAudio(mixBuff, m_audioBufferSize/2);
							mixBuffPos = 0;
							periodic++;
							//std::cout << periodic << std::endl;
						}

						currTime += 1.0;
					}

				} // end of frame loop
			}


			if(rowFinishStyle == 1)
			{
				pCurrPattern = patternLoop;
				currentRow = loopRow;
				continue;
			}
			if(rowFinishStyle == 2)
			{
				break;
			}
			while(*pCurrPattern != 0)
			{
				readSlot(pCurrPattern);
			}
			pCurrPattern++;
			currentRow++;
		} // end of pCurrPattern loop

		if(rowFinishStyle == 0)
		{
			currentRow = 0;
		}

	} // end of while
}
