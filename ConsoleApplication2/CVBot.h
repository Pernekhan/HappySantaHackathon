//
//  CVBot.h
//  HappySanta
//
//  Created by ABYAS on 30/10/15.
//  Copyright © 2015 ABY Applied Systems. All rights reserved.
//

#ifndef CVBOT_H
#define CVBOT_H

#include <opencv2/opencv.hpp>
#include <time.h>
#include "tools.h"

#include <iostream>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/video/background_segm.hpp"
#include <stdio.h>


const double g = 9.8;

struct InitVals
{
	double velocity;
	double angle;
};

class CVBot
{
public:
	CVBot(const cv::Point &init_point);
	~CVBot();

	InitVals findInitialValues(cv::Mat scene);
    
private:
    const cv::Point init_point;
};

#endif /* CVBOT_H */