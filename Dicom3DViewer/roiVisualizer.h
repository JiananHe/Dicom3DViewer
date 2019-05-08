#pragma once
#include "seriesVisualizer.h"
#include "RangeSlider.h"
#include <vtkThresholdPoints.h>
#include <vtkTable.h>
#include <vtkDoubleArray.h>
#include <vtkKMeansStatistics.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLookupTable.h>
#include <vtkKMeansDistanceFunctor.h>
#include <vtkKMeansDistanceFunctorCalculator.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkMultiBlockDataSet.h>
#include <sstream>

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
