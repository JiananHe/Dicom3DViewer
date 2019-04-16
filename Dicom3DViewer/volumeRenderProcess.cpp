#include "volumeRenderProcess.h"

VolumeRenderProcess::VolumeRenderProcess(QVTKWidget * qvtk_widget)
{
	this->my_vr_widget = qvtk_widget;

	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	volume_render = vtkSmartPointer<vtkRenderer>::New();

	volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
	volumeScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeGradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volume = vtkSmartPointer<vtkVolume>::New();
}

VolumeRenderProcess::~VolumeRenderProcess()
{
}

void VolumeRenderProcess::volumeRenderFlow(QString folder_path)
{
	QByteArray ba = folder_path.toLocal8Bit();
	const char *folderName_str = ba.data();
	cout << "Open floder: " << folderName_str << std::endl;

	//reader
	dicoms_reader->SetDirectoryName(folderName_str);
	dicoms_reader->Update();

	//get images dimension
	int imageDims[3];
	dicoms_reader->GetOutput()->GetDimensions(imageDims);
	cout << "dimension[] :" << imageDims[0] << " " << imageDims[1] << " " << imageDims[2] << endl;

	//gradient
	calcGradientMagnitude();

	//Mapper
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> RcGpuMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	RcGpuMapper->SetInputConnection(dicoms_reader->GetOutputPort());

	//vtkVolumeProperty
	volumeProperty->RemoveAllObservers();
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->ShadeOn();
	volumeProperty->SetAmbient(0.1);
	volumeProperty->SetDiffuse(0.9);
	volumeProperty->SetSpecular(0.2);
	volumeProperty->SetSpecularPower(10.0);
	volumeProperty->SetColor(volumeColor);
	volumeProperty->SetGradientOpacity(volumeGradientOpacity);
	volumeProperty->SetScalarOpacity(volumeScalarOpacity);

	//volume
	volume->RemoveAllObservers();
	volume->SetMapper(RcGpuMapper);
	volume->SetProperty(volumeProperty);

	//render
	volume_render->AddViewProp(volume);
	my_vr_widget->GetRenderWindow()->AddRenderer(volume_render);
	my_vr_widget->GetRenderWindow()->Render();
}

void VolumeRenderProcess::calcGradientMagnitude()
{
	// Calc gradient value
	// Smooth the image
	vtkSmartPointer<vtkImageGaussianSmooth> gs = vtkSmartPointer<vtkImageGaussianSmooth>::New();
	gs->SetInputConnection(dicoms_reader->GetOutputPort());
	gs->SetDimensionality(3);
	gs->SetRadiusFactors(1, 1, 0);

	vtkSmartPointer<vtkImageGradient> imgGradient = vtkSmartPointer<vtkImageGradient>::New();
	imgGradient->SetInputConnection(gs->GetOutputPort());
	imgGradient->SetDimensionality(3);

	imgMagnitude = vtkSmartPointer<vtkImageMagnitude>::New();
	imgMagnitude->SetInputConnection(imgGradient->GetOutputPort());
	imgMagnitude->Update();
}

void VolumeRenderProcess::setBgColor(QColor color)
{
	vtkSmartPointer<vtkNamedColors> bg_color = vtkSmartPointer<vtkNamedColors>::New();
	bg_color->SetColor("bg_color", color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0);
	volume_render->SetBackground(bg_color->GetColor3d("bg_color").GetData());
	my_vr_widget->GetRenderWindow()->Render();
	my_vr_widget->update();
}

vtkColorTransferFunction * VolumeRenderProcess::getVolumeColorTf()
{
	return this->volumeColor;
}

vtkPiecewiseFunction * VolumeRenderProcess::getVolumeOpacityTf()
{
	return this->volumeScalarOpacity;
}

vtkPiecewiseFunction * VolumeRenderProcess::getVolumeGradientTf()
{
	return this->volumeGradientOpacity;
}

vtkDICOMImageReader * VolumeRenderProcess::getDicomReader()
{
	return dicoms_reader;
}

double VolumeRenderProcess::getMinGrayValue()
{
	double range[2];
	dicoms_reader->GetOutput()->GetScalarRange(range);
	return range[0];
}

double VolumeRenderProcess::getMaxGrayValue()
{
	double range[2];
	dicoms_reader->GetOutput()->GetScalarRange(range);
	return range[1];
}

double VolumeRenderProcess::getMinGradientValue()
{
	double range[2];
	imgMagnitude->GetOutput()->GetScalarRange(range);
	return range[0];
}

double VolumeRenderProcess::getMaxGradientValue()
{
	double range[2];
	imgMagnitude->GetOutput()->GetScalarRange(range);
	return range[1];
}

void VolumeRenderProcess::setVRMapper(const char * str_mapper)
{
	if (str_mapper == "ray_cast")
	{
		vtkSmartPointer<vtkGPUVolumeRayCastMapper> RcGpuMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
		RcGpuMapper->SetInputConnection(dicoms_reader->GetOutputPort());

		volume->SetMapper(RcGpuMapper);
		update();
	}
	else if (str_mapper == "smart")
	{
		vtkSmartPointer<vtkSmartVolumeMapper> volumeMapperSmart = vtkSmartPointer<vtkSmartVolumeMapper>::New();
		volumeMapperSmart->SetInputData(dicoms_reader->GetOutput());

		volume->SetMapper(volumeMapperSmart);
		update();
	}
}

void VolumeRenderProcess::saveAsSTL()
{
	/*cout << "save as stl" << endl;
	dicoms_reader->Update();
	vtkSmartPointer<vtkImageDataGeometryFilter> imageDataGeometryFilter = vtkSmartPointer<vtkImageDataGeometryFilter>::New();
	imageDataGeometryFilter->SetInputData(dicoms_reader->GetOutput());
	imageDataGeometryFilter->Update();

	vtkSmartPointer<vtkSTLWriter> stlWriter = vtkSmartPointer<vtkSTLWriter>::New();
	stlWriter->SetFileName("D:\\Code\\Dicom3DViewer\\build\\demo.stl");
	stlWriter->SetInputConnection(dicoms_reader->GetOutputPort());
	stlWriter->Update();
	stlWriter->Write();*/
}

void VolumeRenderProcess::update()
{
	my_vr_widget->GetRenderWindow()->Render();
	my_vr_widget->update();
}

