#pragma once

#include <QVTKWidget.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkimagedata.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkNamedColors.h>
#include <vtkImageAccumulate.h>

class VolumeRenderProcess : public QVTKWidget
{
public:
	explicit VolumeRenderProcess(QVTKWidget *);
	~VolumeRenderProcess();
	void volumeRenderFlow(QString );
	void setBgColor(QColor );

	vtkColorTransferFunction* getVolumeColorTf();
	vtkPiecewiseFunction* getVolumeOpacityTf();

	double getMinGrayValue();
	double getMaxGrayValue();

	void update();

private:
	QVTKWidget* my_vr_widget;
	vtkSmartPointer<vtkDICOMImageReader> dicoms_reader;
	vtkSmartPointer<vtkRenderer> volume_render;

	vtkSmartPointer<vtkVolumeProperty> volumeProperty;
	vtkSmartPointer<vtkColorTransferFunction> volumeColor;
	vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity;
	vtkSmartPointer<vtkVolume> volume;
};