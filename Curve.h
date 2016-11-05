#ifndef _curve_h
#define _curve_h

#define NDEBUG

//#include "spline.h"
#include <vector>
//#include <string>
//#include <cmath>

typedef struct cp {
	double x, y;
}cp;

class Curve
{
	public:
		Curve();
		std::vector<cp> getControlPoints();
		void setControlPoints(std::vector<cp> pts);
		void clearpoints();
		void insertpoint(double x, double y);
		bool deletepoint(double x, double y);
		void clampto(double min, double max);
		void scalepoints(double s);
		double getpoint(double x);
		cp getctrlpoint();
		cp getctrlpoint(int i);
		void setctrlpoint(int i, cp p);
		//vector index of cp if found, -1 if not:
		int isctrlpoint(double x, double y, int radius);
		bool isendpoint(double x, double y, int radius);

	private:


		bool loaded;
		std::vector<cp> controlpts;
		std::vector<double> X, Y;
		tk::spline s;
		cp ctrlpoint;
		double mn, mx;

		void sortpoints();
		void setpoints ();

};


#endif
