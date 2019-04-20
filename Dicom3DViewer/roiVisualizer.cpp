#include "roiVisualizer.h"

RoiVisualizer::RoiVisualizer(QFrame * vtk_frame, QString name, QFrame * slider_frame) :
	SeriesVisualizer(vtk_frame, name, slider_frame)
{
}

RoiVisualizer::~RoiVisualizer()
{

}

void RoiVisualizer::setRoiGrayRange(float roi_min, float roi_max)
{
	assert(roi_min <= roi_max);
	this->roi_min = roi_min;
	this->roi_max = roi_max;
}

void RoiVisualizer::transferData()
{
	double range[2];
	getOriginData()->GetScalarRange(range);
	cout << "src gray range: " << range[0] << " " << range[1] << endl;

	//thresh the gray data to get roi according to the roi range
	vtkSmartPointer<vtkImageThreshold> roi_thresh = vtkSmartPointer<vtkImageThreshold>::New();
	roi_thresh->SetInputData(getOriginData());
	roi_thresh->ThresholdBetween(roi_min, roi_max);
	roi_thresh->ReplaceInOff();
	roi_thresh->ReplaceOutOn();
	roi_thresh->SetReplaceOut(range[0]);
	roi_thresh->Update();

	vtkSmartPointer<vtkImageCast> ic = vtkSmartPointer< vtkImageCast>::New();
	ic->SetInputData(roi_thresh->GetOutput());
	ic->SetOutputScalarTypeToFloat();
	ic->Update();

	setVisualData(ic->GetOutput());
}
