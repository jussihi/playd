/*
 * s3mContainer.hpp
 *
 *  Created on: Sep 11, 2017
 *      Author: jussi
 */

#ifndef S3MCONTAINER_HPP_
#define S3MCONTAINER_HPP_

#include "Types.hpp"
#include "ALSAPlayer.hpp"
#include <stdio.h>
#include <vector>

struct Instrument{
	byte type;
	char filename[12];
	byte memSeg[3];
	int32_t length;
	int32_t HIleng;
	int32_t loopBegin;
	int32_t HILbeg;
	int32_t loopEnd;
	int32_t HILend;
	byte volume;
	byte packFlag;
	byte flags;
	uint32_t c4Speed;
	byte* sampleData;
	double c4SpeedFactor;
	byte name[28];
	byte scrs[4];

	Instrument () :
		type(0),
		length(0),
		loopBegin(0),
		loopEnd(0),
		c4Speed(0)
	{}
};

struct Channel{
	uint32_t lastInstrument;
	uint32_t baseNote;
	double sampleOffset;
	uint32_t vibratoOffset;
	double livePeriod;
	double slideToPeriod;
	double stablePeriod;
	double liveHz;
	int volume;
	uint32_t arpeggio;

	uint32_t cacheVolumeSlide, cachePitchSlide;
	uint32_t cacheVibratoHi, cacheVibratoLo;
	uint32_t cacheTremoloHi, cacheTremoloLo;
	uint32_t cachePortamento;
	uint32_t cacheArpeggio;

	Channel() :
		baseNote(255),
		vibratoOffset(0),
		livePeriod(0.0),
		stablePeriod(0),
		liveHz(0.0),
		volume(0),
		arpeggio(0),
		cachePortamento(0)

	{}
};

struct Slot{
	uint32_t channel;
	uint32_t note;
	uint32_t instrument;
	uint32_t volume;
	uint32_t command;
	uint32_t info;
};

class s3mContainer
{
public:

	/*
	 * Constructor object
	 */
	s3mContainer();

	/*
	 * Destructor
	 */
	~s3mContainer();

	/*
	 * Load new song into this container entity
	 * @param filename: the filename to load. Should be in
	 * the same folder where the program is being run.
	 */
	void loadSong(const char* filename);

	/*
	 * Play song that is currently loaded
	 */
	void playSong();


private:

	/*
	 * Private member functions
	 */
	static Slot readSlot(const byte*& pSlot);

	static double loadSample(const Instrument& ins, double& s, double incRate);


	/*
	 * Member variables
	 */
	char m_name[29];					// name, 28 chars with ending NUL
	byte m_type;						// File type = 16

	uint16_t m_ordersNum;				// Number of orders in file
	uint16_t m_instrumentNum;			// Number of instruments in file
	uint16_t m_patternNum;				// number of patterns
	uint16_t m_flags;					// flags, unsupported in latest tracker ver
	uint16_t m_cwtv;					// created with tracker version
	uint16_t m_ffv;						// File format version

	char m_scrm[4];						// should contain SCRM

	byte m_globalVolume;				// global volume
	byte m_initialSpeed;				// initial speed
	byte m_initialTempo; 				// initial tempo
	byte m_masterVolume;				// master volume

	uint16_t m_special;					// Special pointer, not used in ver 3

	byte m_channelSettings[32];			// Channel setting for all 32 channels!
										// bit 8: channel enabled
										// bits 0-7: channel type
										// 0..7 Left sample, 8..15 right sample, 16..31 Adlib channels (9 melody + 5 drums)

	byte* m_orders;						// orders stored in a byte array

	uint16_t* m_instrumentPPs;			// parapointers to instruments
	uint16_t* m_patternPPs;				// parapointers to patterns

	Instrument* m_instruments;			// instrument array

	byte** m_patternData;				// pattern data
	byte** m_patternEnd;				// pattern endings

	ALSAPlayer* m_player;				// ALSA Player class

	uint32_t m_audioBufferSize;			// size of audio buffer

}; // end class s3mContainer


#endif /* S3MCONTAINER_HPP_ */
