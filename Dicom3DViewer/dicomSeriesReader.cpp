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

	//show range
	double range[2];
	nonMax->GetOutput()->GetScalarRange(range);
	cout << "nonmax range: " << range[0] << " " << range[1] << endl;
	
	//get imagedata attribute
	int extent[6];
	double spacing[3];
	double origin[3];
	nonMax->GetOutput()->GetOrigin(origin);
	nonMax->GetOutput()->GetExtent(extent);
	nonMax->GetOutput()->GetSpacing(spacing);
	
	edge_viewer->SetInputConnection(nonMax->GetOutputPort());
	dicom_edge_widget->SetRenderWindow(edge_viewer->GetRenderWindow());
	edge_viewer->SetupInteractor(dicom_edge_widget->GetInteractor());


	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle1 = vtkSmartPointer<myVtkInteractorStyleImage>::New();
	myInteractorStyle1->SetImageViewer(edge_viewer);
	myInteractorStyle1->SetSliderSlices(dicom_series_slider, slice_min_label, slice_max_label, slice_cur_label);
	dicom_edge_widget->GetInteractor()->SetInteractorStyle(myInteractorStyle1);
	dicom_edge_widget->GetInteractor()->Initialize();

	edge_viewer->SetSlice(10);
	edge_viewer->GetRenderer()->ResetCamera();
	edge_viewer->Render();
	dicom_edge_widget->GetInteractor()->Start();


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

