//
//  tools.cpp
//  HappySanta
//
//  Created by Zhalgas Baibatyr on 30/10/15.
//  Copyright © 2015 ABY Applied Systems. All rights reserved.
//

#include "tools.h"

QuadFn::QuadFn() : isDefined(false) {}

void QuadFn::findCoefficients(const cv::Point2d &p1, const cv::Point2d &p2, const cv::Point2d &p3)
{
    double tmpVal0 = p2.x - p1.x;
    double tmpVal1 = p3.x * (p3.x - p1.x - p2.x) + p1.x * p2.x;
    
    if (tmpVal0 == 0 || tmpVal1 == 0)
    {
        CV_Error(CV_StsDivByZero, "Division by zero in \"findCoefficients()\"!");
        
        return;
    }
    
    A = (p3.y - (p3.x * (p2.y - p1.y) + p2.x * p1.y - p1.x * p2.y) / tmpVal0) / tmpVal1;
    B = (p2.y - p1.y) / tmpVal0 - A * (p1.x + p2.x);
    C = (p2.x * p1.y - p1.x * p2.y) / tmpVal0 + A * p1.x * p2.x;
    
    isDefined = true;
    
    return;
}

void QuadFn::calcAngleAndVelocity(const cv::Point2d &p0, double &angle, double &velocity)
{
    if (isDefined == false)
    {
        perror("Function is not defined!");
        exit(EXIT_FAILURE);
    }
    
    const double g = 9.8;
    
    angle = -atan(2 * A * p0.x + B);
    velocity = sqrt(std::abs(g / (2 * A * cos(angle) * cos(angle))));
    angle *= 180.0 / CV_PI;
    
    return;
}

double QuadFn::operator() (const double &x)
{
    if (isDefined == false)
    {
        perror("Function is not defined!");
        exit(EXIT_FAILURE);
    }
    
    return (A * x * x + B * x + C);
}
