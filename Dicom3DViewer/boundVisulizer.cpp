#include "boundVisulizer.h"

BoundVisualizer::BoundVisualizer(QFrame * vtk_frame, QString name, QFrame * slider_frame) :
	SeriesVisualizer(vtk_frame, name, slider_frame)
{
}

BoundVisualizer::~BoundVisualizer()
{
}

void BoundVisualizer::setMagnitudeThresh(float threshold)
{
	mag_threshold = threshold;
}

void BoundVisualizer::transferData()
{
	double range[2];
	getOriginData()->GetScalarRange(range);
	cout << "roi gray range: " << range[0] << " " << range[1] << endl;

	//calc the gradient magnitude of original data and thresh with mag_thresh
	// Smooth the image
	vtkSmartPointer<vtkImageGaussianSmooth> gs = vtkSmartPointer<vtkImageGaussianSmooth>::New();
	gs->SetInputData(getOriginData());
	gs->SetDimensionality(3);
	gs->SetRadiusFactors(1, 1, 0);

	//gradient with centre difference in three dimentions
	vtkSmartPointer <vtkImageGradient> imgGradient = vtkSmartPointer<vtkImageGradient>::New();
	imgGradient->SetInputConnection(gs->GetOutputPort());
	imgGradient->SetDimensionality(3);
	imgGradient->Update();
	
	//gradient magnitude
	vtkSmartPointer <vtkImageMagnitude> imgMagnitude = vtkSmartPointer <vtkImageMagnitude>::New();
	imgMagnitude->SetInputData(imgGradient->GetOutput());
	imgMagnitude->Update();

	//non maximum suppression
	vtkSmartPointer<vtkImageNonMaximumSuppression> nonMax = vtkSmartPointer<vtkImageNonMaximumSuppression>::New();
	nonMax->SetMagnitudeInputData(imgMagnitude->GetOutput());
	nonMax->SetVectorInputData(imgGradient->GetOutput());
	nonMax->SetDimensionality(3);
	nonMax->Update();

	//thresh the magnitude
	vtkSmartPointer<vtkImageThreshold> mag_thresh = vtkSmartPointer<vtkImageThreshold>::New();
	mag_thresh->SetInputData(nonMax->GetOutput());
	mag_thresh->ThresholdByUpper(mag_threshold);
	mag_thresh->Update();

	vtkSmartPointer<vtkImageCast> ic = vtkSmartPointer< vtkImageCast>::New();
	ic->SetInputData(mag_thresh->GetOutput());
	ic->SetOutputScalarTypeToFloat();
	ic->Update();

	setVisualData(ic->GetOutput());
}


