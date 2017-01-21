
#include "gimage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <exception>
#include <omp.h>

#include "rawimage.h"
#include "jpegimage.h"
#include "tiffimage.h"
#include "strutil.h"

#define PI            3.14159265358979323846

//Range 0.0-255.0 constants
//#define SCALE_16BIT 256.0
//#define SCALE_8BIT 1.0
//#define SCALE_CURVE 1.0

//Range 0.0-1.0 constants
#define SCALE_16BIT 65536.0
#define SCALE_8BIT 256.0
#define SCALE_CURVE 256.0


//Constructors/Destructor:

gImage::gImage() 
{
	w=0; 
	h=0;
}

gImage::gImage(const gImage &o)
{
	w = o.w;
	h = o.h;
	c = o.c;
	imginfo = o.imginfo;
	image = o.image;

	profile = new char[o.profile_length];
	memcpy(profile, o.profile, o.profile_length);
	profile_length = o.profile_length;
}

gImage::gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, BPP bits, std::map<std::string,std::string> imageinfo, char * icc_profile, unsigned icc_profile_length)
{
	image.resize(width*height);
	w=width;
	h=height;
	c=colors;

	if (bits ==BPP_16) {
		unsigned short * src = (unsigned short *) imagedata;
		if (colors == 1) {  //turn into a three-color grayscale
			for (unsigned y=0; y<h; y++) {
				for (unsigned x=0; x<w; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (unsigned short) src[0]/SCALE_16BIT;
					image[pos].g = (unsigned short) src[0]/SCALE_16BIT;
					image[pos].b = (unsigned short) src[0]/SCALE_16BIT;
					src += 1;
				}
			}
			c = 3;
		}
		else if (colors == 3) {
			for (unsigned y=0; y<h; y++) {
				for (unsigned x=0; x<w; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (unsigned short) src[0]/SCALE_16BIT;
					image[pos].g = (unsigned short) src[1]/SCALE_16BIT;
					image[pos].b = (unsigned short) src[2]/SCALE_16BIT;
					src += 3;
				}
			}
		}
		else {
			w = 0;
			h = 0;
			return;
		}
	}					

	if (bits == BPP_8) {
		char * src = (char *) imagedata;
		if (colors == 1) {  //turn into a three-color grayscale
			for (unsigned y=0; y<h; y++) {
				for (unsigned x=0; x<w; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (unsigned char) src[0]/SCALE_8BIT;
					image[pos].g = (unsigned char) src[0]/SCALE_8BIT;
					image[pos].b = (unsigned char) src[0]/SCALE_8BIT;
					src += 1;
				}
			}
			c = 3;
		}
		else if (colors == 3) {
			for (unsigned y=0; y<height; y++) {
				for (unsigned x=0; x<width; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (unsigned char) src[0]/SCALE_8BIT;
					image[pos].g = (unsigned char) src[1]/SCALE_8BIT;
					image[pos].b = (unsigned char) src[2]/SCALE_8BIT;
					src += 3;
				}
			}
		}
		else {
			w = 0;
			h = 0;
			return;
		}
	}

	imginfo = imageinfo;

	if (icc_profile) {
		profile = new char[icc_profile_length];
		memcpy(profile, icc_profile, icc_profile_length);
		//delete [] icc_profile;
		profile_length = icc_profile_length;
	}
	else {
		profile = NULL;
		profile_length = 0;
	}

}

gImage::gImage(unsigned width, unsigned height, unsigned colors, std::map<std::string,std::string> imageinfo)
{
	image.resize(width*height);
	w=width;
	h=height;
	c=colors;

	for (unsigned y=0; y<height; y++) {
		for (unsigned x=0; x<width; x++) {
			unsigned pos = x + y*w;
			image[pos].r = 0.0;
			image[pos].g = 0.0;
			image[pos].b = 0.0;
		}
	}

	imginfo = imageinfo;
	profile =  NULL;
	profile_length = 0;
}


gImage::~gImage()
{
	if (profile) delete [] profile;
}


//Getters:

std::string gImage::getRGBCharacteristics()
{
	std::string charac;
	if (sizeof(PIXTYPE) == 2) charac.append("half, ");
	if (sizeof(PIXTYPE) == 4) charac.append("float, ");
	if (sizeof(PIXTYPE) == 8) charac.append("double, ");
	if (SCALE_CURVE == 1.0)   charac.append("0.0 - 255.0");
	if (SCALE_CURVE == 256.0) charac.append("0.0 - 1.0");
	return charac;
}


pix gImage::getPixel(unsigned x,  unsigned y)
{
	pix nullpix = {(PIXTYPE) 0.0, (PIXTYPE) 0.0, (PIXTYPE) 0.0};
	int i = x + y*w;
	if ((x < w) && (y < h))
		return image[i];
	else
		return nullpix;
}

std::vector<float> gImage::getPixelArray(unsigned x,  unsigned y)
{
	int i = x + y*w;
	std::vector<float> pixel;
	pixel.resize(3);
	if ((x < w) && (y < h)) {
		pixel[0] = (float) image[i].r;
		pixel[1] = (float) image[i].g;
		pixel[2] = (float) image[i].b;
	}
	else {
		pixel[0] = 0.0;
		pixel[1] = 0.0;
		pixel[2] = 0.0;
	}
	return pixel;
}


//structs for making raw images
struct cpix { char r, g, b; };
struct uspix { unsigned short r, g, b; };

char * gImage::getTransformedImageData(BPP bits, cmsHPROFILE profile)
{
	cmsHPROFILE hImgProfile;
	cmsUInt32Number informat, outformat;
	cmsHTRANSFORM hTransform;
	char * imagedata;
	if (bits == BPP_16) {
		imagedata = new char[w*h*c*2];
		outformat = TYPE_RGB_16;
	}
	else if (bits == BPP_8) {
		imagedata = new char[w*h*c];
		outformat = TYPE_RGB_8;
	}
	else
		return NULL;

	if (sizeof(PIXTYPE) == 2) informat = TYPE_RGB_HALF_FLT; 
	if (sizeof(PIXTYPE) == 4) informat = TYPE_RGB_FLT;
	if (sizeof(PIXTYPE) == 8) informat = TYPE_RGB_DBL;

	hImgProfile = cmsOpenProfileFromMem(getProfile(), getProfileLength());

	if (hImgProfile != NULL & profile != NULL) {
		hTransform = cmsCreateTransform(hImgProfile, informat, profile, outformat, INTENT_PERCEPTUAL, 0);
		cmsDoTransform(hTransform, image.data(), imagedata, w*h);
	}

	return imagedata;
}

char * gImage::getImageData(BPP bits, cmsHPROFILE profile)
{
	cmsHPROFILE hImgProfile;
	cmsUInt32Number format;
	cmsHTRANSFORM hTransform;
	char * imagedata;
	if (bits == BPP_16)
		imagedata = new char[w*h*c*2];
	else if (bits == BPP_8)
		imagedata = new char[w*h*c];
	else
		return NULL;

	if (bits == BPP_16) {
		uspix * dst = (uspix *) imagedata;
		#pragma omp parallel for
		for (unsigned y=0; y<h; y++) {
			for (unsigned x=0; x<w; x++) {
				unsigned pos = x + y*w;
				dst[pos].r = (unsigned short) lrint(fmin(fmax(image[pos].r*SCALE_16BIT,0.0),65535.0)); 
				dst[pos].g = (unsigned short) lrint(fmin(fmax(image[pos].g*SCALE_16BIT,0.0),65535.0));
				dst[pos].b = (unsigned short) lrint(fmin(fmax(image[pos].b*SCALE_16BIT,0.0),65535.0)); 
			}
		}
		format = TYPE_RGB_16;
	}

	if (bits == BPP_8) {
		cpix * dst = (cpix *) imagedata;
		#pragma omp parallel for
		for (unsigned y=0; y<h; y++) {
			for (unsigned x=0; x<w; x++) {
				unsigned pos = x + y*w;
				dst[pos].r = (unsigned char) lrint(fmin(fmax(image[pos].r*SCALE_8BIT,0.0),255.0)); 
				dst[pos].g = (unsigned char) lrint(fmin(fmax(image[pos].g*SCALE_8BIT,0.0),255.0));
				dst[pos].b = (unsigned char) lrint(fmin(fmax(image[pos].b*SCALE_8BIT,0.0),255.0)); 
			}
		}
		format = TYPE_RGB_8;
	}

	if (profile) {
		hImgProfile = cmsOpenProfileFromMem(getProfile(), getProfileLength());
		if (hImgProfile != NULL & profile != NULL) {
			hTransform = cmsCreateTransform(hImgProfile, format, profile, format, INTENT_PERCEPTUAL, 0);
			cmsCloseProfile(hImgProfile);
			cmsDoTransform(hTransform, imagedata, imagedata, w*h);
		}
	}

	return imagedata;
}

std::vector<pix>& gImage::getImageData()
{
	return image;
}

pix* gImage::getImageDataRaw()
{
	return image.data();
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

char * gImage::getProfile()
{
	return profile;
}

unsigned gImage::getProfileLength()
{
	return profile_length;
}

std::map<std::string,std::string> gImage::getInfo(const char * filename)
{
	unsigned width, height, colors, bpp;
	char ext[5];
	std::map<std::string, std::string> imgdata;
	strncpy(ext,filename+strlen(filename)-3,3); ext[3] = '\0';
	if (strcmp(ext,"tif") == 0) _loadTIFFInfo(filename, &width, &height, &colors, &bpp, imgdata);
	if (strcmp(ext,"NEF") == 0) _loadRAWInfo_m(filename, &width, &height, &colors, &bpp, imgdata);
	if ((strcmp(ext,"jpg") == 0) | (strcmp(ext,"JPG") == 0)) _loadJPEGInfo(filename, &width, &height, &colors, imgdata);
	return imgdata;
}

GIMAGE_FILETYPE gImage::getFileType(const char * filename)
{
	std::string fname = filename;
	int dotpos = fname.find_last_of(".");
	std::string ext = fname.substr(dotpos+1);
	if (ext.compare("tif") == 0) return FILETYPE_TIFF;
	if (ext.compare("tiff") == 0) return FILETYPE_TIFF;
	if (ext.compare("NEF") == 0) return FILETYPE_RAW;
	if ((ext.compare("jpg") == 0) | (ext.compare("JPG") == 0)) return FILETYPE_JPEG;
	return FILETYPE_UNKNOWN;
}

std::string gImage::Version()
{
	return VERSION;
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


void gImage::ApplyConvolutionKernel(double kernel[3][3], int threadcount)
{
	std::vector<pix>& dst = getImageData();
	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;

	#pragma omp parallel for num_threads(threadcount)
	for(int y = 1; y < h-1; y++) {
		for(int x = 1; x < w-1; x++) {
			int pos = x + y*w;
			double R=0.0; double G=0.0; double B=0.0;
			for (int kx=0; kx<3; kx++) {
				for (int ky=0; ky<3; ky++) {
					int kpos = w*(y-1+ky) + (x-1+kx);
					R += src[kpos].r * kernel[kx][ky];
					G += src[kpos].g * kernel[kx][ky];
					B += src[kpos].b * kernel[kx][ky];
				}
			}

			dst[pos].r = R;
			dst[pos].g = G;
			dst[pos].b = B;
		}
	} 
	delete s;
}

void gImage::ApplySharpen(int strength, int threadcount)
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

	ApplyConvolutionKernel(kernel, threadcount);
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

	double rangle = angle * PI / 180.0;
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


void gImage::ApplyRotate(double angle, int threadcount)
{
	//gImage I, J, K, L;
	unsigned x1, x2, y1, y2;
	double rangle = angle * PI / 180.0;

	ApplyXShear(rangle,threadcount);
	ApplyYShear(rangle,threadcount);
	ApplyXShear(rangle,threadcount);
	ImageBounds(&x1, &x2, &y1, &y2);
	ApplyCrop(x1, y1, x2, y2,threadcount);
}

void gImage::ApplyXShear(double rangle, int threadcount)
{
	double sine = sin(rangle);
	double tangent = tan(rangle/2.0);
	int dw = tangent * (double) h;
	unsigned nw = w+abs(dw);

	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;
	image.resize(nw*h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i = 0; i<image.size(); i++) {
		image[i].r = 0.0;
		image[i].g = 0.0;
		image[i].b = 0.0;
	}

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
			image[dpos].r = src[spos].r;
			image[dpos].g = src[spos].g;
			image[dpos].b = src[spos].b;
		}
	}

	w = nw;
	delete s;
}

void gImage::ApplyYShear(double rangle, int threadcount)
{
	double sine = sin(rangle);
	double tangent = tan(rangle/2.0);
	int dh = sine * (double) w;
	unsigned nh = h+abs(dh);
	unsigned dw = w;

	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;
	std::vector<pix>& dst = getImageData();
	dst.resize(w*nh);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i = 0; i<image.size(); i++) {
		image[i].r = 0.0;
		image[i].g = 0.0;
		image[i].b = 0.0;
	}

	if (dh < 0) dh = -dh; else dh = 0;

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
	h = nh;
	delete s;
}

void gImage::ImageBounds(unsigned *x1, unsigned *x2, unsigned *y1, unsigned *y2)
{
	*x1 = 0; *x2 = w; *y1 = 0; *y2 = h;
	std::vector<pix>& src = getImageData();
	for (int x=0; x<w; x++) {
		for (int y=0; y<h; y++) {
			unsigned pos = x + y*w;
			if (src[pos].r + src[pos].g + src[pos].b > 0.0) {
				*x1 = x;
				goto endx1;
			}
		}
	}
	endx1:
	for (int x=w-1; x>0; x--) {
		for (int y=0; y<h; y++) {
			unsigned pos = x + y*w;
			if (src[pos].r + src[pos].g + src[pos].b > 0.0) {
				*x2 = x;
				goto endx2;
			}
		}
	}
	endx2:
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			unsigned pos = x + y*w;
			if (src[pos].r + src[pos].g + src[pos].b > 0.0) {
				*y1 = y;
				goto endy1;
			}
		}
	}
	endy1:
	for (int y=h-1; y>0; y--) {
		for (int x=0; x<w; x++) {
			unsigned pos = x + y*w;
			if (src[pos].r + src[pos].g + src[pos].b > 0.0) {
				*y2 = y;
				goto endy2;
			}
		}
	}
	endy2:
	return;
}


//Crop
//
//Credit: me
//
//Cropping an image is trivial.  This method uses a left-top and right-bottom bound to 
//extricate the portion of the image of interest.
//

void gImage::ApplyCrop(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int threadcount)
{
	if (x1>w | y1>h | x2>w | y2>h) return;
	if (x1>=x2 | y1>=y2) return;

	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;
	image.resize((x2-x1) * (y2-y1));

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i = 0; i<image.size(); i++) {
		image[i].r = 0.0;
		image[i].g = 0.0;
		image[i].b = 0.0;
	}

	unsigned dw = x2-x1;
	unsigned dh = y2-y1;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<dw; x++) {
		for (unsigned y=0; y<dh; y++) {
			unsigned dpos = x + y*dw;
			unsigned spos = x1+x + ((y+y1) * w);
			image[dpos].r = src[spos].r;
			image[dpos].g = src[spos].g;
			image[dpos].b = src[spos].b;
		}
	}
	w=dw;
	h=dh;
	delete s;
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


/*
gImage gImage::ToneLine(double low, double high, int threadcount)
{
	gImage S(w, h, c, imginfo);
	std::vector<pix>& src = getImageData();
	std::vector<pix>& dst = S.getImageData();

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
	return S;
}
*/

void gImage::ApplyToneCurve(std::vector<cp> ctpts, int threadcount)
{
	std::vector<pix>& dst = getImageData();

	Curve c;
	c.setControlPoints(ctpts);
	c.scalepoints(1.0/SCALE_CURVE);

	#pragma omp parallel for num_threads(threadcount)
	for (int x=0; x<w; x++) {
		for (int y=0; y<h; y++) {
			int pos = x + y*w;;
			dst[pos].r = c.getpoint(dst[pos].r);
			dst[pos].g = c.getpoint(dst[pos].g);
			dst[pos].b = c.getpoint(dst[pos].b);
		}
	}
}

void gImage::ApplyToneLine(double low, double high, int threadcount)
{

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


void gImage::ApplySaturate(double saturate, int threadcount)
{
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			double R = image[pos].r;
			double G = image[pos].g;
			double B = image[pos].b;

			double  P=sqrt(
			R*R*Pr+
			G*G*Pg+
			B*B*Pb ) ;

			image[pos].r=P+(R-P)*saturate;
			image[pos].g=P+(G-P)*saturate;
			image[pos].b=P+(B-P)*saturate;

		}
	}
}

//Tint
//
//Credit:
//
//Tint allows the addition or subtraction of a single value from a single
//channel.  My use case is to introduce tint to grayscale images.
//



void gImage::ApplyTint(double red,double green,double blue, int threadcount)
{
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos].r += red;
			image[pos].g += green;
			image[pos].b += blue;

		}
	}
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


void gImage::ApplyGray(double redpct, double greenpct, double bluepct, int threadcount)
{
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			//double G = floor(image[pos].r*redpct + image[pos].g*greenpct + image[pos].b*bluepct)+0.5;
			double G = image[pos].r*redpct + image[pos].g*greenpct + image[pos].b*bluepct;
			image[pos].r=G;
			image[pos].g=G;
			image[pos].b=G;

		}
	}
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


void gImage::ApplyNLMeans(double sigma, int local, int patch, int threadcount)
{
	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;
	std::vector<pix>& dst = getImageData();

	unsigned spitch = w;
	unsigned dpitch = w;

	unsigned iw = w;
	unsigned ih = h;
	double sigma2 = pow(2*(sigma/SCALE_CURVE),2);  //UI sigmas are relevant to 0-255

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
			//pix * wdstpix = dstbits + dpitch*y + x;
			unsigned wdstpix = dpitch*y + x;
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
							//pix * ppix = srcbits + spitch*(py+q+s) + (px+p+r);
							unsigned ppix = spitch*(py+q+s) + (px+p+r);
							//pix * lpix = srcbits + spitch*(py+s) + (px+r);
							unsigned lpix = spitch*(py+s) + (px+r);
							diffR += pow((src[ppix].r - src[lpix].r),2);
							diffG += pow((src[ppix].g - src[lpix].g),2);
							diffB += pow((src[ppix].b - src[lpix].b),2);
							//gmic: diff += pow(i[x+p+r,y+q+s] - i[x+r,y+s],2);
						}
					}
					double weightR = exp(-diffR/sigma2);
					double weightG = exp(-diffG/sigma2);
					double weightB = exp(-diffB/sigma2);
					//pix * localpix = srcbits + spitch*(y+q) + (x+p);
					unsigned localpix = spitch*(y+q) + (x+p);
					valueR += weightR*src[localpix].r;
					valueG += weightG*src[localpix].g;
					valueB += weightB*src[localpix].b;
					//gmic: value += weight*i(x+p,y+q);
					sum_weightsR += weightR;
					sum_weightsG += weightG;
					sum_weightsB += weightB;
				}
			}
			dst[wdstpix].r   = (valueR/(1e-5 + sum_weightsR));
			dst[wdstpix].g = (valueG/(1e-5 + sum_weightsG));
			dst[wdstpix].b  = (valueB/(1e-5 + sum_weightsB));
		}
	}
	delete s;
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
	x *= PI;
	if(x != 0) return(sin(x) / x);
	return(1.0);
}

#define Lanczos3_support (3.0)

double Lanczos3_filter(double t)
{
	if(t < 0) t = -t;
	if(t < 3.0) return(sinc(t) * sinc(t/3.0));
	return(0.0);
}


#define CatmullRom_support (2.0)

double CatmullRom_filter(double t) { 
	if(t < -2) return 0;
	if(t < -1) return (0.5*(4 + t*(8 + t*(5 + t))));
	if(t < 0)  return (0.5*(2 + t*t*(-5 - 3*t)));
	if(t < 1)  return (0.5*(2 + t*t*(-5 + 3*t)));
	if(t < 2)  return (0.5*(4 + t*(-8 + t*(5 - t))));
	return 0;
}


#define	Bspline_support	(2.0)

double Bspline_filter(double t)
{
	double tt;

	if(t < 0) t = -t;
	if(t < 1) {
		tt = t * t;
		return((.5 * tt * t) - tt + (2.0 / 3.0));
	} else if(t < 2) {
		t = 2 - t;
		return((1.0 / 6.0) * (t * t * t));
	}
	return(0.0);
}


#define	Bicubic_support	(2.0)

#define	B	(1.0 / 3.0)
#define	C	(1.0 / 3.0)

double Bicubic_filter(double t)
{
	double tt;

	tt = t * t;
	if(t < 0) t = -t;
	if(t < 1.0) {
		t = (((12.0 - 9.0 * B - 6.0 * C) * (t * tt))
		   + ((-18.0 + 12.0 * B + 6.0 * C) * tt)
		   + (6.0 - 2 * B));
		return(t / 6.0);
	} else if(t < 2.0) {
		t = (((-1.0 * B - 6.0 * C) * (t * tt))
		   + ((6.0 * B + 30.0 * C) * tt)
		   + ((-12.0 * B - 48.0 * C) * t)
		   + (8.0 * B + 24 * C));
		return(t / 6.0);
	}
	return(0.0);
}

#define	bilinear_support	(1.0)

double bilinear_filter(double t)
{
	if(t < 0.0) t = -t;
	if(t < 1.0) return(1.0 - t);
	return(0.0);
}


#define	box_support		(0.5)

double box_filter(double t)
{
	if((t > -0.5) & (t <= 0.5)) return(1.0);
	return(0.0);
}



void gImage::ApplyResize(unsigned width, unsigned height, RESIZE_FILTER filter, int threadcount)
{
	typedef struct {
		int	pixel;
		double	weight;
	} CONTRIB;

	typedef struct {
		int	n;		// number of contributors 
		CONTRIB	*p;		// pointer to list of contributions 
	} CLIST;


	CLIST	*contrib;		// array of contribution lists 

	double xscale, yscale;		// zoom scale factors 
	int i, j, k;			// loop variables
	int n;				// pixel number 
	double center, left, right;	// filter calculation variables 
	double wi, fscale, weight;	// filter calculation variables 

	double fwidth;
	double (*filterf)(double);

	switch (filter) {
		case FILTER_BOX:
			fwidth = box_support;
			filterf = box_filter;
			break;
		case FILTER_BILINEAR:
			fwidth = bilinear_support;
			filterf = bilinear_filter;
			break;
		case FILTER_BSPLINE:
			fwidth = Bspline_support;
			filterf = Bspline_filter;
			break;
		case FILTER_BICUBIC:
			fwidth = Bicubic_support;
			filterf = Bicubic_filter;
			break;
		case FILTER_CATMULLROM:
			fwidth = CatmullRom_support;
			filterf = CatmullRom_filter;
			break;
		case FILTER_LANCZOS3:
			fwidth = Lanczos3_support;
			filterf = Lanczos3_filter;
			break;
	}


	xscale = (double) width / (double) w;
	yscale = (double) height / (double) h;

	std::vector<pix>& src = getImageData();
	std::vector<pix> *t = new std::vector<pix>(image);
	std::vector<pix> &tmp = *t;

	tmp.resize(width*h);
	std::vector<pix>&dst = getImageData();


	// Compute row contributions:
	contrib = new CLIST[width];
	if(xscale < 1.0) {
		wi = fwidth / xscale;
		fscale = 1.0 / xscale;
		for(i = 0; i < width; ++i) {
			contrib[i].n = 0;
			contrib[i].p = new CONTRIB[(int) (wi * 2 + 1)];
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
			contrib[i].p = new CONTRIB[(int) (fwidth * 2 + 1)];
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
	for(unsigned y = 0; y < h; ++y) {
		unsigned raster = w*y;
		for(unsigned x = 0; x < width; ++x) {
			unsigned pos = x + width*y;
			double weightr = 0.0;
			double weightg = 0.0;
			double weightb = 0.0;
			for(unsigned j = 0; j < contrib[x].n; ++j) {
				unsigned rpos = raster+contrib[x].p[j].pixel;
				weightr += image[rpos].r * contrib[x].p[j].weight;
				weightg += image[rpos].g * contrib[x].p[j].weight;
				weightb += image[rpos].b * contrib[x].p[j].weight;
			}
			tmp[pos].r = weightr;
			tmp[pos].g = weightg;
			tmp[pos].b = weightb;
		}
	}


	//delete the memory allocated for horizontal filter weights:
	for(i = 0; i < width; ++i) {
		delete contrib[i].p;
	}
	delete[] contrib;


	// Compute column contributions:
	contrib = new CLIST[height];
	if(yscale < 1.0) {
		wi = fwidth / yscale;
		fscale = 1.0 / yscale;
		for(i = 0; i < height; ++i) {
			contrib[i].n = 0;
			contrib[i].p = new CONTRIB[(int) (wi * 2 + 1)];
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
			contrib[i].p = new CONTRIB[(int) (fwidth * 2 + 1)];
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

	dst.resize(width*height);
	w = width;
	h = height;

	// Apply column contributions:
	#pragma omp parallel for num_threads(threadcount)
	for(unsigned x = 0; x < width; ++x) {
		for(unsigned y = 0; y < height; ++y) {
			unsigned pos = x + y*width;
			double weightr = 0.0;
			double weightg = 0.0;
			double weightb = 0.0;
			for(unsigned j = 0; j < contrib[y].n; ++j) {
				unsigned cpos = x+(width*contrib[y].p[j].pixel);
				weightr += tmp[cpos].r * contrib[y].p[j].weight;
				weightg += tmp[cpos].g * contrib[y].p[j].weight;
				weightb += tmp[cpos].b * contrib[y].p[j].weight;
			}
			dst[pos].r = weightr;
			dst[pos].g = weightg;
			dst[pos].b = weightb;

		}
	}

	//delete the memory allocated for vertical filter weights:
	for(i = 0; i < height; ++i) {
		delete contrib[i].p;
	}
	delete[] contrib;
	delete t;
}




//Image Information:

std::string gImage::Stats()
{
	double rmin, rmax, gmin, gmax, bmin, bmax, tmin, tmax;
	rmin = image[0].r; rmax = image[0].r; gmin=image[0].g; gmax = image[0].g; bmin = image[0].b; bmax = image[0].b;
	tmin = SCALE_CURVE; tmax = 0.0;
	int iter = 0;

	for(unsigned y = 1; y < h; y++) {
		for(unsigned x = 1; x < w; x++) {
			unsigned pos = x + y*w;
			if (image[pos].r > rmax) rmax = image[pos].r;
			if (image[pos].g > gmax) gmax = image[pos].g;
			if (image[pos].b > bmax) bmax = image[pos].b;
			if (image[pos].r < rmin) rmin = image[pos].r;
			if (image[pos].g < gmin) gmin = image[pos].g;
			if (image[pos].b < bmin) bmin = image[pos].b;
			double tone = (image[pos].r + image[pos].g + image[pos].b) / 3.0; 
			if (tone > tmax) tmax = tone; if (tone < tmin) tmin = tone;
			iter++;
		}
	}
	return string_format("rmin: %f\trmax: %f\ngmin: %f\tgmax: %f\nbmin: %f\tbmax: %f\n\ntonemin: %f\ttonemax: %f\n", rmin, rmax, gmin, gmax, bmin, bmax, tmin, tmax);
	//printf("iterations: %d\n\n", iter);
}

std::vector<long> gImage::Histogram()
{
	std::vector<long> histogram(256, 0);
	for(unsigned y = 0; y < h; y++) {
		for(unsigned x = 0; x < w; x++) {
			unsigned pos = x + y*w;
			double t = ((image[pos].r + image[pos].g + image[pos].b) / 3.0) * SCALE_CURVE;
			if (t < 0.0) t = 0.0;
			if (t > 255.0) t = 255.0;
			histogram[floor(t+0.5)]++;
		}
	}
	return histogram;
}




//Loaders:

gImage gImage::loadImageFile(const char * filename, std::string params)
{
	GIMAGE_FILETYPE ext = gImage::getFileType(filename);

	if (ext == FILETYPE_TIFF) return gImage::loadTIFF(filename, params);
	if (ext == FILETYPE_RAW) return gImage::loadRAW(filename, params);
	if (ext == FILETYPE_JPEG) return gImage::loadJPEG(filename, params);
	return gImage();
}

gImage gImage::loadRAW(const char * filename, std::string params)
{
	unsigned width, height, bpp, colors, icclength;
	BPP bits;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	char * image = _loadRAW_m(filename, &width, &height, &colors, &bpp, imgdata, params, &iccprofile, &icclength);
	switch (bpp) {
		case 8: 
			bits = BPP_8;
			break;
		case 16:
			bits = BPP_16;
			break;
		//default:
		//	throw bad_typeid;
	}
	gImage I(image, width, height, colors, bits, imgdata, iccprofile, icclength);
	delete [] image;
	if (icclength && iccprofile) delete [] iccprofile;
	return I;

}

gImage gImage::loadJPEG(const char * filename, std::string params)
{
	unsigned width, height, colors, bpp;
	std::map<std::string,std::string> imgdata;
	char * image = _loadJPEG(filename, &width, &height, &colors, imgdata);
	gImage I(image, width, height, colors, BPP_8, imgdata);
	delete [] image;
	return I;
}


gImage gImage::loadTIFF(const char * filename, std::string params)
{
	unsigned width, height, colors, bpp, icclength;
	BPP bits;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	char * image = _loadTIFF(filename, &width, &height, &colors, &bpp, imgdata, params, &iccprofile, &icclength);
	switch (bpp) {
		case 8: 
			bits = BPP_8;
			break;
		case 16:
			bits = BPP_16;
			break;
		//default:
		//	throw bad_typeid;
	}
	gImage I(image, width, height, colors, bits, imgdata, iccprofile, icclength);
	delete [] image;
	if (icclength && iccprofile) delete [] iccprofile;
	return I;
}


//Savers:

bool gImage::saveImageFile(const char * filename, std::string params, cmsHPROFILE profile)
{
	char ext[5];
	strncpy(ext,filename+strlen(filename)-3,3); ext[3] = '\0';
	if (strcmp(ext,"tif") == 0) {
		if (profile)
			saveTIFF(filename, BPP_16, profile);
		else
			saveTIFF(filename, BPP_16);
		return true;
	}
	if (strcmp(ext,"jpg") == 0) {
		if (profile)
			saveJPEG(filename, params, profile);
		else
			saveJPEG(filename, params);
		return true;
	}
	return false;
}


void gImage::saveJPEG(const char * filename, std::string params, cmsHPROFILE profile)
{
	if (profile) {
		_writeJPEG(filename, getTransformedImageData(BPP_8, profile),  w, h, c, imginfo, params);  //slower, but not as noisy
		//_writeJPEG(filename, getImageData(BPP_8, profile),  w, h, c, imginfo, params);  //seems a bit noisier
	}
	else
		_writeJPEG(filename, getImageData(BPP_8),  w, h, c, imginfo, params);
}

void gImage::saveTIFF(const char * filename, BPP bits, cmsHPROFILE profile)
{
	unsigned b = 16;
	if (bits == BPP_16) b = 16;
	if (bits == BPP_8)  b = 8;
	if (profile)
		_writeTIFF(filename, getTransformedImageData(bits, profile),  w, h, c, b, imginfo);
		//_writeTIFF(filename, getImageData(bits, profile),  w, h, c, b, imginfo);
	else
		_writeTIFF(filename, getImageData(bits),  w, h, c, b, imginfo);		
}


//ICC Profiles:
//
//Primaries and black/white points from Elle Stone's make-elles-profiles.c, 
//https://github.com/ellelstone/elles_icc_profiles, GPL V2
//
cmsCIExyY d50_romm_spec= {0.3457, 0.3585, 1.0};
cmsCIEXYZ d50_romm_spec_media_whitepoint = {0.964295676, 1.0, 0.825104603};
cmsCIExyY d65_srgb_adobe_specs = {0.3127, 0.3290, 1.0};
cmsCIEXYZ d65_media_whitepoint = {0.95045471, 1.0, 1.08905029};

cmsCIExyYTRIPLE aces_primaries_prequantized = 
{
{0.734704192222, 0.265298276252,  1.0},
{-0.000004945077, 0.999992850272,  1.0},
{0.000099889199, -0.077007518685,  1.0}
};

cmsCIExyYTRIPLE romm_primaries = {
{0.7347, 0.2653, 1.0},
{0.1596, 0.8404, 1.0},
{0.0366, 0.0001, 1.0}
};

cmsCIExyYTRIPLE widegamut_pascale_primaries = {
{0.7347, 0.2653, 1.0},
{0.1152, 0.8264, 1.0},
{0.1566, 0.0177, 1.0}
};

cmsCIExyYTRIPLE adobe_primaries_prequantized = {
{0.639996511, 0.329996864, 1.0},
{0.210005295, 0.710004866, 1.0},
{0.149997606, 0.060003644, 1.0}
};

cmsCIExyYTRIPLE srgb_primaries_pre_quantized = {
{0.639998686, 0.330010138, 1.0},
{0.300003784, 0.600003357, 1.0},
{0.150002046, 0.059997204, 1.0}
};

cmsHPROFILE gImage::makeLCMSProfile(const std::string name, float gamma)
{
	cmsHPROFILE profile;
	cmsCIExyYTRIPLE c;
	if (name == "srgb") c =  srgb_primaries_pre_quantized;
	else if (name == "wide") c =  widegamut_pascale_primaries;
	else if (name == "adobe") c =  adobe_primaries_prequantized;
	else if (name == "prophoto") c =  romm_primaries;
	else return NULL;
	cmsToneCurve *curve[3], *tonecurve;
	tonecurve = cmsBuildGamma (NULL, gamma);
	curve[0] = curve[1] = curve[2] = tonecurve;

	profile =  cmsCreateRGBProfile ( &d65_srgb_adobe_specs, &c, curve);

	std::string descr = "rawproc D65-"+name;
	cmsMLU *description;
	description = cmsMLUalloc(NULL, 1);
	cmsMLUsetASCII(description, "en", "US", descr.c_str());
	cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);

	return profile;
	
}

void gImage::makeICCProfile(cmsHPROFILE hProfile, char * profile, cmsUInt32Number  &profilesize)
{
	cmsSaveProfileToMem(hProfile, NULL, &profilesize);
	profile = new char[profilesize];
	cmsSaveProfileToMem(profile, profile, &profilesize);
}




