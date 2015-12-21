//
//  tools.h
//  HappySanta
//
//  Created by ABYAS on 30/10/15.
//  Copyright © 2015 ABY Applied Systems. All rights reserved.
//

#ifndef tools_h
#define tools_h

#include <opencv2/core/core.hpp>

/* Quadratic function: f(x) = Ax^2 + Bx + C */
class QuadFn
{
public:
    QuadFn();
    void findCoefficients(const cv::Point2d &p1, const cv::Point2d &p2, const cv::Point2d &p3);
    void calcAngleAndVelocity(const cv::Point2d &p0, double &angle, double &velocity);
    double operator() (const double &x);
    
private:
    double A;
    double B;
    double C;
    
    bool isDefined;
};

#endif /* tools_h */
