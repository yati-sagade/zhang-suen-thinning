#include <set>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

typedef std::pair<int, int> Point;
typedef unsigned char uchar_t;
//-------------------------------------------------------------------------
void print_img(const cv::Mat& image)
{
    std::cout << std::endl;
    for (int i = 0; i < image.rows; i++)
    {
        for(int j = 0; j < image.cols; j++)
            std::cout << (int)image.at<uchar_t>(i, j) << " ";
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
//-------------------------------------------------------------------------
void display(const std::string& windowname, const cv::Mat& image)
{
    cv::namedWindow(windowname);
    cv::imshow(windowname, image);
}
//-------------------------------------------------------------------------
int num_one_pixel_neighbours(const cv::Mat& image, const Point& point)
{
    int ret = 0, r = point.first, c = point.second;
    for (int i = r - 1; i <= r + 1; i++)
        for (int j = c - 1; j <= c + 1; j++)
            if (i != r || j != c)
                if (image.at<uchar_t>(i, j) >= 1) ret++;
    return ret;
}
//-------------------------------------------------------------------------
int connectivity(const cv::Mat& image, const Point& point)
{
    int r = point.first, c = point.second, ret = 0;
    if (image.at<uchar_t>(r, c+1) >= 1 && image.at<uchar_t>(r-1, c+1) == 0)
        ret++;
    if (image.at<uchar_t>(r-1, c+1) >= 1 && image.at<uchar_t>(r-1, c) == 0)
        ret++;    
    if (image.at<uchar_t>(r-1, c) >= 1 && image.at<uchar_t>(r-1, c-1) == 0)
        ret++;
    if (image.at<uchar_t>(r-1, c-1) >= 1 && image.at<uchar_t>(r, c-1) == 0)
        ret++;
    if (image.at<uchar_t>(r, c-1) >= 1 && image.at<uchar_t>(r+1, c-1) == 0)
        ret++;
    if (image.at<uchar_t>(r+1, c-1) >= 1 && image.at<uchar_t>(r+1, c) == 0)
        ret++;
    if (image.at<uchar_t>(r+1, c) >= 1 && image.at<uchar_t>(r+1, c+1) == 0)
        ret++;
    if (image.at<uchar_t>(r+1, c+1) >= 1 && image.at<uchar_t>(r, c+1) == 0)
        ret++;
    return ret;
}
//-------------------------------------------------------------------------
void delete_pixels(cv::Mat& image, std::set<Point>& points)
{
    for (auto it = points.begin(); it != points.end(); it++)
    {
        image.at<uchar_t>(it->first, it->second) = 0;
    }
    points.clear();
}
//-------------------------------------------------------------------------
void thin(cv::Mat& image)
{
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            if (image.at<uchar_t>(i, j) > 0)
                image.at<uchar_t>(i, j) = 0;
            else
                image.at<uchar_t>(i, j) = 1;
        }
    }

    while (true)
    {
        std::set<Point> points;
        for (int i = 1; i < image.rows - 1; i++)
        {
            for (int j = 1; j < image.cols - 1; j++)
            {
                if (image.at<uchar_t>(i, j) != 1) continue;
                Point p(i, j);
                int k = num_one_pixel_neighbours(image, p);
                if ((k >= 2 && k <= 6) && connectivity(image, p) == 1)
                {
                    int p1 = image.at<uchar_t>(i, j + 1) * 
                             image.at<uchar_t>(i - 1, j) *
                             image.at<uchar_t>(i, j - 1),

                        p2 = image.at<uchar_t>(i - 1, j) *
                             image.at<uchar_t>(i + 1, j) *
                             image.at<uchar_t>(i, j - 1);

                    if (p1 == 0 && p2 == 0)
                    {
                        points.insert(p);
                    }
                }
            }
        }
        if (points.size() == 0)
            break;
        delete_pixels(image, points);
        for (int i = 1; i < image.rows; i++)
        {
            for(int j = 1; j < image.cols; j++)
            {
                if(image.at<uchar_t>(i, j) != 1) continue;
                Point p(i, j);
                int k = num_one_pixel_neighbours(image, p);
                if ((k >= 2 && k <= 6) && connectivity(image, p) == 1)
                {
                    int p1 = image.at<uchar_t>(i - 1, j) *
                             image.at<uchar_t>(i, j + 1) *
                             image.at<uchar_t>(i + 1, j),

                        p2 = image.at<uchar_t>(i, j + 1) *
                             image.at<uchar_t>(i + 1, j) *
                             image.at<uchar_t>(i, j - 1);
                    if (p1 == 0 && p2 == 0)
                    {
                        points.insert(p);
                    }
                }
            }
        }
        if (points.size() == 0)
            break;
        delete_pixels(image, points);
    }

    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            if (image.at<uchar_t>(i, j) > 0)
                image.at<uchar_t>(i, j) = 0;
            else
                image.at<uchar_t>(i, j) = 255;
        }
    }
}
//-------------------------------------------------------------------------
int main(int argc, char *argv[]) 
{
    // cv::Mat image = (cv::Mat_<uchar_t>(3, 3) << 0, 1, 1, 
    //                                             0, 1, 0, 
    //                                             1, 1, 1);
    // std::cout << num_one_pixel_neighbours(image, {1, 1}) << std::endl;
    // std::cout << connectivity(image, {1, 1}) << std::endl;
    // print_img(image);
    // std::set<Point> points = {
    //     {0, 0}, 
    //     {1, 0},
    //     {1, 1},
    //     {2, 2}
    // };
    // std::cout << "passing to thin()" << std::endl;
    // thin(image);
    // std::cout << "Deleting a few points" << std::endl;
    // delete_pixels(image, points);
    // print_img(image);
    // std::cout << "again passing to thin()" << std::endl;
    // thin(image);
    cv::Mat image = cv::imread(argv[1], 0);
    cv::Mat dst = image.clone();    
    thin(dst);
    display("Original", image);
    display("Thinned", dst);
    cv::waitKey(0);
    return 0;
}

