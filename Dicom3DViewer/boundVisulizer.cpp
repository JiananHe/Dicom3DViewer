#include "boundVisulizer.h"

BoundVisualizer::BoundVisualizer(QFrame * vtk_frame, QString name, QFrame * slider_frame) :
	SeriesVisualizer(vtk_frame, name, slider_frame)
{
	imgMagnitude = vtkSmartPointer <vtkImageMagnitude>::New(); 
	mag_thresh = vtkSmartPointer<vtkImageThreshold>::New();

	magnitude_thresh_slider = vtk_frame->findChild<QSlider*>("magnitude_thresh_slider");
	magnitude_max_label = vtk_frame->findChild<QLabel*>("magnitude_max_label");
	magnitude_min_label = vtk_frame->findChild<QLabel*>("magnitude_min_label");
	magnitude_cur_label = vtk_frame->findChild<QLabel*>("magnitude_cur_label");

	mag_threshold = 0;
}

BoundVisualizer::~BoundVisualizer()
{
}

void BoundVisualizer::setMagnitudeThresh(float threshold)
{
	mag_threshold = threshold;
}

QString BoundVisualizer::getPositionMag(int x, int y)
{
	vtkSmartPointer<vtkImageData> gradient = imgMagnitude->GetOutput();
	vtkSmartPointer<vtkPointData> pointData_gd = vtkSmartPointer<vtkPointData>::New();

	vtkSmartPointer<vtkWorldPointPicker> picker = vtkSmartPointer<vtkWorldPointPicker>::New();

	double pickCoords[3];
	picker->Pick(x, y, 0, viewer->GetRenderer());
	picker->GetPickPosition(pickCoords);

	// Fixes some numerical problems with the picking
	double *bounds = viewer->GetImageActor()->GetDisplayBounds();
	int axis = viewer->GetSliceOrientation();
	pickCoords[axis] = bounds[2 * axis];

	vtkPointData* pd_gd = gradient->GetPointData();
	if (!pd_gd)
	{
		return "None";
	}

	pointData_gd->InterpolateAllocate(pd_gd, 1, 1);

	// Use tolerance as a function of size of source data
	double tol2 = gradient->GetLength();
	tol2 = tol2 ? tol2 * tol2 / 1000.0 : 0.001;

	// Find the cell that contains pos
	int subId_gv;
	double pcoords_gv[3], weights_gv[8];
	vtkCell* cell_gv = gradient->FindAndGetCell(pickCoords, NULL, -1, tol2, subId_gv, pcoords_gv, weights_gv);

	// Find the cell that contains pos
	int subId_gd;
	double pcoords_gd[3], weights_gd[8];
	vtkCell* cell_gd = gradient->FindAndGetCell(pickCoords, NULL, -1, tol2, subId_gd, pcoords_gd, weights_gd);
	if (cell_gd)
	{
		// Interpolate the point data
		pointData_gd->InterpolatePoint(pd_gd, 0, cell_gd->PointIds, weights_gd);
		double* tuple = pointData_gd->GetScalars()->GetTuple(0);
		return QString::number(tuple[0], 10, 2);
	}
	else
		return "None";
}

void BoundVisualizer::updateVisualData()
{
	//thresh the magnitude
	/*vtkSmartPointer<vtkImageThreshold> mag_thresh = vtkSmartPointer<vtkImageThreshold>::New();
	mag_thresh->SetInputData(getVisualData());*/
	mag_thresh->ThresholdByUpper(mag_threshold);
	mag_thresh->Update();

	/*vtkSmartPointer<vtkImageCast> ic = vtkSmartPointer< vtkImageCast>::New();
	ic->SetInputData(mag_thresh->GetOutput());
	ic->SetOutputScalarTypeToFloat();
	ic->Update();*/

	setVisualData(mag_thresh->GetOutput());

	//viewer->SetInputData(getVisualData());
	int slice = viewer->GetSlice();
	viewer->SetSlice(slice);
	viewer->Render();
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
	imgMagnitude->SetInputData(imgGradient->GetOutput());
	imgMagnitude->Update();

	//non maximum suppression
	vtkSmartPointer<vtkImageNonMaximumSuppression> nonMax = vtkSmartPointer<vtkImageNonMaximumSuppression>::New();
	nonMax->SetMagnitudeInputData(imgMagnitude->GetOutput());
	nonMax->SetVectorInputData(imgGradient->GetOutput());
	nonMax->SetDimensionality(3);
	nonMax->Update();

	vtkSmartPointer<vtkImageCast> ic = vtkSmartPointer< vtkImageCast>::New();
	ic->SetInputData(nonMax->GetOutput());
	ic->SetOutputScalarTypeToFloat();
	ic->Update();

	//thresh the magnitude
	mag_thresh->SetInputData(ic->GetOutput());
	mag_thresh->ThresholdByUpper(mag_threshold);
	mag_thresh->Update();

	setVisualData(mag_thresh->GetOutput());
}

void BoundVisualizer::setMagSliderValue()
{
	double mag_range[2];
	getVisualData()->GetScalarRange(mag_range);

	magnitude_thresh_slider->setMaximum(int(mag_range[1]));
	magnitude_thresh_slider->setMinimum(int(mag_range[0]));
	magnitude_max_label->setText(QString::number(mag_range[1]));
	magnitude_min_label->setText(QString::number(mag_range[0]));
	magnitude_cur_label->setText(QString::number(mag_threshold));
}

