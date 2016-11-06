
#include "gimage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>

#include "rawimage.h"
#include "jpegimage.h"
#include "tiffimage.h"


//Constructor/Destructor:

gImage::gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, unsigned bits)
{
	img = (pix *) malloc(width*height*sizeof(pix));
	w=width;
	h=height;
	c=colors;

	if (bits == 16) {
		unsigned short * src = (unsigned short *) imagedata;
		for (unsigned y=0; y<h; y++) {
			pix * dst = (pix *) (img + w*y);
			for (unsigned x=0; x<w; x++) {
				dst[x].r =  (unsigned short) src[0]/256;
				dst[x].g =  (unsigned short) src[1]/256;
				dst[x].b = (unsigned short) src[2]/256;
				src += 3;
			}
		}
	}					

	if (bits == 8) {
		char * src = (char *) imagedata;
		for (unsigned y=0; y<height; y++) {
			pix * dst = (pix *) (img + w*y);
			for (unsigned x=0; x<width; x++) {
				dst[x].r = (unsigned char) src[0];
				dst[x].g = (unsigned char) src[1];
				dst[x].b = (unsigned char) src[2];
				src += 3;
			}
		}
	}
	if (bits == 0) img = (pix *) imagedata; //supplied by caller
}


gImage::gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, unsigned bits, std::map<std::string,std::string> imageinfo)
{
	img = (pix *) malloc(width*height*sizeof(pix));
	w=width;
	h=height;
	c=colors;

	if (bits == 16) {
		unsigned short * src = (unsigned short *) imagedata;
		for (unsigned y=0; y<h; y++) {
			pix * dst = (pix *) (img + w*y);
			for (unsigned x=0; x<w; x++) {
				dst[x].r = (unsigned short) src[0]/256;
				dst[x].g = (unsigned short) src[1]/256;
				dst[x].b = (unsigned short) src[2]/256;
				src += 3;
			}
		}
	}					

	if (bits == 8) {
		char * src = (char *) imagedata;
		for (unsigned y=0; y<height; y++) {
			pix * dst = (pix *) (img + w*y);
			for (unsigned x=0; x<width; x++) {
				dst[x].r = (unsigned char) src[0];
				dst[x].g = (unsigned char) src[1];
				dst[x].b = (unsigned char) src[2];
				src += 3;
			}
		}
	}
	if (bits == 0) img = (pix *) imagedata; // imagedata is already pix *

	imginfo = imageinfo;

}

gImage::gImage(unsigned width, unsigned height, unsigned colors, std::map<std::string,std::string> imageinfo)
{
	img = (pix *) malloc(width*height*sizeof(pix));
	w=width;
	h=height;
	c=colors;
	for (unsigned y=0; y<height; y++) {
		pix * dst = (pix *) (img + w*y);
		for (unsigned x=0; x<width; x++) {
			dst[x].r = 0.0;
			dst[x].g = 0.0;
			dst[x].b = 0.0;
		}
	}
	imginfo = imageinfo;
}


gImage::~gImage()
{
	free(img);
}


//Getters:


gImage * gImage::Copy()
{
	pix * i = (pix *) malloc(w*h*sizeof(pix));
	memcpy(i, img, w*h*sizeof(pix));
	std::map<std::string,std::string> info = imginfo;
	return new gImage((char *)i, w, h, c, 0, info);
}


char * gImage::getImageData(unsigned bits)
{
	char *imagedata = (char*) malloc(w*h*c*(bits/8));

	if (bits == 16) {
		unsigned short * dst = (unsigned short *) imagedata;
		for (unsigned y=0; y<h; y++) {
			pix * src = (pix *) (img + w*y);
			for (unsigned x=0; x<w; x++) {
				dst[0] = (unsigned short) floor(fmin(fmax(src[x].r*256.0,0.0),65535.0)); 
				dst[1] = (unsigned short) floor(fmin(fmax(src[x].g*256.0,0.0),65535.0));
				dst[2] = (unsigned short) floor(fmin(fmax(src[x].b*256.0,0.0),65535.0)); 
				dst += 3;
			}
		}
	}

	if (bits == 8) {
		char * dst = (char *) imagedata;
		for (unsigned y=0; y<h; y++) {
			pix * src = (pix *) (img + w*y);
			for (unsigned x=0; x<w; x++) {
				dst[0] = (unsigned char) floor(fmin(fmax(src[x].r,0.0),255.0)); 
				dst[1] = (unsigned char) floor(fmin(fmax(src[x].g,0.0),255.0));
				dst[2] = (unsigned char) floor(fmin(fmax(src[x].b,0.0),255.0)); 
				dst += 3;
			}
		}
	}
	return imagedata;
}

pix * gImage::getImageData()
{
	return img;
}

unsigned gImage::getWidth()
{
	return w;
}

unsigned gImage::getHeight()
{
	return h;
}

unsigned gImage::getColors()
{
	return c;
}

std::map<std::string,std::string> gImage::getInfo()
{
	return imginfo;
}

int gImage::ThreadCount()
{
#if defined(_OPENMP)
	return omp_get_max_threads();
#else
	return 1;
#endif
}

// Setters:

void gImage::setInfo(std::string name, std::string value)
{
	imginfo[name] = value;
}




//Image Operations:
//
//This is a compendium of basic image operations, kept simple for 
//ease of understanding while maintaining basic image quality




//Convolution Kernels and Sharpening
//
//Credit: Various
//
//Changing a pixel depending on the values of its neighbors is probably the
//second fundamental image transformation to understand, after so-called
//curve-based lookup tables.  Particularly, there are a set of matrices
//describing weights of the center pixel and its neigbors that produce
//various effects, such as blurring and edge highlighting.  The general
//algorith traverses the image and calculates a new value for each pixel
//based on the sum of the weighted values of its neighbors.
//
//In the ConvolutionKernal algorithm presented below, the inner kx and ky 
//loops do the collection of the weighted neighbor pixel values using the 
//kernel parameter, which is hard-coded to be a 3x3 matrix of doubles.  
//Larger matrices are discussed elsewhere for more control and 'quality', 
//but the simple 3x3 matrix is quite suitable for what I consider to be the 
//most useful convolution kernel application - sharpening downsized images.
//
//The Sharpen method below defines a kernel specific to image sharpening,
//or more specifically, the enhancement of edge contrast.  The kernel is 
//described in numerous places, usually as a moderate application:
//
// 0 -1  0
//-1  5 -1
// 0 -1  0
//
//So, if you multiply the source pixel by 5, and each of the surrounding
//pixels by the corresponding values, and then sum it all up, you get a
//new value for the source pixel that will present more 'edge contrast',
//or acutance.  The method actually implements a range of sharpening by
//computing a matrix based on a linear 'strength', 0=no sharpening, 
//10=ludicrous sharpening.  The matrix values are calculated from 
//the strength number, preserving the relationship expressed in the 
//matrix described above.
//
//There are more complex algorithms available for 'sharpening', as it 
//were, but I find the simple 3x3 convolution kernel to work nicely
//for re-introducing acutance lost in down-sizing an image for web
//publication.  The application for that purpose is most valuable 
//going from 0 to 1 in strength, for ~640x480 images; 2 and above 
//start to introduce the halos and ringing that most find offensive.
//

gImage * gImage::ConvolutionKernel(double kernel[3][3], int threadcount)
{
	gImage *S = Copy();
	pix * cpy = S->getImageData();

	#pragma omp parallel for num_threads(threadcount)
	for(unsigned y = 1; y < h-1; y++) {
		for(unsigned x = 1; x < w-1; x++) {
			pix * dst = (pix *) (cpy + w*y + x);
			double R=0.0; double G=0.0; double B=0.0;
			for (unsigned kx=0; kx<3; kx++) {
				for (int ky=0; ky<3; ky++) {
					pix * src = (pix *) (img + w*(y-1+ky) + (x-1+kx));
					R += src->r * kernel[kx][ky];
					G += src->g * kernel[kx][ky];
					B += src->b * kernel[kx][ky];
				}
			}

			dst->r = R;
			dst->g = G;
			dst->b = B;
		}
	} 
	#pragma omp barrier

	return S;
}



gImage * gImage::Sharpen(int strength, int threadcount)
{
	double kernel[3][3] =
	{
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0
	};

	double x = -((strength)/4.0);
	kernel[0][1] = x;
	kernel[1][0] = x;
	kernel[1][2] = x;
	kernel[2][1] = x;
	kernel[1][1] = strength+1;

	return ConvolutionKernel(kernel, threadcount);

}


//Rotate
//
//Credit: Alan Paeth, "A Fast Algorithm for General Raster Rotation", Graphics Interface '86
//
//Image rotation is a deceivingly complex operation.  One would think it to be sufficient to 
//apply the sine and cosine of the rotation angle to each pixel coordinate, but you also have to
//manage the new image boundary, and the image can develop blocky aliasing in the edges.
//I started my exploration of image rotation with such an approach, and I've left it in 
//commented-out.
//
//These days, most 'quality' image rotation is done using Paeth's approach, which is the 
//application of three shears, one in the X direction, one in the Y direction, and a final
//one in the X direction to produce the rotation.  One wonders about the thinking required 
//to determine such transformations would produce the desired result.
//
//Three methods are presented to perform Paeth roatation: XShear and YShear, and a Rotate
//method to use them in the proscribed order.  Not implemented at this writing is an 
//intelligent adjustment of the rotated pixel to reflect its changed relationship with 
//adjunct pixels.  This does not seem to be an egregious issue with large out-of-camera
//images.
//

/*
gImage * gImage::Rotate(double angle, int threadcount)
{

	pix * src = getImageData();

	double rangle = angle * M_PI / 180.0;
	double cosine = cos(-rangle);
	double sine = sin(-rangle);

	int x1 = (int) -(h * sine);
	int y1 = (int)  (h * cosine);
	int x2 = (int)  (w * cosine - h * sine);
	int y2 = (int)  (h * cosine + w * sine);
	int x3 = (int)  (w * cosine);
	int y3 = (int)  (w * sine);

	int minx = std::min(0,std::min(x1, std::min(x2,x3)));
	int miny = std::min(0,std::min(y1, std::min(y2,y3)));
	int maxx = std::max(x1, std::max(x2,x3));
	int maxy = std::max(y1, std::max(y2,y3));

	int tx = minx;
	int ty = -miny;

	gImage *S = new gImage(maxx,maxy-miny, c, imginfo);
	pix * dst = S->getImageData();
	unsigned dw = S->getWidth();
	unsigned dh = S->getHeight();

	//printf("1:%d,%d  2:%d,%d  3:%d,%d\n", x1,y1,x2,y2,x3,y3);
	//printf("min: %d,%d   max:%d,%d\n",minx, miny, maxx, maxy);
	//printf("new image: %dx%d  shift: %d,%d\n", dw, dh, tx, ty);

	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {

			unsigned u = ( cosine * x + sine * y) - tx;
			unsigned v = (-sine * x + cosine * y) - ty;

			if (u < 0 | u >= dw) continue;
			if (v < 0 | v >= dh) continue;
			dst[x + y*dw].r = src[u + v*w].r;
			dst[x + y*dw].g = src[u + v*w].g;
			dst[x + y*dw].b = src[u + v*w].b;

		}
	}
	
	return S;

}
*/

gImage * gImage::Rotate(double angle, int threadcount)
{
	gImage *I, *J, *K, *L;
	double rangle = angle * M_PI / 180.0;

	I = XShear(rangle,threadcount);
	J = I->YShear(rangle,threadcount);
	delete I;
	K = J->XShear(rangle,threadcount);
	delete J;
	return K;
}

gImage *gImage::XShear(double rangle, int threadcount)
{
	double sine = sin(rangle);
	double tangent = tan(rangle/2.0);
	int dw = tangent * (double) h;
	unsigned nw = w+abs(dw);


	gImage *S = new gImage(nw, h, c, imginfo);
	pix * src = getImageData();
	pix * dst = S->getImageData();

	if (dw < 0) dw = 0;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned u = (x - tangent * y) + dw;
			unsigned v = y;
			if (u >= nw) continue;
			if (v >= h) continue;
			unsigned dpos =u + v*nw;
			unsigned spos = x + y*w;
			dst[dpos].r = src[spos].r;
			dst[dpos].g = src[spos].g;
			dst[dpos].b = src[spos].b;
		}
	}
	#pragma omp barrier

	return S;
}

gImage *gImage::YShear(double rangle, int threadcount)
{
	double sine = sin(rangle);
	double tangent = tan(rangle/2.0);
	int dh = sine * (double) w;
	unsigned nh = h+abs(dh);
	unsigned dw = w;

	if (dh < 0) dh = -dh; else dh = 0;

	gImage *S = new gImage(w, nh, c, imginfo);
	pix * src = getImageData();
	pix * dst = S->getImageData();

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned u = x;
			unsigned v = (y + sine * x) + dh;
			if (u >= w) continue;
			if (v >= nh) continue;
			unsigned dpos = u + v*dw;
			unsigned spos = x + y*w;
			dst[dpos].r = src[spos].r;
			dst[dpos].g = src[spos].g;
			dst[dpos].b = src[spos].b;
		}
	}
	#pragma omp barrier

	return S;
}


//Crop
//
//Credit: me
//
//Cropping an image is trivial.  This method uses a left-top and right-bottom bound to 
//extricate the portion of the image of interest.
//

gImage *gImage::Crop(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int threadcount)
{
	if (x1>w | y1>h | x2>w | y2>h) return NULL;
	if (x1>=x2 | y1>=y2) return NULL;

	pix * src = getImageData();
	gImage *S = new gImage(x2-x1, y2-y1, c, imginfo);
	pix * dst = S->getImageData();
	unsigned dw = S->getWidth();
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<S->getWidth(); x++) {
		for (unsigned y=0; y<S->getHeight(); y++) {
			unsigned dpos = x + y*dw;
			unsigned spos = x1+x + ((y+y1) * w);
			dst[dpos].r = src[spos].r;
			dst[dpos].g = src[spos].g;
			dst[dpos].b = src[spos].b;
		}
	}
	#pragma omp barrier

	return S;
}

//Curve
//
//Credit: Tino Kluge, http://kluge.in-chemnitz.de/opensource/spline/, GPLV2
//
//The so-called curve is an extremely useful construct for manipulating image attributes.
//Really, the concept is that of a lookup-table, where a tone or color is looked up to 
//determine a new value, based on some previously computed translation to produce the 
//table.  The curve notion provides an intuitive (well, to some) paradigm for building
//these lookup tables. When  you want to boost the tone of shadows, for instance, you 
//want to manage the transition from that transform into the highlights, or the image
//will show the manipulation quite obviously.  A spline-based graph used to  produce
//the lookup table helps to manage the 'budget' of tones when you go to make a transform.
//
//The algorithm implemented below uses a C++ spline library published by Tino Kluge. Its
//input is a list of 'control points', through which a smooth curve is posited, and its
//output is a Y value for a specified X coordinate at any point on the curve.  A
//challenge is presented in gimage by using floating point RGB values, in that a 
//lookup table for distinct values is impractical.  Accordingly, the actual transformation 
//is not a table lookup but instead the actual spline coordinate computation.  This makes
//the curve application computationally-intensive, and correspondingly slower than its
//lookup equivalent.
//
//Also presented with ApplyCurve is an ApplyLine method, presented in the notion that
//a number of useful 'curves' are really just straight lines, such as those used for
//contrast, brightness, and black-white point adjustment, and should require just a simple
//slope calculation rather than the spline interpolation.  At this writing, the benefit
//is not clear.
//


gImage *gImage::ApplyCurve(std::vector<cp> ctpts, int threadcount)
{
	gImage *S = new gImage(w, h, c, imginfo);
	pix * src = getImageData();
	pix * dst = S->getImageData();
	Curve c;
	c.setControlPoints(ctpts);

	#pragma omp parallel for num_threads(threadcount)
	for (int x=0; x<w; x++) {
		for (int y=0; y<h; y++) {
			int pos = x + y*w;
			dst[pos].r = c.getpoint(src[pos].r);
			dst[pos].g = c.getpoint(src[pos].g);
			dst[pos].b = c.getpoint(src[pos].b);
		}
	}
	#pragma omp barrier

	return S;
}

gImage *gImage::ApplyLine(double low, double high, int threadcount)
{
	gImage *S = new gImage(w, h, c, imginfo);
	pix * src = getImageData();
	pix * dst = S->getImageData();

	double slope = 255.0 / (high-low);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			dst[pos].r = src[pos].r * slope;
			dst[pos].g = src[pos].g * slope;
			dst[pos].b = src[pos].b * slope;
		}
	}
	#pragma omp barrier
	return S;
}


//Saturation
//
//Credit: Darel Rex Finley, http://alienryderflex.com/saturation.html, public domain
//
//This is a simple HSL saturation routine.  There are better ways to 
//manipulate color, but this one is self-contained to a RGB color space.
//Maybe later...
//


#define  Pr  .299
#define  Pg  .587
#define  Pb  .114

gImage *gImage::Saturate(double saturate, int threadcount)
{
	gImage *S = new gImage(w, h, c, imginfo);
	pix * src = getImageData();
	pix * dst = S->getImageData();

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			double R = src[pos].r;
			double G = src[pos].g;
			double B = src[pos].b;

			double  P=sqrt(
			R*R*Pr+
			G*G*Pg+
			B*B*Pb ) ;

			dst[pos].r=P+(R-P)*saturate;
			dst[pos].g=P+(G-P)*saturate;
			dst[pos].b=P+(B-P)*saturate;

		}
	}
	#pragma omp barrier

	return S;
}

//Tint
//
//Credit:
//
//Tint allows the addition or subtraction of a single value from a single
//channel.  My use case is to introduce tint to grayscale images.
//

gImage *gImage::Tint(double red,double green,double blue, int threadcount)
{
	gImage *S = new gImage(w, h, c, imginfo);
	pix * src = getImageData();
	pix * dst = S->getImageData();

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			dst[pos].r=src[pos].r + red;
			dst[pos].g=src[pos].g + green;
			dst[pos].b=src[pos].b + blue;

		}
	}
	#pragma omp barrier

	return S;
}

//Grayscaling
//
//Credit: Various
//
//Grayscaling an image is essentially computing a single tone value from the red, green, and blue
//components of each pixel.  This is typically done by giving each component a percentage weight
//in a sum; the typical percentages used are red=0.21, green=0.72, and blue=0.07; see 
//https://en.wikipedia.org/wiki/Grayscale for a discussion.
//
//The algorithm implemented allows for individual specification of the percentages.  It does not
//produce a single-valued image matrix; it loads the same gray tone in all three channels.  My
//thought would be to make an option to convert to, say, an 8-bit grayscale image when saving.
//Retaining the three-channel image allows subsequent manipulation of the channels to introduce
//tint.

gImage *gImage::Gray(double redpct, double greenpct, double bluepct, int threadcount)
{
	gImage *S = new gImage(w, h, c, imginfo);
	pix * src = getImageData();
	pix * dst = S->getImageData();

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			double G = floor(src[pos].r*redpct + src[pos].g*greenpct + src[pos].b*bluepct)+0.5;
			dst[pos].r=G;
			dst[pos].g=G;
			dst[pos].b=G;

		}
	}
	#pragma omp barrier

	return S;
}

//NLMeans Denoise
//
//Credit: Antoni Buades, et.al., and David Tschumperlé, principal programmer of GMIC, non-commercial use
//
//Denoising an image removes the "speckle" produced usually by low exposure values, where
//the light signal is not too far from the noise signal inherent in the sensor.  The
//essential operation is to 'average' surrounding pixels as the result value for the 
//source pixel.  In that regard, a 'blur' operation reduces noise, but at the expense of
//image detail because the average doesn't recognize edges.
//
//The Non-Local Means algorithm, first described by Antoni Buades, Bartomeu Coll and 
//Jean-Michel Morel in the IPOL Journal attempts to address this expense by recognizing similar
//toned pixels in the group, and giving them more weight in the average.  The implementation
//below is a simplistic but straightforward interpretation of the algorithm presented as a 
//GMIC script by David Tschumperlé in his blog at http://gmic.eu
//


gImage *gImage::NLMeans(double sigma, int local, int patch, int threadcount)
{

	gImage *S = new gImage(w, h, c, imginfo);
	pix * srcbits = getImageData();
	pix * dstbits = S->getImageData();

	unsigned spitch = w;
	unsigned dpitch = w;

	unsigned iw = w;
	unsigned ih = h;
	double sigma2 = pow(2*sigma,2);

	//y|x upper|lower bound computations, used to offset patch to avoid out-of-image references 
	unsigned yplb = patch+local+1;
	unsigned ypub = ih-yplb;
	unsigned xplb = yplb;
	unsigned xpub = iw-xplb;

	#pragma omp parallel for num_threads(threadcount)
	for(unsigned y = local; y < ih-local; y++) {
		unsigned py = y;
		if (py<yplb) py = yplb;
		if (py>ypub) py = ypub;
		for(unsigned x = local; x < iw-local; x++) {
			unsigned px = x;
			if (px<xplb) px = xplb;
			if (px>xpub) px = xpub;
			pix * wdstpix = dstbits + dpitch*y + x;
			double valueR = 0.0;
			double valueG = 0.0;
			double valueB = 0.0;
			double sum_weightsR = 0.0;
			double sum_weightsG = 0.0;
			double sum_weightsB = 0.0;
			for (int q = -local; q<=local; ++q) {
				for (int p = -local; p<=local; ++p) {
					double diffR = 0.0;
					double diffG = 0.0;
					double diffB = 0.0;
					for (int s = -patch; s<=patch; ++s) {
						for (int r = -patch; r<=patch; ++r) {
							pix * ppix = srcbits + spitch*(py+q+s) + (px+p+r);
							pix * lpix = srcbits + spitch*(py+s) + (px+r);
							diffR += pow((ppix->r - lpix->r),2);
							diffG += pow((ppix->g - lpix->g),2);
							diffB += pow((ppix->b - lpix->b),2);
							//gmic: diff += pow(i[x+p+r,y+q+s] - i[x+r,y+s],2);
						}
					}
					double weightR = exp(-diffR/sigma2);
					double weightG = exp(-diffG/sigma2);
					double weightB = exp(-diffB/sigma2);
					pix * localpix = srcbits + spitch*(y+q) + (x+p);
					valueR += weightR*localpix->r;
					valueG += weightG*localpix->g;
					valueB += weightB*localpix->b;
					//gmic: value += weight*i(x+p,y+q);
					sum_weightsR += weightR;
					sum_weightsG += weightG;
					sum_weightsB += weightB;

				}
			}
			wdstpix->r   = (valueR/(1e-5 + sum_weightsR));
			wdstpix->g = (valueG/(1e-5 + sum_weightsG));
			wdstpix->b  = (valueB/(1e-5 + sum_weightsB));
		}
	}

	return S;

}


//Resizing
//
//Credit: Graphics Gems III: Schumacher, Dale A., General Filtered Image Rescaling, p. 8-16
//https://github.com/erich666/GraphicsGems/blob/master/gemsiii/filter.c, public domain
//
//Resizing an image is either an interpolation (reduction) or extrapolation (expansion) of an image's 
//dimensions.  In the former case, it is the summarization of a group of pixels into one; in the 
//latter, it is essentially the creation of information not captured in the original image. This
//endeavor has to be well-formed in order to render a pleasing result.
//
//The essential operation of resize has two parts: 1) reduction/expansion in one dimension, then
//2) reduction/expansion of the intermediate image in the other direction.  So, the inter/extrapolation
//of a pixel is peformed with its row or column neighbors.  The amalgamation of neighbor pixels is
//performed with a filter algorithm, essentially a lookup of an equation based on distance from the
//source pixel.  Numerous filters have been presented in the literature; a representative sample is 
//included in this library.  
//
//The resize algorithm presented by Schumacher in Graphics Gems optimizes the application of a filter in 
//four steps: 1) for the destination image size in one dimension, calculate the filter contributions
//for each pixel in one row/colum of that dimension; 2) apply those contributions to the source image to 
//produce an intermediate image changed in size for that dimension; 3) calculate the filter contributions
//for each pixel in one row/colum of the other dimension; 4) apply those contributions to the
//intermediate image to produce the destination image.
//
//The relevant code below are the functions for each of the filters, followed by the Resize method 
//programmed in the pattern describe above.  You'll recognize a lot of the Schumacher's code; the
//contribution collection loops and data structures are pasted verbatim, as well as the filter
//functions.  At this writing, only the Lanczos3 filter is used, but that will change in later
//commits.
//

double sinc(double x)
{
	x *= M_PI;
	if(x != 0) return(sin(x) / x);
	return(1.0);
}

double Lanczos3_filter(double t)
{
	if(t < 0) t = -t;
	if(t < 3.0) return(sinc(t) * sinc(t/3.0));
	return(0.0);
}


gImage * gImage::Resize(unsigned width, unsigned height, RESIZE_FILTER filter, int threadcount)
{
	typedef struct {
		int	pixel;
		double	weight;
	} CONTRIB;

	typedef struct {
		int	n;		/* number of contributors */
		CONTRIB	*p;		/* pointer to list of contributions */
	} CLIST;


	CLIST	*contrib;		/* array of contribution lists */

	double xscale, yscale;		/* zoom scale factors */
	int i, j, k;			/* loop variables */
	int n;				/* pixel number */
	double center, left, right;	/* filter calculation variables */
	double wi, fscale, weight;	/* filter calculation variables */

	//hardcoded lanczos3
	double fwidth = 3.0; //for lanczos3
	double (*filterf)(double) = Lanczos3_filter;

	gImage *S = new gImage(width, height, c, imginfo);
	pix * dst = S->getImageData();
	gImage * T = new gImage(width, h, c, imginfo);
	pix * tmp = T->getImageData();
	pix * src = getImageData();

	xscale = (double) width / (double) w;
	yscale = (double) height / (double) h;


	// Compute row contributions:
	contrib = (CLIST *)calloc(width, sizeof(CLIST));
	if(xscale < 1.0) {
		wi = fwidth / xscale;
		fscale = 1.0 / xscale;
		for(i = 0; i < width; ++i) {
			contrib[i].n = 0;
			contrib[i].p = (CONTRIB *)calloc((int) (wi * 2 + 1),
					sizeof(CONTRIB));
			center = (double) i / xscale;
			left = ceil(center - wi);
			right = floor(center + wi);
			for(j = (int)left; j <= (int)right; ++j) {
				weight = center - (double) j;
				weight = (*filterf)(weight / fscale) / fscale;
				if(j < 0) {
					n = -j;
				} else if(j >= w) {
					n = (w - j) + w - 1;
				} else {
					n = j;
				}
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	} else {
		for(i = 0; i < width; ++i) {
			contrib[i].n = 0;
			contrib[i].p = (CONTRIB *)calloc((int) (fwidth * 2 + 1),
					sizeof(CONTRIB));
			center = (double) i / xscale;
			left = ceil(center - fwidth);
			right = floor(center + fwidth);
			for(j = (int)left; j <= (int)right; ++j) {
				weight = center - (double) j;
				weight = (*filterf)(weight);
				if(j < 0) {
					n = -j;
				} else if(j >= w) {
					n = (w - j) + w - 1;
				} else {
					n = j;
				}
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	}
	//dumpContrib(contrib,width);

	// Apply row contributions:
	#pragma omp parallel for num_threads(threadcount)
	for(unsigned y = 0; y < T->getHeight(); ++y) {
		pix * raster = (pix *) (src + w*y);
		pix * tmpimg = (pix *) (tmp + width*y);

		for(unsigned x = 0; x < T->getWidth(); ++x) {
			double weightr = 0.0;
			double weightg = 0.0;
			double weightb = 0.0;
			for(unsigned j = 0; j < contrib[x].n; ++j) {
				weightr += raster[contrib[x].p[j].pixel].r
					* contrib[x].p[j].weight;
				weightg += raster[contrib[x].p[j].pixel].g
					* contrib[x].p[j].weight;
				weightb += raster[contrib[x].p[j].pixel].b
					* contrib[x].p[j].weight;
			}
			tmpimg[x].r = weightr;
			tmpimg[x].g = weightg;
			tmpimg[x].b = weightb;
		}
	}
	#pragma omp barrier

	//free the memory allocated for horizontal filter weights:
	for(i = 0; i < T->getWidth(); ++i) {
		free(contrib[i].p);
	}
	free(contrib);


	// Compute column contributions:
	contrib = (CLIST *)calloc(height, sizeof(CLIST));
	if(yscale < 1.0) {
		wi = fwidth / yscale;
		fscale = 1.0 / yscale;
		for(i = 0; i < height; ++i) {
			contrib[i].n = 0;
			contrib[i].p = (CONTRIB *)calloc((int) (wi * 2 + 1),
					sizeof(CONTRIB));
			center = (double) i / yscale;
			left = ceil(center - wi);
			right = floor(center + wi);
			for(j = (int)left; j <= (int)right; ++j) {
				weight = center - (double) j;
				weight = (*filterf)(weight / fscale) / fscale;
				if(j < 0) {
					n = -j;
				} else if(j >= h) {
					n = (h - j) + h - 1;
				} else {
					n = j;
				}
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	} else {
		for(i = 0; i < height; ++i) {
			contrib[i].n = 0;
			contrib[i].p = (CONTRIB *)calloc((int) (fwidth * 2 + 1),
					sizeof(CONTRIB));
			center = (double) i / yscale;
			left = ceil(center - fwidth);
			right = floor(center + fwidth);
			for(j = (int)left; j <= (int)right; ++j) {
				weight = center - (double) j;
				weight = (*filterf)(weight);
				if(j < 0) {
					n = -j;
				} else if(j >= h) {
					n = (h - j) + h - 1;
				} else {
					n = j;
				}
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	}

	// Apply column contributions:
	#pragma omp parallel for num_threads(threadcount)
	for(unsigned x = 0; x < width; ++x) {
		for(unsigned y = 0; y < height; ++y) {
			double weightr = 0.0;
			double weightg = 0.0;
			double weightb = 0.0;
			for(unsigned j = 0; j < contrib[y].n; ++j) {
				weightr += tmp[x+(width*contrib[y].p[j].pixel)].r
					* contrib[y].p[j].weight;
				weightg += tmp[x+(width*contrib[y].p[j].pixel)].g
					* contrib[y].p[j].weight;
				weightb += tmp[x+(width*contrib[y].p[j].pixel)].b
					* contrib[y].p[j].weight;
			}
			dst[x+y*width].r = weightr;
			dst[x+y*width].g = weightg;
			dst[x+y*width].b = weightb;

		}
	}
	#pragma omp barrier

	//free the memory allocated for vertical filter weights:
	for(i = 0; i < height; ++i) {
		free(contrib[i].p);
	}
	free(contrib);


	delete T;
	return S;

}


//Image Information:

void gImage::Stats()
{
	double rmin, rmax, gmin, gmax, bmin, bmax;
	rmin = img->r; rmax = img->r; gmin=img->g; gmax = img->g; bmin = img->b; bmax = img->b;
	int iter = 0;

	for(unsigned y = 1; y < h; y++) {
		for(unsigned x = 1; x < w; x++) {
			pix * p = (pix *) img + y + x;
			if (p->r > rmax) rmax = p->r;
			if (p->g > gmax) gmax = p->g;
			if (p->b > bmax) bmax = p->b;
			if (p->r < rmin) rmin = p->r;
			if (p->g < gmin) gmin = p->g;
			if (p->b < bmin) bmin = p->b;
			iter++;
		}
	}
	printf("rmin: %f\trmax: %f\ngmin: %f\tgmax: %f\nbmin: %f\tbmax: %f\n", rmin, rmax, gmin, gmax, bmin, bmax);
	printf("iterations: %d\n\n", iter);
}

std::vector<long> gImage::Histogram()
{
	std::vector<long> histogram(256, 0);
	for(unsigned y = 0; y < h; y++) {
		for(unsigned x = 0; x < w; x++) {
			unsigned pos = x + y*w;
			double t = (img[pos].r + img[pos].g + img[pos].b) / 3.0;
			if (t < 0.0) t = 0.0;
			if (t > 255.0) t = 255.0;
			histogram[floor(t+0.5)]++;
		}
	}
	return histogram;
}




//Loaders:

gImage * gImage::loadImageFile(const char * filename, std::string params)
{
	char ext[5];
	strncpy(ext,filename+strlen(filename)-3,3); ext[3] = '\0';
	if (strcmp(ext,"tif") == 0) return gImage::loadTIFF(filename, params);
	if (strcmp(ext,"NEF") == 0) return gImage::loadRAW(filename, params);
	if (strcmp(ext,"jpg") == 0) return gImage::loadJPEG(filename, params);
	return NULL;
}

gImage * gImage::loadRAW(const char * filename, std::string params)
{
	unsigned width, height, colors, bpp, icclength;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	//std::string params = "autobright=1";
	char * image = _loadRAW_m(filename, &width, &height, &colors, &bpp, imgdata, params, iccprofile, &icclength);
	gImage * I = new gImage(image, width, height, colors, bpp, imgdata);
	free(image);
	return I;

}

gImage * gImage::loadJPEG(const char * filename, std::string params)
{
	unsigned width, height, colors, bpp;
	std::map<std::string,std::string> imgdata;
	char * image = _loadJPEG(filename, &width, &height, &colors, imgdata);
	gImage * I = new gImage(image, width, height, colors, 8, imgdata);
	free(image);
	return I;
}


gImage * gImage::loadTIFF(const char * filename, std::string params)
{
	unsigned width, height, colors, bpp;
	std::map<std::string,std::string> imgdata;
	char * image = _loadTIFF(filename, &width, &height, &colors, &bpp, imgdata);
	gImage * I = new gImage(image, width, height, colors, bpp, imgdata);
	free(image);
	return I;
}


//Savers:

bool gImage::saveImageFile(const char * filename)
{
	char ext[5];
	strncpy(ext,filename+strlen(filename)-3,3); ext[3] = '\0';
	if (strcmp(ext,"tif") == 0) {
		saveTIFF(filename, 16);
		return true;
	}
	if (strcmp(ext,"jpg") == 0) {
		saveJPEG(filename);
		return true;
	}
	return false;
}


void gImage::saveJPEG(const char * filename)
{
	_writeJPEG(filename, getImageData(8),  w, h, c, imginfo);
}

void gImage::saveTIFF(const char * filename, unsigned bits)
{
	_writeTIFF(filename, getImageData(bits),  w, h, c, bits, imginfo);
}




