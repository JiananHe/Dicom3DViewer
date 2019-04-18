#include "dicomSeriesReader.h"

vtkStandardNewMacro(myVtkInteractorStyleImage);

DicomSeriesReader::DicomSeriesReader(QFrame *widget)
{
	img_viewer = vtkSmartPointer<vtkImageViewer2>::New();
	edge_viewer = vtkSmartPointer<vtkImageViewer2>::New();
	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();

	dicom_reader_widget = widget->findChild<QVTKWidget* >("dicomSlicerWidget");
	dicom_edge_widget = widget->findChild<QVTKWidget* >("dicomEdgeWidget");
	dicom_coords_label = widget->findChild<QLabel* >("dicom_coords_label");
	dicom_gray_label = widget->findChild<QLabel* >("dicom_gray_label");
	dicom_gradient_label = widget->findChild<QLabel* >("dicom_gradient_label");

	dicom_series_slider = widget->findChild<QSlider* >("dicom_series_slider");
	slice_max_label = widget->findChild<QLabel* >("slice_max_label");
	slice_min_label = widget->findChild<QLabel* >("slice_min_label");
	slice_cur_label = widget->findChild<QLabel* >("slice_cur_label");
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

	imgMagnitude = vtkSmartPointer<vtkImageMagnitude>::New();
	imgMagnitude->SetInputConnection(imgGradient->GetOutputPort());
	imgMagnitude->Update();
}

double DicomSeriesReader::getMinGradientValue()
{
	double range[2];
	imgMagnitude->GetOutput()->GetScalarRange(range);
	return range[0];
}

double DicomSeriesReader::getMaxGradientValue()
{
	double range[2];
	imgMagnitude->GetOutput()->GetScalarRange(range);
	return range[1];
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

	vtkSmartPointer<vtkImageCast> ic = vtkSmartPointer<vtkImageCast>::New();
	ic->SetOutputScalarTypeToFloat();
	ic->SetInputData(imageData);
	ic->Update();

	cout << "Number of components: " << ic->GetOutput()->GetNumberOfScalarComponents() << endl;

	vtkSmartPointer<vtkThresholdPoints> min_thresh = vtkSmartPointer<vtkThresholdPoints>::New();
	min_thresh->ThresholdByUpper(100);
	min_thresh->SetInputData(ic->GetOutput());
	min_thresh->Update();

	cout << min_thresh->GetOutput()->GetNumberOfPoints() << endl;
	cout << min_thresh->GetOutput()->GetNumberOfCells() << endl;

	//show edge
	vtkSmartPointer<vtkPolyData> pointsPolydata = vtkSmartPointer<vtkPolyData>::New();
	pointsPolydata = min_thresh->GetOutput();

	vtkSmartPointer<vtkVertexGlyphFilter> vertexFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	vertexFilter->SetInputData(pointsPolydata);
	vertexFilter->Update();

	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
	polydata->ShallowCopy(vertexFilter->GetOutput());
	// Setup colors
	vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
	colors->SetNumberOfComponents(1);
	colors->SetName("Colors");

	double pixel_range[2];
	ic->GetOutput()->GetScalarRange(pixel_range);
	double gap = pixel_range[1] - pixel_range[0];

	int dimension[3];
	ic->GetOutput()->GetDimensions(dimension);

	double poly_bounds[6];
	polydata->GetBounds(poly_bounds);
	double spacing_x = (poly_bounds[1] - poly_bounds[0]) / (dimension[0] - 1);
	double spacing_y = (poly_bounds[3] - poly_bounds[2]) / (dimension[1] - 1);
	double spacing_z = (poly_bounds[5] - poly_bounds[4]) / (dimension[2] - 1);

	for (int i = 0; i < polydata->GetNumberOfPoints(); i++)
	{
		double coords[3];
		polydata->GetPoint(i, coords);
		float * pixel = (float *)ic->GetOutput()->GetScalarPointer(
			(coords[0] - poly_bounds[0]) / spacing_x, (coords[1] - poly_bounds[2]) / spacing_y, (coords[2] - poly_bounds[4]) / spacing_z);
		colors->InsertNextTuple1((*pixel - pixel_range[0]) * 255.0 / gap);
	}

	polydata->GetPointData()->SetScalars(colors);

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputData(polydata);

	vtkSmartPointer<vtkActor>  actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	vtkSmartPointer<vtkRenderer> render = vtkSmartPointer<vtkRenderer>::New();
	render->AddActor(actor);
	render->SetBackground(.0, .0, .0);

	dicom_edge_widget->GetRenderWindow()->AddRenderer(render);
	render->ResetCamera();
	render->Render();
	dicom_edge_widget->GetInteractor()->Start();


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

void DicomSeriesReader::slideMove(int pos)
{
	img_viewer->SetSlice(pos);
	img_viewer->Render();

	edge_viewer->SetSlice(pos);
	edge_viewer->Render();

	slice_cur_label->setText(QString::number(pos, 10));
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

