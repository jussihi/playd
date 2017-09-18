/*
 * main.cpp
 *
 *  Created on: Sep 11, 2017
 *      Author: jussi
 */

#include "s3mContainer.hpp"
#include "ALSAPlayer.hpp"


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("Usage: %s <filename>\n", argv[0]);
		return -1;
	}
	s3mContainer player;
	player.loadSong(argv[1]);
	player.playSong();
	return 0;
}
