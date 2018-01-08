/*
 * MultiBuffer.cpp
 *
 *  Created on: Jan 8, 2018
 *      Author: jussi
 */

#include "MultiBuffer.hpp"
#include <iostream>

MultiBuffer::MultiBuffer(const uint32_t& w_buffers, const uint32_t& w_buffSize) : m_currPosition(0)
{
	for(uint32_t i = 0; i < w_buffers; i++)
	{
		m_bufferList.push_back(new byte[w_buffSize]);
	}

}

MultiBuffer::~MultiBuffer()
{
	for(std::vector<byte*>::iterator i = m_bufferList.begin(); i != m_bufferList.end(); i++)
	{
		delete[] &i;
	}
}

byte* MultiBuffer::getNextBuffer()
{
	std::cout << m_currPosition << std::endl;
	if(m_bufferList.empty())
	{
		return nullptr;
	}
	if(m_currPosition == m_bufferList.size() - 1)
	{
		m_currPosition = 0;
		return m_bufferList[0];
	}
	m_currPosition++;
	return m_bufferList[m_currPosition];
}
