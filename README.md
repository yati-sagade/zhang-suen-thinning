This is an adaptation of the Zhang-Suen thinning algorithm and related
pre-processing and post-processing steps as presented in [Algorithms for Image
Processing and Computer Vision](http://www.amazon.com/Algorithms-Image-Processing-Computer-Vision/dp/0470643854)
by JR Parker.


Prerequisites
--------------
### For only C++
- [OpenCV][3]

### For Python support
- [OpenCV][3], built with Python bindings.
- [Boost::Python][1]
- [Boost.Numpy][2]


    $ git clone https://github.com/yati-sagade/zhang-suen-thinning.git
    $ cd zhang-suen-thinning

### Building

The library exposing the function that performs Zhang-Suen thinning can be
build in two ways. For use in C++ programs, the static llibrary
`libzhangsuen.a` can be produced by going into the toplevel directory for the
project and typing
    
    $ make

This will produce `libzhangsuen.a` in the current directory, which can then be
used to link with C++ programs using the library.

For use with Python, OpenCV must be installed, built with Python support. Then,
the Python extension can be built by doing

    $ make python

This will produce `zhangsuen.so` in `python/`, which is an extension directly
importable from Python:

    $ cd python
    $ python
    Python 2.7.3 (default, Sep  9 2012, 17:41:34) 
    [GCC 4.7.1] on linux2
    Type "help", "copyright", "credits" or "license" for more information.
    >>> import zhangsuen
    >>> zhangsuen.thin
    <Boost.Python.function object at 0x12f67e0>
    >>>

### Reference

#### C++

    void thin(cv::Mat& img,
              bool need_boundary_smoothing=false,
              bool need_acute_angle_emphasis=false,
              bool destair=false)

#### Python
    zhangsuen.thin(img,
                   need_boundary_smoothing,
                   need_acute_angle_emphasis,
                   destair) -> numpy.ndarray

#### Arguments

    Perform Zhang-Suen thinning, optionally with some pre and post processing.

    img: The input grayscale(ideally binarized) image (A `numpy.ndarray` in
         Python). Thinning is performed in place in C++ and a new `numpy.ndarray`
         object holding the thinned image is returned in Python. 

    need_boundary_smoothing: Whether or not to perform the Stentiford Boundary
                             smoothing pre processing to reduce line fuzz. Line
                             fuzz is the name given to spurious projections and
                             extra lines on the skeleton boundaries.

    need_acute_angle_emphasis: Whether or not to perform acute-angle emphasis
                               to reduce necking. 

                               Sometimes, a narrow point at the intersection of
                               lines is stretched into a small line segment.
                               This is called "necking".
                               This type of artifacts can be reduced by setting
                               all the black points that tend to choke an acute
                               angle joint to white. This operation is known as
                               acute-angle emphasis.

    destair: Whether to perform Holt\'s destairing pre processing operation.

             Sometimes, after thinning, there still are pixels that could be
             removed. Many of such removable pixels form staircases in oblique
             lines. Holt\'s post processing, called destairing, attempts to
             delete such points which, when removed, do not affect the
             connectivity or shape information of the skeleton.

[1]: http://www.boost.org/doc/libs/1_53_0/libs/python/doc/
[2]: https://github.com/ndarray/Boost.NumPy
[3]: http://opencv.org/
