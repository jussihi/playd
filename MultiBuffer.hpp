/*
 * MultiBuffer.hpp
 *
 *  Created on: Jan 8, 2018
 *      Author: jussi
 */

#ifndef MULTIBUFFER_HPP_
#define MULTIBUFFER_HPP_

#include "Types.hpp"
#include <vector>

class MultiBuffer {
public:
	MultiBuffer(const uint32_t& w_buffers, const uint32_t& w_buffSize);

	virtual ~MultiBuffer();

	byte* getNextBuffer();


private:
	std::vector<byte*> m_bufferList;
	uint32_t m_currPosition;
};

#endif /* MULTIBUFFER_HPP_ */
