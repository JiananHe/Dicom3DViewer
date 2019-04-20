#pragma once
#include "dicomSeriesReader.h"
#include <vector>
#include <qframe.h>
#include <qtextedit.h>
#include <qpushbutton.h>
using namespace std;

class BoundVisualizer
{
public:
	BoundVisualizer(QFrame * frame);
	~BoundVisualizer();

	void setRoiGrayValue(float roi_gv);

	void findROIBound(vtkImageData * igd, vtkImageData * imd, vtkImageData *igv, vtkThresholdPoints * bmp);

private:
	vtkImageData * imageGradientData;
	vtkImageData * imageMagnitudeData;
	vtkImageData * imageGrayData;
	vtkPolyData * boundMagnitudePoly;
	int dims[3];

	float roi_gv;
	float roi_gv_offset;

	vector<float> roi_bound_gv;
	vector<float> roi_bound_gd;

	QTextEdit * bound_grayValue_label;
	QTextEdit * bound_grayOffset_label;

	int * calcMaxGradientAxisAndOrient(float * gradient, float cur_gv);
	bool isOutOfImage(int * coords);
};