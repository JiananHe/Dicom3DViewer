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

	void setMagnitudeThresh(float);
private:
	float mag_threshold;
};