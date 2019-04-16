#include "dicomSeriesReader.h"

vtkStandardNewMacro(myVtkInteractorStyleImage);

DicomSeriesReader::DicomSeriesReader(QFrame *widget)
{
	img_viewer = vtkSmartPointer<vtkImageViewer2>::New();
	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();

	dicom_reader_widget = widget->findChild<QVTKWidget* >("dicomSlicerWidget");
	dicom_coords_label = widget->findChild<QLabel* >("dicom_coords_label");
	dicom_gray_label = widget->findChild<QLabel* >("dicom_gray_label");
	dicom_gradient_label = widget->findChild<QLabel* >("dicom_gradient_label");
}

DicomSeriesReader::~DicomSeriesReader()
{
}


void DicomSeriesReader::drawDicomSeries(QString folder_path)
{
	// Read all the DICOM files in the specified directory.
	QByteArray ba = folder_path.toLocal8Bit();
	const char *folderName_str = ba.data();

	dicoms_reader->SetDirectoryName(folderName_str);
	dicoms_reader->Update();

	img_viewer->SetInputConnection(dicoms_reader->GetOutputPort());

	dicom_reader_widget->SetRenderWindow(img_viewer->GetRenderWindow());

	img_viewer->SetupInteractor(dicom_reader_widget->GetInteractor());

	// set my interactor style
	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle = vtkSmartPointer<myVtkInteractorStyleImage>::New();
	myInteractorStyle->SetImageViewer(img_viewer);
	dicom_reader_widget->GetInteractor()->SetInteractorStyle(myInteractorStyle);
	dicom_reader_widget->GetInteractor()->Initialize();


	/*img_viewer->SetSliceOrientationToXY();
	img_viewer->GetImageActor()->InterpolateOff();*/

	img_viewer->Render();
	img_viewer->GetRenderer()->ResetCamera();
	img_viewer->Render();
	dicom_reader_widget->GetInteractor()->Start();
}

double DicomSeriesReader::getPositionGv(int x, int y)
{
	//show coords
	dicom_coords_label->setText("X=" + QString::number(x) + " Y=" + QString::number(y) + " Z=" + QString::number(img_viewer->GetSlice()));

	vtkSmartPointer<vtkImageData> image = img_viewer->GetInput();
	vtkSmartPointer<vtkWorldPointPicker> picker = vtkSmartPointer<vtkWorldPointPicker>::New();
	vtkSmartPointer<vtkPointData> pointData = vtkSmartPointer<vtkPointData>::New();
	double pickCoords[3];
	picker->Pick(x, y, 0, img_viewer->GetRenderer());
	picker->GetPickPosition(pickCoords);

	// Fixes some numerical problems with the picking
	double *bounds = img_viewer->GetImageActor()->GetDisplayBounds();
	//std::cout << bounds[4] << std::endl;
	int axis = img_viewer->GetSliceOrientation();
	pickCoords[axis] = bounds[2 * axis];

	vtkPointData* pd = image->GetPointData();
	if (!pd)
	{
		return 0.0;
	}
	
	pointData->InterpolateAllocate(pd, 1, 1);

	// Use tolerance as a function of size of source data
	double tol2 = image->GetLength();
	tol2 = tol2 ? tol2 * tol2 / 1000.0 : 0.001;

	// Find the cell that contains pos
	int subId;
	double pcoords[3], weights[8];
	vtkCell* cell = image->FindAndGetCell(pickCoords, NULL, -1, tol2, subId, pcoords, weights);
	if (cell)
	{
		// Interpolate the point data
		pointData->InterpolatePoint(pd, 0, cell->PointIds, weights);
		int components = pointData->GetScalars()->GetNumberOfComponents();
		double* tuple = pointData->GetScalars()->GetTuple(0);
		if (components == 1)
			dicom_gray_label->setText(QString::number(tuple[0], 10, 2));
		else
			dicom_gray_label->setText("None");
		/*for (int i = 0; i < components; i++)
		{
			cout << tuple[i] << endl;
		}*/
	}
	else
		dicom_gray_label->setText("None");

	return 0.0;
}
