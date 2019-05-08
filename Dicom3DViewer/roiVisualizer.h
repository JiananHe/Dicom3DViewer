#pragma once
#include "seriesVisualizer.h"
#include "RangeSlider.h"
#include <vtkThresholdPoints.h>

class RoiVisualizer : public SeriesVisualizer
{
public:
	RoiVisualizer(QFrame *, QString, QFrame*);
	~RoiVisualizer();

	void transferData();
	void setOriginData(vtkSmartPointer<vtkImageData>);

	bool setRoiGrayRange(float, float);
	void updateVisualData();

	float getRoiRangeMin();
	float getRoiRangeMax();

	void kMeansCalc();
private:
	float roi_min;
	float roi_max; 
	vtkSmartPointer<vtkImageThreshold> roi_thresh;
	vtkSmartPointer<vtkImageShiftScale> roi_ss;

	RangeSlider * roi_range_slider;
	QLabel * roi_min_label;
	QLabel * roi_max_label;
};
