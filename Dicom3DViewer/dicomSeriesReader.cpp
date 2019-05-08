#include "dicomSeriesReader.h"

vtkStandardNewMacro(myVtkInteractorStyleImage);

DicomSeriesReader::DicomSeriesReader(QFrame *widget)
{
	img_viewer = vtkSmartPointer<vtkImageViewer2>::New();
	edge_viewer = vtkSmartPointer<vtkImageViewer2>::New();
	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	max_thresh_img = vtkSmartPointer<vtkImageThreshold>::New();
	max_thresh_poly = vtkSmartPointer<vtkThresholdPoints>::New();

	dicom_reader_widget = widget->findChild<QVTKWidget* >("dicomSlicerWidget");
	dicom_edge_widget = widget->findChild<QVTKWidget* >("dicomEdgeWidget");
	dicom_coords_label = widget->findChild<QLabel* >("dicom_coords_label");
	dicom_gray_label = widget->findChild<QLabel* >("dicom_gray_label");
	dicom_gradient_label = widget->findChild<QLabel* >("dicom_gradient_label");

	dicom_series_slider = widget->findChild<QSlider* >("dicom_series_slider");
	slice_max_label = widget->findChild<QLabel* >("slice_max_label");
	slice_min_label = widget->findChild<QLabel* >("slice_min_label");
	slice_cur_label = widget->findChild<QLabel* >("slice_cur_label");

	gradient_thresh_slider = widget->findChild<QSlider* >("gradient_thresh_slider");
	gradient_min_label = widget->findChild<QLabel* >("gradient_min_label");
	gradient_max_label = widget->findChild<QLabel* >("gradient_max_label");
	gradient_cur_label = widget->findChild<QLabel* >("gradient_cur_label");

	gradient_thresh = 1100;
	roi_gv_offset = 100;
}

DicomSeriesReader::~DicomSeriesReader()
{
}


void DicomSeriesReader::calcGradientMagnitude()
{
	// Calc gradient value
	// Smooth the image
	vtkSmartPointer<vtkImageGaussianSmooth> gs = vtkSmartPointer<vtkImageGaussianSmooth>::New();
	gs->SetInputConnection(dicoms_reader->GetOutputPort());
	gs->SetDimensionality(3);
	gs->SetRadiusFactors(1, 1, 0);

	imgGradient = vtkSmartPointer<vtkImageGradient>::New();
	imgGradient->SetInputConnection(gs->GetOutputPort());
	imgGradient->SetDimensionality(3);
	imgGradient->Update();
	cout << "img Gradient components: " << imgGradient->GetOutput()->GetNumberOfScalarComponents() << endl;

	imgMagnitude = vtkSmartPointer<vtkImageMagnitude>::New();
	imgMagnitude->SetInputConnection(imgGradient->GetOutputPort());
	imgMagnitude->Update();
	cout << "img imgMagnitude components: " << imgMagnitude->GetOutput()->GetNumberOfScalarComponents() << endl;

	//magnitude range
	imgMagnitude->GetOutput()->GetScalarRange(magnitude_range);

	gradient_min_label->setText(QString::number(magnitude_range[0], 10, 0));
	gradient_max_label->setText(QString::number(magnitude_range[1], 10, 0));
	gradient_cur_label->setText(QString::number(gradient_thresh, 10));
	gradient_thresh_slider->setMinimum(int(magnitude_range[0] + 0.5));
	gradient_thresh_slider->setMaximum(int(magnitude_range[1] + 0.5));
	gradient_thresh_slider->setValue(gradient_thresh);
}

double DicomSeriesReader::getMinGradientValue()
{
	return magnitude_range[0];
}

double DicomSeriesReader::getMaxGradientValue()
{
	return magnitude_range[1];
}

void DicomSeriesReader::drawDicomSeries(QString folder_path)
{
	// Read all the DICOM files in the specified directory.
	QByteArray ba = folder_path.toLocal8Bit();
	const char *folderName_str = ba.data();

	dicoms_reader->SetDirectoryName(folderName_str);
	dicoms_reader->Update();

	//gradient
	calcGradientMagnitude();

	img_viewer->SetInputConnection(dicoms_reader->GetOutputPort());
	dicom_reader_widget->SetRenderWindow(img_viewer->GetRenderWindow());
	img_viewer->SetupInteractor(dicom_reader_widget->GetInteractor());

	// set my interactor style
	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle = vtkSmartPointer<myVtkInteractorStyleImage>::New();
	myInteractorStyle->SetImageViewer(img_viewer);
	myInteractorStyle->SetSliderSlices(dicom_series_slider, slice_min_label, slice_max_label, slice_cur_label);
	dicom_reader_widget->GetInteractor()->SetInteractorStyle(myInteractorStyle);
	dicom_reader_widget->GetInteractor()->Initialize();

	img_viewer->GetRenderer()->ResetCamera();
	img_viewer->Render();
	dicom_reader_widget->GetInteractor()->Start();
}

void DicomSeriesReader::cannyEdgeExtraction()
{	
	// Non maximum suppression
	vtkSmartPointer<vtkImageNonMaximumSuppression> nonMax = vtkSmartPointer<vtkImageNonMaximumSuppression>::New();
	nonMax->SetMagnitudeInputData(imgMagnitude->GetOutput());
	nonMax->SetVectorInputData(imgGradient->GetOutput());
	nonMax->SetDimensionality(3);
	nonMax->Update();

	vtkSmartPointer<vtkImageData> imageData = vtkSmartPointer<vtkImageData>::New();
	imageData = nonMax->GetOutput();

	int* dims = imageData->GetDimensions();
	cout << "Image data dims: " << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2] << endl;
	cout << "Number of points: " << imageData->GetNumberOfPoints() << endl;
	cout << "Number of cells: " << imageData->GetNumberOfCells() << endl;

	//change scalar to float
	ic = vtkSmartPointer<vtkImageCast>::New();
	ic->SetOutputScalarTypeToDouble();
	ic->SetInputData(imageData);
	ic->Update();
	cout << "Number of components: " << ic->GetOutput()->GetNumberOfScalarComponents() << endl;

	//set max magnitude thresh with vtkImageThreshold
	max_thresh_img->ThresholdByUpper(gradient_thresh);
	max_thresh_img->SetOutValue(0);
	max_thresh_img->ReplaceOutOn();
	max_thresh_img->ReplaceInOff();
	max_thresh_img->SetInputData(ic->GetOutput());
	max_thresh_img->Update();

	//set max magnitude thresh with vtkThresholdPoints
	max_thresh_poly->ThresholdByUpper(gradient_thresh);
	max_thresh_poly->SetInputData(ic->GetOutput());
	max_thresh_poly->Update();
	cout << "Number of points in max_thresh_poly: " << max_thresh_poly->GetOutput()->GetNumberOfPoints() << endl;

	//show magnitude(edge) after thresh
	edge_viewer->SetInputConnection(max_thresh_img->GetOutputPort());
	dicom_edge_widget->SetRenderWindow(edge_viewer->GetRenderWindow());
	edge_viewer->SetupInteractor(dicom_edge_widget->GetInteractor());

	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle1 = vtkSmartPointer<myVtkInteractorStyleImage>::New();
	myInteractorStyle1->SetImageViewer(edge_viewer);
	myInteractorStyle1->SetSliderSlices(dicom_series_slider, slice_min_label, slice_max_label, slice_cur_label);
	dicom_edge_widget->GetInteractor()->SetInteractorStyle(myInteractorStyle1);
	dicom_edge_widget->GetInteractor()->Initialize();

	edge_viewer->SetSlice(0);
	edge_viewer->GetRenderer()->ResetCamera();
	edge_viewer->Render();
	dicom_edge_widget->GetInteractor()->Start();

	//show edge
	//vtkSmartPointer<vtkPolyData> pointsPolydata = vtkSmartPointer<vtkPolyData>::New();
	//pointsPolydata = max_thresh_poly->GetOutput();

	//vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	//vertexFilter->SetInputData(pointsPolydata);
	//vertexFilter->Update();

	//vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
	//polydata->ShallowCopy(vertexFilter->GetOutput());
	//// Setup colors
	//vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	//colors->SetNumberOfComponents(1);
	//colors->SetName("Colors");

	//double pixel_range[2];
	//ic->GetOutput()->GetScalarRange(pixel_range);
	//double gap = pixel_range[1] - pixel_range[0];

	//int dimension[3];
	//ic->GetOutput()->GetDimensions(dimension);

	//double poly_bounds[6];
	//polydata->GetBounds(poly_bounds);
	//double spacing_x = (poly_bounds[1] - poly_bounds[0]) / (dimension[0] - 1);
	//double spacing_y = (poly_bounds[3] - poly_bounds[2]) / (dimension[1] - 1);
	//double spacing_z = (poly_bounds[5] - poly_bounds[4]) / (dimension[2] - 1);

	//for (int i = 0; i < polydata->GetNumberOfPoints(); i++)
	//{
	//	double coords[3];
	//	polydata->GetPoint(i, coords);
	//	float * pixel = (float *)ic->GetOutput()->GetScalarPointer(
	//		(coords[0] - poly_bounds[0]) / spacing_x, (coords[1] - poly_bounds[2]) / spacing_y, (coords[2] - poly_bounds[4]) / spacing_z);
	//	colors->InsertNextTuple1((*pixel - pixel_range[0]) * 255.0 / gap);
	//}

	//polydata->GetPointData()->SetScalars(colors);

	//vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	//mapper->SetInputData(polydata);

	//vtkSmartPointer<vtkActor>  actor = vtkSmartPointer<vtkActor>::New();
	//actor->SetMapper(mapper);

	//vtkSmartPointer<vtkRenderer> render = vtkSmartPointer<vtkRenderer>::New();
	//render->AddActor(actor);
	//render->SetBackground(.0, .0, .0);

	//dicom_edge_widget->GetRenderWindow()->AddRenderer(render);
	//render->ResetCamera();
	//render->Render();
	//dicom_edge_widget->GetInteractor()->Start();



	//**********************************************************************************************

	//traverseImageData(nonMax->GetOutput());
	
	////show edge
	//edge_viewer->SetInputConnection(nonMax->GetOutputPort());
	//dicom_edge_widget->SetRenderWindow(edge_viewer->GetRenderWindow());
	//edge_viewer->SetupInteractor(dicom_edge_widget->GetInteractor());


	//vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle1 = vtkSmartPointer<myVtkInteractorStyleImage>::New();
	//myInteractorStyle1->SetImageViewer(edge_viewer);
	//myInteractorStyle1->SetSliderSlices(dicom_series_slider, slice_min_label, slice_max_label, slice_cur_label);
	//dicom_edge_widget->GetInteractor()->SetInteractorStyle(myInteractorStyle1);
	//dicom_edge_widget->GetInteractor()->Initialize();

	//edge_viewer->SetSlice(0);
	//edge_viewer->GetRenderer()->ResetCamera();
	//edge_viewer->Render();
	//dicom_edge_widget->GetInteractor()->Start();


	/*edge_viewer->GetRenderer()->ResetCamera();
	edge_viewer->Render();
	dicom_edge_widget->GetInteractor()->Start();*/

	//get the centre point of slice plane
	//double center[3];
	////center[0] = origin[0] + spacing[0] * 0.5 * (extent[0] + extent[1]);
	//center[0] = origin[0];
	//center[1] = origin[1];
	//center[2] = origin[2];
	//static double axialElements[16] = {
	//	1, 0, 0, 0,
	//	0, 1, 0, 0,
	//	0, 0, 1, 0,
	//	0, 0, 0, 1
	//};

	//vtkSmartPointer<vtkMatrix4x4> resliceAxes = vtkSmartPointer<vtkMatrix4x4>::New();
	//resliceAxes->DeepCopy(axialElements);
	//resliceAxes->SetElement(0, 3, center[0]);
	//resliceAxes->SetElement(1, 3, center[1]);
	//resliceAxes->SetElement(2, 3, center[2]);

	//vtkSmartPointer<vtkImageReslice> reslice = vtkSmartPointer<vtkImageReslice>::New();
	//reslice->SetInputConnection(nonMax->GetOutputPort());
	//reslice->SetOutputDimensionality(2);
	//reslice->SetResliceAxes(resliceAxes);
	//reslice->SetInterpolationModeToLinear();

	//vtkSmartPointer<vtkLookupTable> colorTable = vtkSmartPointer<vtkLookupTable>::New();
	//colorTable->SetRange(0, 1000);
	//colorTable->SetValueRange(0.0, 1.0);
	//colorTable->SetSaturationRange(0.0, 0.0);
	//colorTable->SetRampToLinear();
	//colorTable->Build();

	//vtkSmartPointer<vtkImageMapToColors> colorMap = vtkSmartPointer<vtkImageMapToColors>::New();
	//colorMap->SetLookupTable(colorTable);
	//colorMap->SetInputConnection(reslice->GetOutputPort());
	//colorMap->Update();

	//vtkSmartPointer<vtkImageActor> imgActor = vtkSmartPointer<vtkImageActor>::New();
	//imgActor->SetInputData(colorMap->GetOutput());

	//vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	//renderer->AddActor(imgActor);
	//renderer->SetBackground(1.0, 1.0, 1.0);

	//dicom_edge_widget->GetRenderWindow()->AddRenderer(renderer);
	//dicom_edge_widget->GetRenderWindow()->Render();
	//dicom_edge_widget->GetInteractor()->Start();
}

void DicomSeriesReader::traverseImageData(vtkImageData * imageData)
{
	


	//int count = 0;
	//int zero_count = 0;
	//for (int z = 0; z < dims[2]; z++)
	//{
	//	for (int y = 0; y < dims[1]; y++)
	//	{
	//		for (int x = 0; x < dims[0]; x++)
	//		{
	//			float * pixel = (float *)ic->GetOutput()->GetScalarPointer(x, y, z);
	//			if (*pixel >= 2)//2051284
	//				++zero_count;
	//			++count;
	//		}
	//	}
	//}
	//cout << "Zero Counts: " << zero_count << endl;
	//cout << "Counts: " << count << endl;

	/*int extent[6];
	ic->GetOutput()->GetExtent(extent);
	int count = 0;
	int zero_count = 0;
	vtkImageIterator<float> it(ic->GetOutput(), extent);
	while (!it.IsAtEnd())
	{
		++count;
		it.NextSpan();
		float * inS = it.BeginSpan();
		float * inSEnd = it.EndSpan();
		if (*inS == 0)
			++zero_count;
	}
	cout << "Zero Counts: " << zero_count << endl;
	cout << "Counts: " << count << endl;*/
}

void DicomSeriesReader::dicomSeriseSlideMove(int pos)
{
	img_viewer->SetSlice(pos);
	img_viewer->Render();

	edge_viewer->SetSlice(pos);
	edge_viewer->Render();

	slice_cur_label->setText(QString::number(pos, 10));
}

void DicomSeriesReader::gradientThreshSlideMove(int pos)
{
	gradient_thresh = pos;

	max_thresh_img->ThresholdByUpper(gradient_thresh);
	max_thresh_img->Update();

	int cur_slice = edge_viewer->GetSlice();
	edge_viewer->Render();
	edge_viewer->SetSlice(cur_slice);

	gradient_cur_label->setText(QString::number(pos, 10));
}

float DicomSeriesReader::getRoiGray()
{
	return roi_gv;
}

vtkImageData * DicomSeriesReader::getImageGradientData()
{
	return imgGradient->GetOutput();
}

vtkImageData * DicomSeriesReader::getImageMagnitudeData()
{
	return imgMagnitude->GetOutput();
}

vtkImageData * DicomSeriesReader::getImageGrayData()
{
	return dicoms_reader->GetOutput();
}

vtkThresholdPoints * DicomSeriesReader::getBoundMagnitudePoly()
{
	return max_thresh_poly;
}

double DicomSeriesReader::getPositionGvAndGd(int x, int y)
{
	//show coords
	dicom_coords_label->setText("X=" + QString::number(x) + " Y=" + QString::number(y) + " Z=" + QString::number(img_viewer->GetSlice()));

	vtkSmartPointer<vtkImageData> image = img_viewer->GetInput();
	vtkSmartPointer<vtkPointData> pointData_gv = vtkSmartPointer<vtkPointData>::New();
	vtkSmartPointer<vtkImageData> gradient = imgMagnitude->GetOutput();
	vtkSmartPointer<vtkPointData> pointData_gd = vtkSmartPointer<vtkPointData>::New();

	vtkSmartPointer<vtkWorldPointPicker> picker = vtkSmartPointer<vtkWorldPointPicker>::New();

	double pickCoords[3];
	picker->Pick(x, y, 0, img_viewer->GetRenderer());
	picker->GetPickPosition(pickCoords);

	// Fixes some numerical problems with the picking
	double *bounds = img_viewer->GetImageActor()->GetDisplayBounds();
	int axis = img_viewer->GetSliceOrientation();
	pickCoords[axis] = bounds[2 * axis];

	vtkPointData* pd_gv = image->GetPointData();
	if (!pd_gv)
	{
		return 0.0;
	}
	vtkPointData* pd_gd = gradient->GetPointData();
	if (!pd_gd)
	{
		return 0.0;
	}
	
	pointData_gv->InterpolateAllocate(pd_gv, 1, 1);
	pointData_gd->InterpolateAllocate(pd_gd, 1, 1);

	// Use tolerance as a function of size of source data
	double tol2 = image->GetLength();
	tol2 = tol2 ? tol2 * tol2 / 1000.0 : 0.001;

	// Find the cell that contains pos
	int subId_gv;
	double pcoords_gv[3], weights_gv[8];
	vtkCell* cell_gv = image->FindAndGetCell(pickCoords, NULL, -1, tol2, subId_gv, pcoords_gv, weights_gv);
	if (cell_gv)
	{
		// Interpolate the point data
		pointData_gv->InterpolatePoint(pd_gv, 0, cell_gv->PointIds, weights_gv);
		double* tuple = pointData_gv->GetScalars()->GetTuple(0);
		dicom_gray_label->setText(QString::number(tuple[0], 10, 2));
		roi_gv = tuple[0];
	}
	else
		dicom_gray_label->setText("None");

	// Find the cell that contains pos
	int subId_gd;
	double pcoords_gd[3], weights_gd[8];
	vtkCell* cell_gd = gradient->FindAndGetCell(pickCoords, NULL, -1, tol2, subId_gd, pcoords_gd, weights_gd);
	if (cell_gd)
	{
		// Interpolate the point data
		pointData_gd->InterpolatePoint(pd_gd, 0, cell_gd->PointIds, weights_gd);
		double* tuple = pointData_gd->GetScalars()->GetTuple(0);
		dicom_gradient_label->setText(QString::number(tuple[0], 10, 2));
	}
	else
		dicom_gradient_label->setText("None");
	
	return 0.0;
}

/*
Purpose: Find the ROI related points among all the edge points
Algorithm:
for ele in edge_points:
	   cur_point = ele;
	1. axis = max(cur_point.gd[3])
	   orient = axis * (roi.gv - ele.gv) > 0 ? -1 : 1;
	2. next_point = get next point according to axis and orient

	3. if next_point.gv in rio.range:
		 select ele;
	   else if next_point.gd < ele.gd/2
		 discard ele;
	   else:
		 cur_point = next_point;
		 repeat 1;

*/
void DicomSeriesReader::findROIBound()
{
	vtkSmartPointer<vtkImageData> imageGradientData = imgGradient->GetOutput();
	vtkSmartPointer<vtkImageData> imageMagnitudeData = imgMagnitude->GetOutput();
	vtkSmartPointer<vtkImageData> imageGrayData = dicoms_reader->GetOutput();

	cout << "imageGradientData com: " << imageGradientData->GetNumberOfScalarComponents() << endl;

	vtkSmartPointer <vtkImageCast> ic1 = vtkSmartPointer<vtkImageCast>::New();
	ic1->SetOutputScalarTypeToDouble();
	ic1->SetInputData(imageGradientData);
	ic1->Update();
	imageGradientData = ic1->GetOutput();

	cout << "imageGradientData: " << *(float*)imageGradientData->GetScalarPointer(100, 100, 20) << endl;
	cout << "imageGradientData com: " << imageGradientData->GetNumberOfScalarComponents() << endl;

	vtkSmartPointer <vtkImageCast> ic2 = vtkSmartPointer<vtkImageCast>::New();
	ic2->SetInputData(imageMagnitudeData);
	ic2->Update();
	imageMagnitudeData = ic2->GetOutput();
	cout << "imageGradientData: " << *(float*)imageMagnitudeData->GetScalarPointer(100, 100, 20) << endl;

	vtkSmartPointer <vtkImageCast> ic3 = vtkSmartPointer<vtkImageCast>::New();
	ic3->SetInputData(imageGrayData);
	ic3->Update();
	imageGrayData = ic3->GetOutput();
	cout << "imageGradientData: " << *(float*)imageGrayData->GetScalarPointer(100, 100, 20) << endl;


	assert(imageGradientData->GetNumberOfScalarComponents() == 3 &&
		imageMagnitudeData->GetNumberOfScalarComponents() == 1 &&
		imageGrayData->GetNumberOfScalarComponents() == 1);

	int dims2[3], dims3[3];
	imageGradientData->GetDimensions(dims);
	imageMagnitudeData->GetDimensions(dims2);
	imageGrayData->GetDimensions(dims3);
	assert(equal(dims, dims + 3, dims2, dims2 + 3) && equal(dims3, dims3 + 3, dims2, dims2 + 3));

	//calc
	max_thresh_poly->ThresholdByUpper(gradient_thresh);
	max_thresh_poly->Update();
	cout << "Number of points in max_thresh_poly: " << max_thresh_poly->GetOutput()->GetNumberOfPoints() << endl;

	double poly_bounds[6];
	max_thresh_poly->GetOutput()->GetBounds(poly_bounds);
	double spacing_x = (poly_bounds[1] - poly_bounds[0]) / (dims[0] - 1);
	double spacing_y = (poly_bounds[3] - poly_bounds[2]) / (dims[1] - 1);
	double spacing_z = (poly_bounds[5] - poly_bounds[4]) / (dims[2] - 1);

	float end_mag_ratio = 0.5;

	for (int i = 0; i < max_thresh_poly->GetOutput()->GetNumberOfPoints(); i++)
	{
		//map poly coords to data coords
		double poly_coords[3];
		int data_coords[3];
		max_thresh_poly->GetOutput()->GetPoint(i, poly_coords);
		data_coords[0] = (poly_coords[0] - poly_bounds[0]) / spacing_x;
		data_coords[1] = (poly_coords[1] - poly_bounds[2]) / spacing_y;
		data_coords[2] = (poly_coords[2] - poly_bounds[4]) / spacing_z;

		//get the gradient in three dimentions
		float * ele_gradient = (float *)imageGradientData->GetScalarPointer(data_coords);
		float * ele_magnitude = (float *)imageMagnitudeData->GetScalarPointer(data_coords);
		float * ele_gray = (float *)imageGrayData->GetScalarPointer(data_coords);

		float * cur_gradient = ele_gradient;
		float * cur_magnitude = ele_magnitude;
		float * cur_gray = ele_gray;

		while (*cur_magnitude > *ele_magnitude * end_mag_ratio && isOutOfImage(data_coords))
		{
			if (*cur_gray <= roi_gv + roi_gv_offset && *cur_gray >= roi_gv - roi_gv_offset)//the gv of cur point in the roi range, then the ele is a roi bound point.
			{
				roi_bound_gd.push_back(*ele_magnitude);
				roi_bound_gv.push_back(*ele_gray);
				break;
			}
			else//find next point along the max gradient axis and orientation
			{
				int * axis_orient = calcMaxGradientAxisAndOrient(cur_gradient, *cur_gray);
				int axis = axis_orient[0];
				int orient = axis_orient[1];

				data_coords[axis] = data_coords[axis] + orient;
				float * cur_gradient = (float *)imageGradientData->GetScalarPointer(data_coords);
				float * cur_magnitude = (float *)imageMagnitudeData->GetScalarPointer(data_coords);
				float * cur_gray = (float *)imageGrayData->GetScalarPointer(data_coords);
			}
		}
	}

	cout << "roi_bound_gd size: " << roi_bound_gd.size() << endl;
	for each (float gd in roi_bound_gd)
	{
		cout << gd << endl;
	}
	cout << "roi_bound_gv size: " << roi_bound_gv.size() << endl;
	for each (float gv in roi_bound_gv)
	{
		cout << gv << endl;
	}
}

int * DicomSeriesReader::calcMaxGradientAxisAndOrient(float * gradient, float cur_gv)
{
	int axis = 0;
	int orient = 1;

	if (abs(gradient[0]) >= abs(gradient[1]) && abs(gradient[0]) >= abs(gradient[2]))
		axis = 0;
	else if (abs(gradient[1]) >= abs(gradient[0]) && abs(gradient[1]) >= abs(gradient[2]))
		axis = 1;
	else
		axis = 2;

	if (gradient[axis] > 0)
	{
		if (cur_gv < roi_gv - roi_gv_offset)
			orient = -1;
		else if (cur_gv > roi_gv + roi_gv_offset)
			orient = 1;
	}
	else
	{
		if (cur_gv < roi_gv - roi_gv_offset)
			orient = 1;
		else if (cur_gv > roi_gv + roi_gv_offset)
			orient = -1;
	}

	int res[2];
	res[0] = axis;
	res[1] = orient;

	return res;
}

bool DicomSeriesReader::isOutOfImage(int * coords)
{
	if (coords[0] < 0 || coords[0] >= dims[0])
		return false;
	if (coords[1] < 0 || coords[1] >= dims[1])
		return false;
	if (coords[2] < 0 || coords[2] >= dims[2])
		return false;
	return true;
}