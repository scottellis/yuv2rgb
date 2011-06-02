/*
  Convert a packed YUV image from the mt9p031 sensor at full size into 
  an rgb (bgr) image for display and also save a jpg version of it. 
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CV_NO_BACKWARD_COMPATIBILITY
#include <cv.h>
#include <highgui.h>


IplImage *load_raw_image(const char *s);
char *load_raw_data(const char *s, int size);
IplImage *convert_image(IplImage *bayer);
void save_image(IplImage *img, const char *orig_file);

int main(int argc, char **argv)
{
	IplImage *yuv, *rgb;

	if (argc < 2) {
		printf("Usage: %s <image file>\n", argv[0]);
		exit(1);
	}

	yuv = load_raw_image(argv[1]);

	if (!yuv) {
		printf("Failed to load raw image %s\n", argv[1]);
		exit(1);
	}

	rgb = convert_image(yuv);

	if (rgb) {
		cvNamedWindow("main", CV_WINDOW_AUTOSIZE);
		cvMoveWindow("main", 10, 10);
		cvShowImage("main", rgb);
		cvWaitKey(0);
		cvDestroyWindow("main");	

		save_image(rgb, argv[1]);
	}
	
	cvReleaseImage(&yuv); 

	if (rgb)
		cvReleaseImage(&rgb);

	return 0;
}

void save_image(IplImage *img, const char *orig_file)
{
	char *p;
	char *s = strdup(orig_file);
	
	if (!s)
		return;

	p = strrchr(s, '.');

	if (!p)
		return;

	strcpy(p, ".jpg");

	cvSaveImage(s, img, 0);

	free(s);
}

IplImage *convert_image(IplImage *yuv)
{
	IplImage *dst;
	
	dst = cvCreateImage(cvGetSize(yuv), IPL_DEPTH_8U, 3);

	if (!dst) 
		printf("cvCreateImage - dst - failed\n");
	else {
		cvCvtColor(yuv, dst, CV_YCrCb2BGR);
	}

	return dst;
}

int get_filesize(const char *s, CvSize *sz)
{
	struct stat st;

	memset(&st, 0, sizeof(st));

	if (stat(s, &st)) {
		perror("stat");
		return 0;
	}

	// there are only three sizes we support
	if (st.st_size == (2 * 2560 * 1920)) {
		sz->width = 2560;
		sz->height = 1920;
		return 1;
	}
	else if (st.st_size == (2 * 1280 * 960)) {
		sz->width = 1280;
		sz->height = 960;
		return 1;
	}
	else if (st.st_size == (2 * 640 * 480)) {
		sz->width = 640;
		sz->height = 480;
		return 1;
	}
	else if (st.st_size == 155648) {
		sz->width = 320;
		sz->height = 240;
		return 1;
	}

	return 0;
}

IplImage *load_raw_image(const char *s)
{
	char *raw;
	IplImage *y, *u, *v, *img;
	int i, j, n;
	CvSize sz;

	if (!get_filesize(s, &sz))
		return NULL;

	n = sz.width * sz.height * 2;

	raw = load_raw_data(s, n);
	
	if (!raw)
		return NULL;

	img = NULL;

	y = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	u = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	v = cvCreateImage(sz, IPL_DEPTH_8U, 1);
	img = cvCreateImage(sz, IPL_DEPTH_8U, 3);

	if (!y || !u || !v || !img)
		goto load_raw_image_done;

	for (i = 0, j = 0; j < n; i++, j += 2)
		y->imageData[i] = raw[j];

	for (i = 0, j = 1; j < n; i += 2, j += 4) {
		u->imageData[i] = raw[j];
		u->imageData[i+1] = raw[j];
	}	

	for (i = 0, j = 3; j < n; i += 2, j += 4) {
		v->imageData[i] = raw[j];
		v->imageData[i+1] = raw[j];
	}	
	
	cvMerge(y, v, u, NULL, img);

load_raw_image_done:

	if (y)	
		cvReleaseImage(&y);

	if (u)
		cvReleaseImage(&u);

	if (v)	
		cvReleaseImage(&v);
	
	if (raw)
		free(raw);

	return img;
}

char *load_raw_data(const char *s, int size)
{
	int fd, len;
	char *dat = malloc(size);

	if (!dat) {
		perror("malloc");
		return NULL;
	}

	fd = open(s, O_RDONLY);

	if (fd < 0) {
		perror("open");
		free(dat);
		return NULL;
	}

	len = read(fd, dat, size);

	if (len != size) {
		printf("read tried %d got %d\n", size, len);
		free(dat);
		return NULL;
	}

	close(fd);

	return dat;
}

