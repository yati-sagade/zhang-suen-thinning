This is an adaptation of the Zhang-Suen thinning algorithm and related
pre-processing and post-processing steps as presented in [Algorithms for Image
Processing and Computer Vision](http://www.amazon.com/Algorithms-Image-Processing-Computer-Vision/dp/0470643854)
by JR Parker.

There are two branches, the `master` branch contains just the required
functions, of which `thin()` is the one that must be called with appropriate
parameters. The other branch, `with-main`, contains a `main()` that takes
a path to an image on the command line, applies the thinning, and shows the
result. Do 
    
    git checkout with-main
    make
    ./zhangsuen chinese.png

to see the results.
