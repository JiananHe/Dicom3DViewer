#pragma once
#include "seriesVisualizer.h"
#include <vtkImageNonMaximumSuppression.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageGradient.h>
#include <vtkImageMagnitude.h>
using namespace std;

class BoundVisualizer : public SeriesVisualizer
{
public:
	BoundVisualizer(QFrame *, QString, QFrame*);
	~BoundVisualizer();

	void transferData();
	void setMagSliderValue();

	void setMagnitudeThresh(float);
	QString getPositionMag(int x, int y);
	void visualizeData();
	void updateVisualData();
private:
	float mag_threshold;
	vtkSmartPointer <vtkImageMagnitude> imgMagnitude;
	vtkSmartPointer<vtkImageThreshold> mag_thresh;

	QSlider * magnitude_thresh_slider;
	QLabel * magnitude_max_label;
	QLabel * magnitude_min_label;
	QLabel * magnitude_cur_label;
};