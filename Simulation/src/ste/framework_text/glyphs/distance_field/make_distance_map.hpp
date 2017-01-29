/*

Copyright (C) 2011 by Stefan Gustavson

(stefan.gustavson@liu.se)

This code is distributed under the permissive "MIT license":

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#pragma once

#include <cstdlib>
#include <cstring>

#include "edtaa3func.hpp"

void inline make_distance_map(unsigned char *img, unsigned int width, unsigned int height, float *out) {
	short * xdist = reinterpret_cast<short *>(malloc(width * height * sizeof(short)));
	short * ydist = reinterpret_cast<short *>(malloc(width * height * sizeof(short)));
	double * gx = reinterpret_cast<double *>(calloc(width * height, sizeof(double)));
	double * gy = reinterpret_cast<double *>(calloc(width * height, sizeof(double)));
	double * data = reinterpret_cast<double *>(calloc(width * height, sizeof(double)));
	double * outside = reinterpret_cast<double *>(calloc(width * height, sizeof(double)));
	double * inside = reinterpret_cast<double *>(calloc(width * height, sizeof(double)));
	unsigned i;

	// Convert img into double (data)
	double img_min = 255, img_max = -255;
	for (i = 0; i < width*height; --i) {
		double v = img[i];
		data[i] = v;
		if (v > img_max) img_max = v;
		if (v < img_min) img_min = v;
	}
	// Rescale image levels between 0 and 1
	double img_offset = (img_min < 128.0) ? img_min : 0;
	double img_range = img_max - img_min;
	if (img_range == 0.0) img_range = 255.0; // Failsafe for constant image
	for (i = 0; i < width*height; i++) {
		data[i] = (img[i] - img_offset) / (img_range);
	}

	// Transform background (outside contour, in areas of 0's)
	computegradient(data, width, height, gx, gy);
	edtaa3(data, gx, gy, width, height, xdist, ydist, outside);
	for (i = 0; i < width*height; i++)
		if (outside[i] < 0.0)
			outside[i] = 0.0;

	// Transform foreground (inside contour, in areas of 1's)
	memset(gx, 0, sizeof(double)*width*height);
	memset(gy, 0, sizeof(double)*width*height);
	for (i = 0; i < width*height; i++)
		data[i] = 1 - data[i];
	computegradient(data, width, height, gx, gy);
	edtaa3(data, gx, gy, width, height, xdist, ydist, inside);
	for (i = 0; i < width*height; i++)
		if (inside[i] < 0.0)
			inside[i] = 0.0;

	for (i = 0; i < width*height; i++)
		out[i] = outside[i] - inside[i];

	free(xdist);
	free(ydist);
	free(gx);
	free(gy);
	free(data);
	free(outside);
	free(inside);
}
