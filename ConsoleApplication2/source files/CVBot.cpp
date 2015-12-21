#pragma comment(linker, "/STACK:36777216")
#include "CVBot.h"

#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <fstream>
#include <cassert>
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <stack>
#include <functional>
#include <numeric>
#include <ctime>
#include <cstdlib>
#include <sstream>

#define f first
#define s second
#define mp make_pair
#define pb push_back
#define pii pair<int, int>
#define pll pair<long long, long long>
#define y1 stupid_y1
#define ll long long
#define forit(it, s) for(__typeof(s.begin()) it = s.begin(); it != s.end(); it++)
#define all(a) a.begin(), a.end()
#define sqr(x) ((x)*(x))
#define sz(a) (int)a.size()
#define file "a"

using namespace cv;
using namespace std;

struct CircleObject{
	int x, y;
	double r;
	CircleObject(int x, int y, double r) : x(x), y(y), r(r) {}
};

CVBot::CVBot(const cv::Point &init_point) : init_point(init_point) {}

CVBot::~CVBot() {}

Mat sobelDetection(Mat matrix){
	Mat src = matrix.clone();
	Mat src_gray, grad;
	cvtColor(src, src_gray, CV_BGR2GRAY);
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;
	int scale = 1;
	int delta = 0;
	int ddepth = CV_16S;
	Sobel(src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);

	Sobel(src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
	convertScaleAbs(grad_y, abs_grad_y);

	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

	return grad;
}

vector < int > find_background_avg_color(Mat matrix){
	vector < int > res = { 254, 227, 203 };
	return res;
}

Mat makeWhiteBottomAvg(Mat scene){
	Mat matrix = scene.clone();
	int dim(256);
	Mat lut(1, &dim, CV_8UC(matrix.channels()));
	int top = 360;
	vector <int> avg = find_background_avg_color(matrix);
	for (int i = top; i < matrix.rows; i++){
		for (int j = 0; j < matrix.cols; j++){
			if (matrix.at<Vec3b>(i, j)[0] >= 250 && matrix.at<Vec3b>(i, j)[1] >= 250 && matrix.at<Vec3b>(i, j)[2] >= 250){
				matrix.at<Vec3b>(i, j)[0] = avg[0];
				matrix.at<Vec3b>(i, j)[1] = avg[1];
				matrix.at<Vec3b>(i, j)[2] = avg[2];
			}
		}
	}
	Mat dst_roi = matrix(Rect(0, top, matrix.cols, matrix.rows - top));
	medianBlur(dst_roi, dst_roi, 3);
	for (int i = top, ii = 0; i < matrix.rows; i++, ii++){
		for (int j = 0; j < matrix.cols; j++){
			matrix.at<Vec3b>(i, j) = dst_roi.at<Vec3b>(ii, j);
		}
	}
	return matrix;
}

vector<vector<Point> > contourDetection(Mat scene){
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(scene, contours, hierarchy, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	return contours;
}

vector<vector<Point> > removeNoise(vector<vector<Point> > &contours){
	vector<vector<Point> > res;
	for (int i = 0; i< contours.size(); i++)
	{
		double area = cv::contourArea(contours[i]);
		if (area < 25 * 25) continue;
		res.push_back(contours[i]);
	}
	vector < vector < Point > > ans;
	for (int i = 0; i < res.size(); i++){
		bool outside = true;
		for (int j = 0; j < res.size() && outside; j++){
			if (i == j) continue;
			vector < Point > &cur = res[i];
			for (int k = 0; k < cur.size() && outside; k++){
				int val = pointPolygonTest(res[j], cur[k], false);
				if (val == 1) outside = false;
			}
		}
		if (outside) ans.push_back(res[i]);
	}
	return ans;
}

vector < CircleObject > getCircles(vector < vector < Point > >  &contours, Mat &scene){
	vector < CircleObject > res;

	for (int i = 0; i< contours.size(); i++)
	{
		double area = cv::contourArea(contours[i]);
		Rect r = boundingRect(contours[i]);
		RotatedRect rect = minAreaRect(contours[i]);

		double radius = rect.size.width / 2;

		int cx = rect.center.x;
		int cy = rect.center.y;

		double width = rect.size.width;
		double height = rect.size.height;
		if (width > height) swap(width, height);

		if (width / height > 0.80){
			res.push_back(CircleObject(cx, cy, radius));
		}

	}
	return res;
}


struct QuadFunction{
	double A;
	double B;
	double C;
	bool isDefined;

	Point2d p1, p2, p3;

	QuadFunction(){}
	QuadFunction(cv::Point2d p1, cv::Point2d p2, cv::Point2d p3) : p1(p1), p2(p2), p3(p3)
	{
		double tmpVal0 = p2.x - p1.x;
		double tmpVal1 = p3.x * (p3.x - p1.x - p2.x) + p1.x * p2.x;

		if (tmpVal0 == 0 || tmpVal1 == 0)
		{
			isDefined = false;
			return;
		}

		A = (p3.y - (p3.x * (p2.y - p1.y) + p2.x * p1.y - p1.x * p2.y) / tmpVal0) / tmpVal1;
		B = (p2.y - p1.y) / tmpVal0 - A * (p1.x + p2.x);
		C = (p2.x * p1.y - p1.x * p2.y) / tmpVal0 + A * p1.x * p2.x;

		isDefined = true;

		return;
	}

	bool intersect(CircleObject o){
		return getDist(o) < o.r + 1e-9;
	}


	double getDist(CircleObject o){
		double x = o.x;
		double y = o.y;


		double l = o.x - o.r, r = o.x + o.r;
		for (int i = 0; i < 10; i++){
			double m1 = l + (r - l) / 3;
			double m2 = r - (r - l) / 3;
			double y1 = A*m1*m1 + B*m1 + C;
			double y2 = A*m2*m2 + B*m2 + C;
			double dist1 = ((x - m1)*(x - m1) + (y - y1)*(y - y1));
			double dist2 = ((x - m2)*(x - m2) + (y - y2)*(y - y2));
			if (dist1 < dist2){
				r = m2;
			}
			else
				l = m1;
		}

		double nx = (l + r) / 2;
		double ny = A*nx*nx + B*nx + C;

		return sqrt((nx - x)*(nx - x) + (ny - y)*(ny - y));
	}

	bool isUpper(CircleObject o){
		double x = o.x;
		double y = o.y;

		double nx = x;
		double ny = A*x*x + B*x + C;

		if (ny <= y)
			return true;
		return false;
	}
};

void iteration(int start, int step, int &maxi, QuadFunction &best, double &minError, vector<CircleObject> &circles, Mat &scene){
	Point start_point = Point(50, 346);
	double radius = 33;

	for (int i = start; i < 340; i+=step){
		for (int j = 50; j < scene.cols; j++){
			int x = j;
			int y = i;
			Point first_point = Point(50, 346);
			Point first_point_left = Point(50 - radius, 346);
			Point first_point_right = Point(50 + radius, 346);

			Point second_point = Point(x, y);
			Point second_point_left = Point(x, y - radius);
			Point second_point_right = Point(x, y + radius);

			Point third_point = Point(first_point.x + 2 * (second_point.x - first_point.x), 346);
			Point third_point_left = Point(first_point_left.x + 2 * (second_point_left.x - first_point_left.x), 346);
			Point third_point_right = Point(first_point_right.x + 2 * (second_point_right.x - first_point_right.x), 346);

			QuadFunction qf_left = QuadFunction(first_point_left, second_point_left, third_point_left);
			QuadFunction qf_right = QuadFunction(first_point_right, second_point_right, third_point_right);
			QuadFunction qf = QuadFunction(first_point, second_point, third_point);

			if (!qf_left.isDefined || !qf_right.isDefined || !qf_right.isDefined){
				continue;
			}

			int cnt = 0;
			double sqrError = 0.0;
			for (int k = 0; k < circles.size(); k++){
				bool hit = false;
				if (qf_left.intersect(circles[k]) || qf_right.intersect(circles[k]) || qf.intersect(circles[k])){
					hit = true;
				}
				else if (qf_left.isUpper(circles[k]) && !qf_right.isUpper(circles[k])){
					hit = true;
				}

				if (hit){
					cnt++;
					double error = qf.getDist(circles[k]);
					sqrError += error*error;

				}
			}

			if (cnt > maxi || (cnt == maxi && sqrError < minError)){
				maxi = cnt;
				best = qf;
				minError = sqrError;
			}
		}
	}

}

InitVals CVBot::findInitialValues(cv::Mat scene)
{
	Mat matrix = scene.clone();
	matrix = makeWhiteBottomAvg(matrix);
	matrix = sobelDetection(matrix);
	threshold(matrix, matrix, 14, 255, CV_THRESH_BINARY);
	vector<vector<Point> > contours = contourDetection(matrix);
	contours = removeNoise(contours);
	vector<CircleObject> circles = getCircles(contours, matrix);


	QuadFunction best = QuadFunction();
	int maxi = -1;
	double minError = 1e18;
	iteration(-888, 1, maxi, best, minError, circles, scene);
	iteration((int)-1e6, (int)1e4, maxi, best, minError, circles, scene);

	InitVals init_vals;

	QuadFn quadratic_func;
	quadratic_func.findCoefficients(best.p1, best.p2, best.p3);
	quadratic_func.calcAngleAndVelocity(init_point, init_vals.angle, init_vals.velocity);

	return init_vals;
}