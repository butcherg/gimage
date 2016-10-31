
#include "gimage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

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

// Setters:

void gImage::setInfo(std::string name, std::string value)
{
	imginfo[name] = value;
}


//Image Operations:

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
				//dst->r = R;
				//dst->g = G;
				//dst->b = B;

			}

			dst->r = R;
			dst->g = G;
			dst->b = B;
		}
	} 

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


	typedef struct {
		int	pixel;
		double	weight;
	} CONTRIB;

	typedef struct {
		int	n;		/* number of contributors */
		CONTRIB	*p;		/* pointer to list of contributions */
	} CLIST;

void dumpContrib(CLIST *c, unsigned w)
{
	for (unsigned i=0; i<w; i++) {
		for (unsigned j=0; j<c[i].n; j++) {
			printf("p:%d, w:%f",c[i].p[j].pixel,c[i].p[j].weight);
		}
		printf("\n\n");
	}
}

gImage * gImage::Resize(unsigned width, unsigned height, FILTER filter, int threadcount)
{






	CLIST	*contrib;		/* array of contribution lists */

	double xscale, yscale;		/* zoom scale factors */
	int i, j, k;			/* loop variables */
	int n;				/* pixel number */
	double center, left, right;	/* filter calculation variables */
	double wi, fscale, weight;	/* filter calculation variables */

	//hardcoded lanczos3
	double fwidth = 3.0; //for lanczos3
	double (*filterf)(double) = Lanczos3_filter;
printf("Resize: 1...\n");
	gImage *S = new gImage(width, height, c, imginfo);
	pix * dst = S->getImageData();
	gImage * tmp = new gImage(width, h, c, imginfo);
	pix * src = getImageData();
printf("Resize: 2...\n");
	xscale = (double) width / (double) w;
	yscale = (double) height / (double) h;
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
dumpContrib(contrib,width);
printf("Resize: 3...\n");

/*
	for(unsigned y = 0; y < tmp->getHeight(); ++y) {
		pix * raster = (pix *) (src + w*y);
printf("Resize: 3a...\n");
		pix * tmpimg = (pix *) (tmp + width*y);
printf("Resize: 3b...\n");
		for(unsigned x = 0; x < tmp->getWidth(); ++x) {
			double weightr = 0.0;
			double weightg = 0.0;
			double weightb = 0.0;
			for(j = 0; j < contrib[x].n; ++j) {
				weightr += raster[contrib[x].p[j].pixel].r
					* contrib[x].p[j].weight;
				weightg += raster[contrib[x].p[j].pixel].g
					* contrib[x].p[j].weight;
				weightb += raster[contrib[x].p[j].pixel].b
					* contrib[x].p[j].weight;
			}
printf("Resize: 3c...\n");
			tmpimg[x].r = weightr;
			tmpimg[x].g = weightg;
			tmpimg[x].b = weightb;
		}
	}
printf("Resize: 4...\n");
*/
	//delete tmp;
	//return S;

	delete S;
	return tmp;
}

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




