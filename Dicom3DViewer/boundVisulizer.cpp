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
	magnitude_cur_label->setText(QString::number(mag_threshold));
}

map<double, double> BoundVisualizer::getRoiBoundMagBp()
{
	roi_bound_gd.clear();
	double res[3] = { 0.0 };

	mag_thresh_poly->SetUpperThreshold(mag_threshold);
	mag_thresh_poly->SetInputData(nonMaxFloat);
	mag_thresh_poly->Update();

	int total_count = mag_thresh_poly->GetOutput()->GetNumberOfPoints();
	mag_thresh_poly->GetOutput()->GetScalarRange(mag_range);

	cout << "roi bound mag range: " << mag_range[0] << " " << mag_range[1] << endl;
	res[0] = mag_range[0];
	res[2] = mag_range[1];

	int gd_size = int(mag_range[1] + 0.5) - int(mag_range[0]);
	int * roi_bound_gds = new int[gd_size];
	memset(roi_bound_gds, 0, gd_size * sizeof(int));

	multimap<int, double> roi_num_gd;
	multimap<double, double> roi_gd_gap;

	vtkPointData * ele = mag_thresh_poly->GetOutput()->GetPointData();
	vtkDataArray * ele_array = ele->GetScalars();
	int num = ele_array->GetNumberOfValues();
	for (int j = 0; j < num; j++)
	{
		double * t = ele_array->GetTuple(j);
		++roi_bound_gds[int(*t - mag_range[0])];
	}

	int * gd_max = max_element(roi_bound_gds, roi_bound_gds + gd_size);
	res[1] = distance(roi_bound_gds, gd_max) + mag_range[0];
	cout << "max gd: " << res[1] << " with " << *gd_max << " points" << endl;
	cout << roi_bound_gd.max_size() << endl;
	for (int i = 0; i < gd_size; i++)
	{
		double mag = i + mag_range[0];
		int mag_num = roi_bound_gds[i];
		//if (mag_num < total_count / gd_size)
		if(mag_num != 0)
			roi_num_gd.insert(pair<int, double>(mag_num, mag));
	}

	double max_num_gd_opacity = 1.0;
	int bp_num = 5;
	int gd_gap = gd_size / bp_num;

	multimap<int, double>::iterator iter = roi_num_gd.begin();
	int j = 0;
	double gd_gap_sum = .0;
	double gd_count_sum = .0;
	double gd_count_sum_max = .0;
	for (; iter != roi_num_gd.end(); ++iter)
	{
		gd_gap_sum += iter->second;
		gd_count_sum += iter->first;
		++j;

		if (j == gd_gap)
		{
			roi_gd_gap.insert(pair<double, double>(gd_gap_sum / j, gd_count_sum));
			if (gd_count_sum_max < gd_count_sum)
				gd_count_sum_max = gd_count_sum;
			gd_gap_sum = .0;
			gd_count_sum = .0;
			j = 0;
		}
	}
	if (j != 0)
	{
		roi_gd_gap.insert(pair<double, double>(gd_gap_sum / j, gd_count_sum));
		if (gd_count_sum_max < gd_count_sum)
			gd_count_sum_max = gd_count_sum;
	}

	multimap<double, double>::iterator iter1 = roi_gd_gap.begin();
	double mag = iter1->first;
	roi_bound_gd.insert(pair<double, double>(mag - 1, 0.0));
	for (; iter1 != roi_gd_gap.end(); ++iter1)
	{
		mag = iter1->first;
		double mag_num_sum = iter1->second;
		double opacity = mag_num_sum / gd_count_sum_max * max_num_gd_opacity;
		roi_bound_gd.insert(pair<double, double>(mag, opacity));
	}
	roi_bound_gd.insert(pair<double, double>(mag + 1, 0.0));

	return roi_bound_gd;
}
