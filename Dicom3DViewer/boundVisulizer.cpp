#include "boundVisulizer.h"

BoundVisualizer::BoundVisualizer(QFrame * vtk_frame, QString name, QFrame * slider_frame) :
	SeriesVisualizer(vtk_frame, name, slider_frame)
{
	imgMagnitude = vtkSmartPointer <vtkImageMagnitude>::New(); 
	mag_thresh_img = vtkSmartPointer<vtkImageThreshold>::New();
	nonMaxFloat = vtkSmartPointer<vtkImageData>::New();
	mag_thresh_poly = vtkSmartPointer<vtkThresholdPoints>::New();

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

float BoundVisualizer::getMaxBoundGradientValue()
{
	return mag_range[1];
}

float BoundVisualizer::getMinBoundGradientValue()
{
	return mag_threshold;
}

void BoundVisualizer::transferData()
{
	//thresh the magnitude
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
	imgMagnitude->SetInputConnection(imgGradient->GetOutputPort());
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
	nonMaxFloat = ic->GetOutput();

	//get range
	nonMaxFloat->GetScalarRange(mag_range);
	cout << "mag range: " << mag_range[0] << " " << mag_range[1] << endl;

	//thresh the magnitude
	mag_thresh_img->SetInputData(nonMaxFloat);

	mag_thresh_img->ThresholdByUpper(mag_threshold);
	mag_thresh_img->ReplaceInOff();
	mag_thresh_img->ReplaceOutOn();
	mag_thresh_img->SetOutValue(0);
	mag_thresh_img->Update();

	setVisualData(mag_thresh_img->GetOutput());

	//set slider value
	magnitude_cur_label->setText(QString::number(mag_threshold));
	magnitude_max_label->setText(QString::number(mag_range[1], 10, 0));
	magnitude_min_label->setText(QString::number(mag_range[0], 10, 0));
	magnitude_thresh_slider->setMaximum(mag_range[1]);
	magnitude_thresh_slider->setMinimum(mag_range[0]);
	magnitude_thresh_slider->setValue(mag_threshold);
}

void BoundVisualizer::updateVisualData()
{
	mag_thresh_img->ThresholdByUpper(mag_threshold);
	mag_thresh_img->Update();

	int slice = viewer->GetSlice();
	viewer->SetSlice(slice);
	viewer->Render();
}

void BoundVisualizer::calcRoiBoundPoly(vtkSmartPointer<vtkImageData> origin_gray, vtkSmartPointer<vtkImageData> origin_mag)
{
	roi_bound_gd.clear();
	roi_bound_gv.clear();

	mag_thresh_poly->SetUpperThreshold(mag_threshold);
	mag_thresh_poly->SetInputData(nonMaxFloat);
	mag_thresh_poly->Update();
	cout << "Number of points in max_thresh_poly: " << mag_thresh_poly->GetOutput()->GetNumberOfPoints() << endl;

	int dims[3];
	origin_gray->GetDimensions(dims);

	double poly_bounds[6];
	mag_thresh_poly->GetOutput()->GetBounds(poly_bounds);
	double spacing_x = (poly_bounds[1] - poly_bounds[0]) / (dims[0] - 1);
	double spacing_y = (poly_bounds[3] - poly_bounds[2]) / (dims[1] - 1);
	double spacing_z = (poly_bounds[5] - poly_bounds[4]) / (dims[2] - 1);

	//get range
	double gray_range[2];
	origin_gray->GetScalarRange(gray_range);
	double mag_range[2];
	origin_mag->GetScalarRange(mag_range);

	int gv_size = int(gray_range[1] + 0.5) - int(gray_range[0]);
	int * roi_bound_gvs = new int[gv_size];
	memset(roi_bound_gvs, 0, gv_size);

	int gd_size = int(mag_range[1] + 0.5) - int(mag_range[0]);
	int * roi_bound_gds = new int[gd_size];
	memset(roi_bound_gds, 0, gd_size);

	for (int i = 0; i < mag_thresh_poly->GetOutput()->GetNumberOfPoints(); i++)
	{
		//map poly coords to data coords
		double poly_coords[3];
		int data_coords[3];
		mag_thresh_poly->GetOutput()->GetPoint(i, poly_coords);
		data_coords[0] = (poly_coords[0] - poly_bounds[0]) / spacing_x;
		data_coords[1] = (poly_coords[1] - poly_bounds[2]) / spacing_y;
		data_coords[2] = (poly_coords[2] - poly_bounds[4]) / spacing_z;

		float * ele_mag = (float *)origin_mag->GetScalarPointer(data_coords);
		float * ele_gray = (float *)origin_gray->GetScalarPointer(data_coords);

		++roi_bound_gvs[int(*ele_gray - gray_range[0])];
		++roi_bound_gds[int(*ele_mag - mag_range[0])];
		/*roi_bound_gv.push_back(int(*ele_gray + 0.5));
		roi_bound_gd.push_back(int(*ele_magnitude + 0.5));*/
	}

	int * gv_max = max_element(roi_bound_gvs, roi_bound_gvs + gv_size);
	cout << "max gv: " << distance(roi_bound_gvs, gv_max) << " with " << *gv_max << " points" << endl;
	int * gd_max = max_element(roi_bound_gds, roi_bound_gds + gd_size);
	cout << "max gd: " << distance(roi_bound_gds, gd_max) << " with " << *gd_max << " points" << endl;
}

vector<int> BoundVisualizer::getRoiBoundGvs()
{
	return roi_bound_gv;
}

vector<int> BoundVisualizer::getRoiBoundGds()
{
	return roi_bound_gd;
}

