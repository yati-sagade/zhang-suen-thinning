#include "zhangsuen.h"

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
                int c = image.at<uchar_t>(i, j);
                if (!c) continue;
                int e = image.at<uchar_t>(i, j + 1),
                    ne = image.at<uchar_t>(i - 1, j + 1),
                    n = image.at<uchar_t>(i - 1, j),
                    nw = image.at<uchar_t>(i - 1, j - 1),
                    w = image.at<uchar_t>(i, j - 1),
                    sw = image.at<uchar_t>(i + 1, j - 1),
                    s = image.at<uchar_t>(i + 1, j),
                    se = image.at<uchar_t>(i + 1, j + 1);

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
void thin(cv::Mat& img, bool need_boundary_smoothing=false,
          bool need_acute_angle_emphasis=false, bool destair=false)
{
    // Make everything either 0 or 1. Though we expect a bilevel image, 
    // the high level (white) can also be 255 rather than 1.
    for (int i = 0; i < img.rows; i++)
    {
        uchar_t *iter = img.ptr<uchar_t>(i);
        for (int j = 0; j < img.cols; j++, iter++)
            *iter = (uchar_t)(*iter != 0);
    }
   
    // Stentiford Boundary smoothing to reduce line fuzz.
    if (need_boundary_smoothing)
        boundary_smooth(img);
    // Acute Angle Emphasis to curb necking.
    if (need_acute_angle_emphasis)
        acute_angle_emphasis(img);
    // If our input image is M x N, create a M + 2 x N + 2 image that is
    // essentially the same image, bordered by 1(white) pixels.
    cv::Mat image = cv::Mat::ones(img.rows + 2, img.cols + 2, CV_8U);
    for (int i = 0; i < img.rows; i++)
    {
        uchar_t *src_iter = img.ptr<uchar_t>(i),
                *dst_iter = image.ptr<uchar_t>(i + 1);
        // Advance from the 0th to the 1st column.
        ++dst_iter;
        for (int j = 0; j < img.cols; j++)
        {
            *dst_iter++ = *src_iter++;
        }
    }
    // The actual zhangsuen thinning procedure would like the black pixels to
    // be 1 and white pixels to be 0. So do that.
    for (int i = 0; i < image.rows; i++)
    {
        uchar_t *iter = image.ptr<uchar_t>(i);
        for (int j = 0; j < image.cols; j++, iter++)
        {
            if (*iter > 0)
                *iter = 0;
            else
                *iter = 1;
        }
    }
    // Now call the actual thinning routine.
    zhangsuen_thin(image);
    // Holt's staircase removal
    if (destair)
        remove_staircases(image);
    // Remember in the image we have, 1 means black and 0 means white. We need
    // to restore that to where 255 means white(background) and 0 means
    // black(object).
    for (int i = 0; i < img.rows; i++)
    {
        uchar_t *dst_iter = img.ptr<uchar_t>(i),
                *src_iter = image.ptr<uchar_t>(i + 1);
        ++src_iter;
        for (int j = 0; j < img.cols; j++)
        {
            *dst_iter++ = (*src_iter++ > 0) ? 0 : 255;
        }
    }
}
//-------------------------------------------------------------------------
void zhangsuen_thin(cv::Mat& image)
{
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
//-------------------------------------------------------------------------
bool match(const cv::Mat& image, const std::vector<uchar_t>& points,
           const std::vector<uchar_t>& values)
{
    bool m = true;
    for (std::vector<Point>::size_type i = 0; i < points.size(); i++)
    {
        if (values[i] == 2)
            continue;
        switch (values[i])
        {
        case 0:
            if (points[i] != 0)
                m = false;
            break;
        case 1:
            if (points[i] != 1)
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
    std::vector<uchar_t> points = {
    image.at<uchar_t>(r - 2, c - 2), image.at<uchar_t>(r - 2, c - 1),
    image.at<uchar_t>(r - 2, c), image.at<uchar_t>(r - 2, c + 1), 
    image.at<uchar_t>(r - 2, c + 2), image.at<uchar_t>(r - 1, c - 2), 
    image.at<uchar_t>(r - 1, c - 1), image.at<uchar_t>(r - 1, c),
    image.at<uchar_t>(r - 1, c + 1), image.at<uchar_t>(r - 1, c + 2),
    image.at<uchar_t>(r, c - 2), image.at<uchar_t>(r, c - 1),
    image.at<uchar_t>(r, c), image.at<uchar_t>(r, c + 1),
    image.at<uchar_t>(r, c + 2), image.at<uchar_t>(r + 1, c - 2),
    image.at<uchar_t>(r + 1, c - 1), image.at<uchar_t>(r + 1, c),
    image.at<uchar_t>(r + 1, c + 1), image.at<uchar_t>(r + 1, c + 2),
    image.at<uchar_t>(r + 2, c - 2), image.at<uchar_t>(r + 2, c - 1), 
    image.at<uchar_t>(r + 2, c), image.at<uchar_t>(r + 2, c + 1), 
    image.at<uchar_t>(r + 2, c + 2)
    };
    // 0 = zero
    // 1 = one 
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

