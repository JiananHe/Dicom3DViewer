#pragma once
#include "seriesVisualizer.h"
#include <vtkThresholdPoints.h>
#include <vector>
using namespace std;

class BoundVisualizer : public SeriesVisualizer
{
public:
	BoundVisualizer(QFrame *, QString, QFrame*);
	~BoundVisualizer();

	void transferData();

	void setMagnitudeThresh(float);
	QString getPositionMag(int x, int y);
	void updateVisualData();

	float getMaxBoundGradientValue();
	float getMinBoundGradientValue();

	void calcRoiBoundPoly(vtkSmartPointer<vtkImageData>, vtkSmartPointer<vtkImageData>);
	vector<int> getRoiBoundGvs();
	vector<int> getRoiBoundGds();

private:
	float mag_threshold;
	double mag_range[2];
	vtkSmartPointer <vtkImageMagnitude> imgMagnitude;
	vtkSmartPointer <vtkImageData> nonMaxFloat;
	vtkSmartPointer<vtkImageThreshold> mag_thresh_img;
	vtkSmartPointer<vtkThresholdPoints> mag_thresh_poly;

	QSlider * magnitude_thresh_slider;
	QLabel * magnitude_max_label;
	QLabel * magnitude_min_label;
	QLabel * magnitude_cur_label;

	vector<int> roi_bound_gv;
	vector<int> roi_bound_gd;
};