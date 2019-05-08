#include "roiVisualizer.h"

RoiVisualizer::RoiVisualizer(QFrame * vtk_frame, QString name, QFrame * slider_frame) :
	SeriesVisualizer(vtk_frame, name, slider_frame)
{
	roi_thresh = vtkSmartPointer<vtkImageThreshold>::New();
	roi_ss = vtkSmartPointer<vtkImageShiftScale>::New();
	roi_range_slider = vtk_frame->findChild<RangeSlider*>("roi_range_slider");
	roi_min_label = vtk_frame->findChild<QLabel*>("roi_min_label");
	roi_max_label = vtk_frame->findChild<QLabel*>("roi_max_label");
}

RoiVisualizer::~RoiVisualizer()
{

}

void RoiVisualizer::setOriginData(vtkSmartPointer<vtkImageData> input_data)
{
	SeriesVisualizer::setOriginData(input_data);

	double init_range[2];
	getOriginData()->GetScalarRange(init_range);
	roi_min_label->setText(QString::number(init_range[0]));
	roi_max_label->setText(QString::number(init_range[1]));
	roi_range_slider->setMinimum(int(init_range[0]));
	roi_range_slider->setMaximum(int(init_range[1]));

	setRoiGrayRange(float(init_range[0]), float(init_range[1]));
}

bool RoiVisualizer::setRoiGrayRange(float rMin, float rMax)
{
	if (rMin == roi_min && rMax == roi_max)
		return false;
	else
	{
		this->roi_min = rMin;
		roi_min_label->setText(QString::number(rMin));
		this->roi_max = rMax;
		roi_max_label->setText(QString::number(rMax)); 
		return true;
	}
}

void RoiVisualizer::updateVisualData()
{
	roi_thresh->ThresholdBetween(roi_min, roi_max);
	roi_thresh->SetReplaceOut(roi_min);
	roi_thresh->Update();

	vtkSmartPointer<vtkImageCast> ic = vtkSmartPointer< vtkImageCast>::New();
	ic->SetInputConnection(roi_thresh->GetOutputPort());
	ic->SetOutputScalarTypeToFloat();
	ic->Update();
	setTransferedData(ic->GetOutput());

	roi_ss->Update();
	setVisualData(roi_ss->GetOutput());

	viewer->SetInputData(getVisualData());
	int slice = viewer->GetSlice();
	viewer->SetSlice(slice);
	viewer->Render();
}

float RoiVisualizer::getRoiRangeMin()
{
	return roi_min;
}

float RoiVisualizer::getRoiRangeMax()
{
	return roi_max;
}

void RoiVisualizer::kMeansCalc()
{
	vtkSmartPointer<vtkImageData> src_data = vtkSmartPointer<vtkImageData>::New();
	src_data = getTransferedData();

	vtkSmartPointer<vtkThresholdPoints> thresh = vtkSmartPointer<vtkThresholdPoints>::New();
	thresh->SetInputData(src_data);
	thresh->ThresholdBetween(roi_min, roi_max);
	thresh->Update();
	cout << "KMeans points: " << thresh->GetOutput()->GetNumberOfPoints() << endl;

	vtkPointData * ele = thresh->GetOutput()->GetPointData();
	vtkDataArray * ele_array = ele->GetScalars();
	int num = ele_array->GetNumberOfTuples();
	cout << "KMeans points: " << num << endl;
}

void RoiVisualizer::transferData()
{
	double range[2];
	getOriginData()->GetScalarRange(range);
	cout << "src gray range: " << range[0] << " " << range[1] << endl;

	//thresh the gray data to get roi according to the roi range
	roi_thresh->SetInputData(getOriginData());
	roi_thresh->ThresholdBetween(roi_min, roi_max);
	roi_thresh->ReplaceInOff();
	roi_thresh->ReplaceOutOn();
	roi_thresh->SetReplaceOut(roi_min);
	roi_thresh->Update();

	vtkSmartPointer<vtkImageCast> ic = vtkSmartPointer< vtkImageCast>::New();
	ic->SetInputConnection(roi_thresh->GetOutputPort());
	ic->SetOutputScalarTypeToFloat();
	ic->Update();
	setTransferedData(ic->GetOutput());

	//shift and scale to 0 - 255 to visualize
	roi_ss->SetInputConnection(ic->GetOutputPort());
	roi_ss->SetShift(-roi_min);
	roi_ss->SetScale(255.0 / (roi_max - roi_min));
	roi_ss->Update();
	setVisualData(roi_ss->GetOutput());
}
