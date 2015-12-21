//
//  game.h
//  HappySanta
//
//  Created by ABYAS on 30/10/15.
//  Copyright © 2015 ABY Applied Systems. All rights reserved.
//

#ifndef GAME_H
#define GAME_H

#include "CVBot.h"
#include <iostream>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

enum obj_type
{
	SANTA, CANNON, FASTENER, SCENE,

	BALL_RED_S, BALL_RED_L,
	BALL_GREEN_S, BALL_GREEN_L,
	BALL_SKY_S, BALL_SKY_L,

	BOX_RED_S, BOX_RED_L,
	BOX_GREEN_S, BOX_GREEN_L,
	BOX_BLUE_S, BOX_BLUE_L,

	CANE_RED_S, CANE_RED_L,
	CANE_GREEN_S, CANE_GREEN_L,
	CANE_BLUE_S, CANE_BLUE_L,
};

struct Scene
{
	std::string image_name;
	std::vector<std::pair<obj_type, cv::RotatedRect>> targets; // Balls and Boxes
	std::vector<std::pair<obj_type, cv::RotatedRect>> canes;   // Canes

	void ReadFromString(const std::string &line); // Fill object vectors from strings from txt file
};

class Game
{
public:
	Game();
	~Game();
	void Start(const bool &visualize = false, const int &start_index = 0, const int &animation_speed = 100);

private:
	const cv::Point init_point;
	std::vector<Scene> scenes;
	std::vector<cv::Mat> objects;
    const std::string dir_resources;
    const std::string dir_scenes;

	void ReadObjectImages();
	void LoadScenes();
    void rotate(cv::Mat &src, const double &angle);
	void overlayImage(cv::Mat background, cv::Mat foreground, const cv::Point2i &tl);
};
#endif /* GAME_H */