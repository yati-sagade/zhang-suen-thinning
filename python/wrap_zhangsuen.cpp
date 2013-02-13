#include <boost/python.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/numpy.hpp>
#include <boost/scoped_array.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <sstream>

#include "zhangsuen.h"

typedef unsigned char uchar_t;

namespace py = boost::python;
namespace np = boost::numpy;

/**
 * ndarray_to_mat(const np::ndarray& array, cv::Mat& result)
 *
 * Args:
 *  array: The input boost::numpy::ndarray. Must be 2D and the data type
 *         should be unsigned char. Also, the inner data alignment should be
 *         row major.
 *
 *  result: The resulting cv::Mat. It is assumed to be initialized to 
 *          have appropriate number of rows and columns
 * 
 * NOTE: The memory is NOT shared between the input ndarray and the resultant
 *       mat, unlike the reverse ndarray_to_mat() function.
 * 
 */
void ndarray_to_mat(const np::ndarray& array, cv::Mat& result)
{
    // Ensure dtype is unsigned char
    if (array.get_dtype() != np::dtype::get_builtin<uchar_t>())
    {
        np::dtype need_dtype = np::dtype::get_builtin<uchar_t>();
        std::stringstream ss;
        ss << "Incorrect data type: " 
           << "Expected: " << py::extract<const char*>(py::str(need_dtype))
           << ", Got: " << py::extract<const char*>(py::str(array.get_dtype()));
        PyErr_SetString(PyExc_TypeError, ss.str().c_str());
        py::throw_error_already_set();
    }
    // Ensure the array is 2D
    if (array.get_nd() != 2)
    {
        std::stringstream ss;
        ss << "Expecting a 2D array, got a " << array.get_nd() << " array.";
        PyErr_SetString(PyExc_TypeError, ss.str().c_str());
        py::throw_error_already_set();
    }
    // Ensure the array is row major
    if (!(array.get_flags() & np::ndarray::C_CONTIGUOUS))
    {
        PyErr_SetString(PyExc_TypeError, "Array must be row major contiguous");
        py::throw_error_already_set();
    }
    uchar_t *data = reinterpret_cast<uchar_t*>(array.get_data());
    int row_incr = array.strides(0) / sizeof(*data),
        col_incr = array.strides(1) / sizeof(*data);
    uchar_t *iter;
    int rows = array.shape(0), cols = array.shape(1);
    for (int i = 0; i < rows; i++)
    {
        iter = data + i * row_incr;
        uchar_t *row = result.ptr<uchar_t>(i);
        for (int j = 0; j < cols; j++)
        {
            *(row + j) = *(iter + j * col_incr); 
        }
    }
}

// Return an ndarray that holds the same information as the underlying cv::Mat
// NOTE: The memory is shared between the returned ndarray and the input image.
static np::ndarray mat_to_ndarray(const cv::Mat& image, bool share_mem=true)
{
    uchar_t* data = image.data;
    // Our images are all grayscale, so the datatype will be (the numpy
    // equivalent of) uchar_t
    np::dtype type(np::dtype::get_builtin<uchar_t>());
    // This is the shape of the resulting ndarray, the number of rows and the
    // number of columns.
    py::tuple shape = py::make_tuple(image.rows, image.cols);
    // Strides for rows and columns. Given a current position in a 2D array, 
    // the row stride is the number of bytes to be traversed from the current
    // position to get to the corresponding column in the next row. The column
    // stride is the number of bytes to be traversed from the current position
    // to get to the next element(column) in the same row.
    py::tuple strides = py::make_tuple(image.step[0] * sizeof(*data), 
                                       image.step[1] * sizeof(*data));
    // The owner of the resulting ndarray. Since we are sharing the memory used
    // by the |image| and the resulting ndarray, we need the resultant to hold
    // a reference to this piece of memory. Since the resultant array holds
    // a reference to its owner object, it is prevented from being deallocated
    // by the GC.
    if (share_mem) 
    {
        //TODO: This doesn't seem to work as the thinned image, when viewed
        //from Python, is all screwed up.
        np::ndarray array = np::from_data(data, type, shape, strides, py::object());
        return array;
    }
    else
    {
        np::ndarray array(np::zeros(shape, type));
        uchar_t *result_data = reinterpret_cast<uchar_t*>(array.get_data());
        int row_incr = array.strides(0) / sizeof(*data),
            col_incr = array.strides(1) / sizeof(*data);
        int mat_row_incr = image.step[0], mat_col_incr = image.step[1];
        for (int i = 0; i < image.rows; i++)
        {
            uchar_t *result_iter = result_data + i * row_incr;
            uchar_t *mat_iter = data + i * mat_row_incr;
            for (int j = 0; j < image.cols; j++, result_iter += col_incr, mat_iter += mat_col_incr)
            {
                *result_iter = *mat_iter;
            }
        }
        return array;
    }
}

void show(const std::string& window, const cv::Mat& mat)
{
    cv::namedWindow(window, CV_WINDOW_NORMAL);
    cv::imshow(window, mat);
    cv::waitKey(0);
}

void show(const std::string& window, const np::ndarray& array)
{
    cv::Mat img(array.shape(0), array.shape(1), CV_8U);
    ndarray_to_mat(array, img);
    show(window, img);
}


static np::ndarray wrap_thin(const np::ndarray& image,
                             bool need_boundary_smoothing=false,
                             bool need_acute_angle_emphasis=false,
                             bool destair=false)
{
    cv::Mat mat(image.shape(0), image.shape(1), CV_8U);
    ndarray_to_mat(image, mat);
    thin(mat, need_boundary_smoothing, need_acute_angle_emphasis, destair);
    return mat_to_ndarray(mat, false /* don't share memory */);
}

static const std::string docstring_thin( 
"Perform Zhang-Suen thinning on `image`.\n\n"
"Args: \n"
"    image(numpy.ndarray): The binarized image to thin. A black foreground on\n"
"                          a lighter background is expected.\n\n"

"    need_boundary_smoothing(bool): Whether to perform the Stentiford boundary\n"
"                                   smoothing operation as a preprocessing step\n"
"                                   or not. This operation is performed to reduce\n"
"                                   line fuzz. Default false.\n\n"

"    need_acute_angle_emphasis(bool): Whether to perform the acute-angle emphasis\n"
"                                     preprocessing operation or not. This\n"
"                                     operation is used to decrease the necking\n"
"                                     effect. Default false.\n\n"

"    destair(bool): Whether to perform the Holt\'s destair post-processing. This\n"
"                   reduces the staircase effect in oblique lines.\n"
"                   Default false.\n");



BOOST_PYTHON_MODULE(zhangsuen)
{
    // Initialize the Python runtime and the Boost.Numpy lib. 
    // This is a must! Otherwise we will get a segfault.
    Py_Initialize();
    np::initialize();
    py::docstring_options doc_options;
    doc_options.disable_cpp_signatures();
    py::def("thin", wrap_thin, docstring_thin.c_str());
}
