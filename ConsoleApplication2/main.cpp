//
//  main.cpp
//  HappySanta
//
//  Created by ABYAS on 30/10/15.
//  Copyright © 2015 ABY Applied Systems. All rights reserved.
//

#include "Game.h"
#include <iostream>

int main()
{
    const bool VISUALIZE = true;
	const int START_INDEX = 0;
    const int ANIMATION_SPEED = 100;
    
    Game game;
    game.Start(VISUALIZE, START_INDEX, ANIMATION_SPEED);

	std::string x;
	std::getline(std::cin, x);
    
    return EXIT_SUCCESS;
}