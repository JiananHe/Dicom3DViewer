#pragma once

#include <QVTKWidget.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkNIFTIImageReader.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkimagedata.h>
#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkMultiVolume.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkNamedColors.h>
#include <vtkImageAccumulate.h>
#include <vtkVolumeMapper.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkSTLWriter.h>
#include <vtkImageDataGeometryFilter.h>

class VolumeRenderProcess : public QVTKWidget
{
public:
	explicit VolumeRenderProcess(QVTKWidget *);
	~VolumeRenderProcess();
	void dicomsVolumeRenderFlow(QString );
	void niiVolumeRenderFlow(QString );
	void addVolumeRenderFlow(QString );
	void setBgColor(QColor );

	vtkColorTransferFunction* getVolumeColorTf();
	vtkPiecewiseFunction* getVolumeOpacityTf();
	vtkPiecewiseFunction* getVolumeGradientTf();

	vtkDICOMImageReader* getDicomReader();
	vtkNIFTIImageReader* getNiiReader();

	double getMinGrayValue();
	double getMaxGrayValue();

	void setVRMapper(const char *);

	void saveAsSTL();

	void update();

private:
	QVTKWidget* my_vr_widget;
	vtkSmartPointer<vtkDICOMImageReader> dicoms_reader;
	vtkSmartPointer< vtkNIFTIImageReader> nii_reader;
	vtkSmartPointer< vtkNIFTIImageReader> nii_reader1;
	vtkSmartPointer< vtkNIFTIImageReader> nii_reader2;
	vtkSmartPointer< vtkNIFTIImageReader> nii_reader3;
	vtkSmartPointer<vtkRenderer> volume_render;

	vtkSmartPointer<vtkMultiVolume> multi_volume;
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> multi_volume_mapper;

	vtkSmartPointer<vtkVolumeProperty> volumeProperty;
	vtkSmartPointer<vtkColorTransferFunction> volumeColor;
	vtkSmartPointer<vtkPiecewiseFunction> volumeScalarOpacity;
	vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity;
	vtkSmartPointer<vtkVolume> volume;

	int volume_port;
};