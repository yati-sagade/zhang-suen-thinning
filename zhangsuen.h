#ifndef __ZHANGSUEN_H__
#define __ZHANGSUEN_H__

#include <set>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <iomanip>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

typedef std::pair<int, int> Point;
typedef unsigned char uchar_t;

int num_one_pixel_neighbours(const cv::Mat& image, const Point& point);

int num_zero_pixel_neighbours(const cv::Mat& image, const Point& point);

int connectivity(const cv::Mat& image, const Point& point);

int yokoi_connectivity(const cv::Mat& image, const Point& point);

void delete_pixels(const cv::Mat& image, const std::set<Point>& points);

void remove_staircases(cv::Mat& image);

void zhangsuen_thin(cv::Mat& img);

void thin(cv::Mat& img, bool need_boundary_smoothing, 
          bool need_acute_angle_emphasis, bool destair);

void boundary_smooth(cv::Mat& image);

void acute_angle_emphasis(cv::Mat& image);

bool match(const cv::Mat& image, const std::vector<Point>& points,
           const std::vector<uchar_t>& values);

bool match_templates(const cv::Mat& image, const Point& point, int k);

#endif
