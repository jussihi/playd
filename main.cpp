/*
 * main.cpp
 *
 *  Created on: Sep 11, 2017
 *      Author: jussi
 */

#include "s3mContainer.hpp"
#include "ALSAPlayer.hpp"
#include <thread>
#include <iostream>
#include <csignal>
#include <sstream>

static std::thread* playerT;
static s3mContainer* player;


void TERMHandler(int signal)
{
    std::cout << std::endl; std::cout << "Signaled exit by user" << std::endl;
    player->requestQuit();
    playerT->join();
    delete player;
    delete playerT;
    exit(0);
}

void playerThread(s3mContainer* w_player, std::string w_command)
{
    if(w_command == "play")
    {
        // placeholder
    }
    else
    {
        w_player->loadSong(w_command);
        w_player->playSong();
    }
}

int main(int argc, char** argv)
{
    // input argument check
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

    // set signal handlers for SIGINT and SIGTERM
    signal(SIGINT, TERMHandler);
    signal(SIGTERM, TERMHandler);

    player = new s3mContainer();
    std::string command(argv[1]);

    playerT = new std::thread;
    *playerT = std::thread(playerThread, player, command);

    std::string readCmd = "";

    while(1)
    {
        std::cin >> readCmd;
        if(readCmd == "quit" || readCmd == "exit")
        {
            break;
        }
        else if(readCmd.substr(4) == "play")
        {
            // parse play command
        }
        else if(readCmd == "pause")
        {
            player->pause();
        }
        else if(readCmd.length() >= 8 && readCmd.substr(8) == "playlist")
        {
            // parse playlist command
        }
        else if(readCmd == "resume")
        {
            player->resume();
        }
    }
    player->requestQuit();

    playerT->join();

    delete player;
    delete playerT;

    return 0;
}
