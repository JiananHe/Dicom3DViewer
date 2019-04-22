#include "dicomVisualizer.h"

DicomVisualizer::DicomVisualizer(QFrame * vtk_frame, QString name, QFrame * slider_frame) :
	SeriesVisualizer(vtk_frame, name, slider_frame)
{
	dicom_coords_label = vtk_frame->findChild<QLabel*>("dicom_coords_label");
	dicom_gray_label = vtk_frame->findChild<QLabel*>("dicom_gray_label");
	dicom_mag_label = vtk_frame->findChild<QLabel*>("dicom_mag_label");
}

DicomVisualizer::~DicomVisualizer()
{
}

void DicomVisualizer::transferData()
{
	vtkSmartPointer<vtkImageCast> ic = vtkSmartPointer< vtkImageCast>::New();
	ic->SetInputData(getOriginData());
	ic->SetOutputScalarTypeToFloat();
	ic->Update();
    
	setVisualData(ic->GetOutput());
}

void DicomVisualizer::showPositionGray(int x, int y)
{
	//show coords
	dicom_coords_label->setText("X=" + QString::number(x) + " Y=" + QString::number(y) + " Z=" + QString::number(viewer->GetSlice()));

	vtkSmartPointer<vtkImageData> image = viewer->GetInput();
	vtkSmartPointer<vtkPointData> pointData_gv = vtkSmartPointer<vtkPointData>::New();

	vtkSmartPointer<vtkWorldPointPicker> picker = vtkSmartPointer<vtkWorldPointPicker>::New();

	double pickCoords[3];
	picker->Pick(x, y, 0, viewer->GetRenderer());
	picker->GetPickPosition(pickCoords);

	// Fixes some numerical problems with the picking
	double *bounds = viewer->GetImageActor()->GetDisplayBounds();
	int axis = viewer->GetSliceOrientation();
	pickCoords[axis] = bounds[2 * axis];

	vtkPointData* pd_gv = image->GetPointData();
	if (!pd_gv)
	{
		return;
	}

	pointData_gv->InterpolateAllocate(pd_gv, 1, 1);

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
}

void DicomVisualizer::showPositionMag(QString text)
{
	dicom_mag_label->setText(text);
}
