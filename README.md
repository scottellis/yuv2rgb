  yuv2rgb
=======

Convert a YUV image from the mt9p031 at one of the three supported sizes
into an rgb (bgr) image for display and also save a jpg version. 

The YUV -> BGR conversion seems non-optimal and takes some gain tweaking
to get a good picture. Nowhere near as nice as the raw Bayer images, but
some of that is to be expected.

The three supported sizes are 2560x1920, 1280x960 and 640x480. 

The program takes care of figuring out the size.

