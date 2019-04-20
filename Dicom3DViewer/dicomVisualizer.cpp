#include "dicomVisualizer.h"

DicomVisualizer::DicomVisualizer(QFrame * vtk_frame, QString name, QFrame * slider_frame) :
	SeriesVisualizer(vtk_frame, name, slider_frame)
{
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
