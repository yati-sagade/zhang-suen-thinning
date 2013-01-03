#include <vector>
#include <utility>
#include <string>
#include <algorithm>
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

typedef unsigned char uchar_t;
typedef std::pair<int, int> Point;
typedef cv::Mat Image;


void display(const std::string& window_title, const cv::Mat& image) {
    cv::namedWindow(window_title, CV_WINDOW_NORMAL);
    cv::imshow(window_title, image);
}

void delete_pixels(Image& image, Image& deletion_map) {
    for (int i = 0; i < deletion_map.rows; i++) {
        for (int j = 0; j < deletion_map.rows; j++) {
            if (deletion_map.at<uchar_t>(i, j))
                image.at<uchar_t>(i, j) = 0;
        }
    }
}

int connectivity(const Image& image, const Point& point) {
    int ret = 0;
    int r = point.first;
    int c = point.second;
    std::vector<uchar_t> pixels = {
        image.at<uchar_t>(r, c+1),
        image.at<uchar_t>(r-1, c+1),
        image.at<uchar_t>(r-1, c),
        image.at<uchar_t>(r-1, c-1),
        image.at<uchar_t>(r, c-1),
        image.at<uchar_t>(r+1, c-1),
        image.at<uchar_t>(r+1, c),
        image.at<uchar_t>(r+1, c+1)
    };
    for (int i = 0; i < 8; i++) {
        if (pixels[i] >= 1 && pixels[(i+1) % 8] == 0)
            ret++;
    }
    return ret;
}

int num_one_neighbours(const Image& image, const Point& point) {
    int r = point.first, c = point.second;
    std::vector<uchar_t> pixels = {
        image.at<uchar_t>(r, c+1),
        image.at<uchar_t>(r-1, c+1),
        image.at<uchar_t>(r-1, c),
        image.at<uchar_t>(r-1, c-1),
        image.at<uchar_t>(r, c-1),
        image.at<uchar_t>(r+1, c-1),
        image.at<uchar_t>(r+1, c),
        image.at<uchar_t>(r+1, c+1)
    };
    return count_if(pixels.begin(), pixels.end(), 
                    [](uchar_t value){
                        return value >= 1;
                    });
}

void thin(Image& image) {
    bool again = false;

    // Convert the image such that black pixels == 1 and white pixels == 0
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            auto& pixel = image.at<uchar_t>(i, j);
            pixel = (pixel >= 1) ? 0 : 1;
        }
    }
    do {
        again = false;
        // A map to keep track of the pixels that can be set to background colour.
        cv::Mat to_delete = cv::Mat::zeros(image.rows, image.cols, CV_8U);
        // First sub-iteration
        for (int i = 1; i < image.rows - 1; i++) {
            for (int j = 1; j < image.cols - 1; j++) {
                Point point = {i, j};
                if (image.at<uchar_t>(i, j) == 0)
                    continue;
                int k = num_one_neighbours(image, point);
                if ((connectivity(image, point) == 1) &&

                    ((k >= 2) && (k <= 6)) &&

                    (image.at<uchar_t>(i, j+1) *
                     image.at<uchar_t>(i-1, j) * 
                     image.at<uchar_t>(i, j-1) == 0) &&

                    (image.at<uchar_t>(i-1, j) *
                     image.at<uchar_t>(i+1, j) *
                     image.at<uchar_t>(i, j-1) == 0))
                {
                    again = true;
                    to_delete.at<uchar_t>(i, j) = 1;
                }
            }
        }
        if (!again)
            break;
        delete_pixels(image, to_delete);
        // Second sub-iteration
        for (int i = 1; i < image.rows - 1; i++) {
            for (int j = 1; j < image.cols - 1; j++) {
                Point point = {i, j};
                if (image.at<uchar_t>(i, j) == 0)
                    continue;
                int k = num_one_neighbours(image, point);
                if ((connectivity(image, point) == 1) &&

                    ((k >= 2) && (k <= 6)) &&

                    (image.at<uchar_t>(i-1, j) *
                     image.at<uchar_t>(i, j+1) * 
                     image.at<uchar_t>(i+1, j) == 0) &&

                    (image.at<uchar_t>(i, j+1) *
                     image.at<uchar_t>(i+1, j) *
                     image.at<uchar_t>(i, j-1) == 0))
                {
                    again = true;
                    to_delete.at<uchar_t>(i, j) = 1;
                }
            }
        }
        delete_pixels(image, to_delete);
    } while (again);

    // Restore image levels
    for (int i = 0; i < image.rows; i++)
        for (int j = 0; j < image.cols; j++) {
            auto& pixel = image.at<uchar_t>(i, j);
            pixel = (pixel >= 1) ? 0 : 255;
        }
}

int main(int argc, char *argv[]) {
    cv::Mat image = cv::imread(argv[1], 0);
    cv::Mat dst = image.clone();
    thin(dst);
    display("Original", image);
    display("Thinned", dst);
    cv::waitKey(0);
    return 0;
}
