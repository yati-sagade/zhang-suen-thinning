#include "zhangsuen.h"

//-------------------------------------------------------------------------
void print_img(const cv::Mat& image)
{
    std::cout << std::endl;
    auto old = std::cout.width();
    for (int i = 0; i < image.rows; i++)
    {
        for(int j = 0; j < image.cols; j++)
            std::cout << std::setw(4) << std::setiosflags(std::ios::right)
                      << (int)image.at<uchar_t>(i, j);
        std::cout << std::endl;
    }
    std::cout << std::setw(old) << std::endl;
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
int num_zero_pixel_neighbours(const cv::Mat& image, const Point& point)
{
    int ret = 0, r = point.first, c = point.second;
    for (int i = r - 1; i <= r + 1; i++)
        for (int j = c - 1; j <= c + 1; j++)
            if (i != r || j != c)
                if (image.at<uchar_t>(i, j) == 0) ret++;
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
int yokoi_connectivity(const cv::Mat& image, const Point& point)
{
    int r = point.first, c = point.second;
    // We use x != 0 to map all zero values to 0 and all non-zero values
    // to 1.
    std::vector<int> N = {
        image.at<uchar_t>(r, c + 1) != 0,
        image.at<uchar_t>(r - 1, c + 1) != 0,
        image.at<uchar_t>(r - 1, c) != 0,
        image.at<uchar_t>(r - 1, c - 1) != 0,
        image.at<uchar_t>(r, c - 1) != 0,
        image.at<uchar_t>(r + 1, c - 1) != 0,
        image.at<uchar_t>(r + 1, c) != 0,
        image.at<uchar_t>(r + 1, c + 1) != 0
    };
    int ret = 0;
    for (std::vector<int>::size_type i = 1; i < N.size(); i += 2)
    {
        int i1 = (i + 1) % 8, i2 = (i + 2) % 8;
        ret += N[i] - N[i] * N[i1] * N[i2];
    }
    return ret;
}
//-------------------------------------------------------------------------
void delete_pixels(cv::Mat& image, std::set<Point>& points)
{
    for (auto& point : points)
    {
        image.at<uchar_t>(point.first, point.second) = 0;
    }
    points.clear();
}
//-------------------------------------------------------------------------
void remove_staircases(cv::Mat& image)
{
    std::set<Point> points;
    for (int i = 0; i < 2; i++)
    {
        for (int i = 1; i < image.rows - 1; i++)
        {
            for (int j = 1; j < image.cols - 1; j++)
            {
                int e = image.at<uchar_t>(i, j + 1),
                    ne = image.at<uchar_t>(i - 1, j + 1),
                    n = image.at<uchar_t>(i - 1, j),
                    nw = image.at<uchar_t>(i - 1, j - 1),
                    w = image.at<uchar_t>(i, j - 1),
                    sw = image.at<uchar_t>(i + 1, j - 1),
                    s = image.at<uchar_t>(i + 1, j),
                    se = image.at<uchar_t>(i + 1, j + 1),
                    c = image.at<uchar_t>(i, j);
                if (i == 0)
                {
                    // North biased staircase removal
                    if (!(c && !(n && 
                                 ((e && !ne && !sw && (!w || !s)) || 
                                  (w && !nw && !se && (!e || !s))))))
                    {
                        points.insert({i, j}); 
                    }
                }
                else
                {
                    // South bias staircase removal
                    if (!(c && !(s && 
		                         ((e && !se && !nw && (!w || !n)) || 
		                          (w && !sw && !ne && (!e || !n))))))
                    {
                        points.insert({i, j});
                    }
	
                }
            }
        }
        delete_pixels(image, points);
    }
}
//-------------------------------------------------------------------------
void thin(cv::Mat& img, bool destair=false)
{
    cv::Mat image = cv::Mat::ones(img.rows + 2, img.cols + 2, CV_8U);
    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            image.at<uchar_t>(i + 1, j + 1) = img.at<uchar_t>(i, j);
        }
    }
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

    if (destair)
        remove_staircases(image);

    for (int i = 0; i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            if (image.at<uchar_t>(i + 1, j + 1) > 0)
                img.at<uchar_t>(i, j) = 0;
            else
                img.at<uchar_t>(i, j) = 255;
        }
    }
}
//-------------------------------------------------------------------------
// The Stentiford boundary smoothing to reduce line fuzz after thinning.
// This is expected to be called with BLACK = 0 and white = nonzero.
void boundary_smooth(cv::Mat& image)
{
    std::set<Point> points;
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            Point point(i, j);
            if (image.at<uchar_t>(i, j) == 0)
            {
                // Mark all black points that have two or fewer black
                // neighbours and a Yokoi connectivity of less than 2.
                if (num_zero_pixel_neighbours(image, point) <= 2 &&
                    yokoi_connectivity(image, point) < 2)
                {
                    points.insert(point);
                }
            }
        }
    }
    // Set all the marked points to white
    for (auto& point : points)
    {
        image.at<uchar_t>(point.first, point.second) = 1;
    }
}
//-------------------------------------------------------------------------
void acute_angle_emphasis(cv::Mat& image)
{
    for (int k = 5; k >= 1; k -= 2)
    {
        std::set<Point> points;
        for (int i = 2; i < image.rows - 2; i++)
        {
            for (int j = 2; j < image.cols - 2; j++)
            {
                if (image.at<uchar_t>(i, j) == 0)
                {
                    Point point(i, j);
                    if (match_templates(image, point, k))
                    {
                        image.at<uchar_t>(i, j) = 2;
                        points.insert(point);
                    }
                }
            }
        }
        if (points.empty())
            break;
        for (auto& point : points)
        {
            image.at<uchar_t>(point.first, point.second) = 1;
        }
        points.clear();
    }
}
//------------------------------------------------------------------------
void print_vec(const std::vector<uchar_t>& values)
{
    for (int i = 0; i < 25; i++)
    {
            if (i % 5 == 0) std::cout << std::endl;
            std::cout << (int)values[i] << " ";
    }
    std::cout << std::endl;
}
//-------------------------------------------------------------------------
bool match(const cv::Mat& image, const std::vector<Point>& points,
           const std::vector<uchar_t>& values)
{
    bool m = true;
    for (std::vector<Point>::size_type i = 0; i < points.size(); i++)
    {
        if (values[i] == 2)
            continue;
        Point pt = points[i];
        switch (values[i])
        {
        case 0:
            if (image.at<uchar_t>(pt.first, pt.second) != 0)
                m = false;
            break;
        case 1:
            if (image.at<uchar_t>(pt.first, pt.second) != 1)
                m = false;
            break;
        }
        if (!m) break;
    }
    return m;
}
//-------------------------------------------------------------------------
bool match_templates(const cv::Mat& image, const Point& point, int k)
{
    int r = point.first, c = point.second;
    std::vector<Point> points = {
    {r - 2, c - 2}, {r - 2, c - 1}, {r - 2, c}, {r - 2, c + 1}, {r - 2, c + 2},
    {r - 1, c - 2}, {r - 1, c - 1}, {r - 1, c}, {r - 1, c + 1}, {r - 1, c + 2},
    {r, c - 2}, {r, c - 1}, {r, c}, {r, c + 1}, {r, c + 2},
    {r + 1, c - 2}, {r + 1, c - 1}, {r + 1, c}, {r + 1, c + 1}, {r + 1, c + 2},
    {r + 2, c - 2}, {r + 2, c - 1}, {r + 2, c}, {r + 2, c + 1}, {r + 2, c + 2}
    };
    // 0 = zero
    // 1 = nonzer0
    // 2 = don't care
    // D1
    std::vector<uchar_t> values = {
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        2, 0, 0, 0, 2
    };
    if (match(image, points, values))
    {
        return true;
    }
    // D2
    if (k >= 2)
    {
        values[1] = 1;
        if (match(image, points, values))
        {
            return true;
        }
    }
    // D3
    if (k >= 3)
    {
        values[1] = 0;
        values[3] = 1;
        if (match(image, points, values))
        {
            return true;
        }
    }
    // D4 
    if (k >= 4)
    {
        values[3] = 0;
        values[1] = values[6] = 1;
        if (match(image, points, values))
        {
            return true;
        }
    }
    // D5
    if (k >= 5)
    {
        values[1] = values[6] = 0;
        values[3] = values[8] = 1;
        if (match(image, points, values))
        {
            return true;
        }
    }
    // U
    values = {
        2, 0, 0, 0, 2,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0
    };
    // U1
    if (match(image, points, values))
    {
        return true;
    }
    // U2
    if (k >= 2)
    {
        values[21] = 1;
        if (match(image, points, values))
        {
            return true;
        }
    }
    // U3
    if (k >= 3)
    {
        values[21] = 0;
        values[23] = 1;
        if (match(image, points, values))
        {
            return true;
        }
    }
    // U4
    if (k >= 4)
    {
        values[23] = 0;
        values[16] = values[21] = 1;
        if (match(image, points, values))
        {
            return true;
        }
    }
    // U5
    if (k >= 5)
    {
        values[16] = values[21] = 0;
        values[18] = values[23] = 1;
        if (match(image, points, values))
        {
            return true;
        }

    }
    return false;
}
//-------------------------------------------------------------------------
int count_pixels(const cv::Mat& image, uchar_t value)
{
    int ret = 0;
    for (int i = 0; i < image.rows; i++)
        for (int j = 0; j < image.cols; j++)
            if (image.at<uchar_t>(i, j) == value)
                ret++;
    return ret;
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
    // cv::Mat image = (cv::Mat_<uchar_t>(6, 6) << 0, 0, 0, 1, 1, 1,
    //                                             1, 0, 0, 0, 1, 1,
    //                                             1, 1, 0, 0, 0, 1,
    //                                             1, 1, 1, 0, 0, 0,
    //                                             0, 0, 0, 0, 0, 0,
    //                                             0, 0, 0, 0, 0, 0);
    // 
    // print_img(image);
    // thin(image);
    // print_img(image);
    //
    cv::Mat image = cv::imread(argv[1], 0);
    for (int i = 0; i < image.rows; i++)
        for (int j = 0; j < image.cols; j++)
            image.at<uchar_t>(i, j) = (uchar_t)(image.at<uchar_t>(i, j) != 0);
    cv::Mat dst = image.clone();    
    acute_angle_emphasis(dst);
    boundary_smooth(dst);
    thin(dst, true);
    std::cout << "Number of black pixels" << std::endl
              << "\t in the original image: " << count_pixels(image, 0)
              << std::endl << "\t in the resultant" << count_pixels(dst, 0)
              << std::endl;
    display("Original", image);
    display("Thinned", dst);
    cv::waitKey(0);
    // cv::Mat image = cv::Mat::zeros(5, 5, CV_8U);
    // for (int i = 0; i < 5; i++)
    //     for (int j = 0; j < 5; j++)
    //     {
    //         unsigned v;
    //         std::cin >> v;
    //         image.at<uchar_t>(i, j) = (uchar_t) v;
    //     }
    // match_templates(image, {2, 2}, 5);
    return 0;
}

