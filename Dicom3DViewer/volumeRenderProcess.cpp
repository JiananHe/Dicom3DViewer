#include "volumeRenderProcess.h"

VolumeRenderProcess::VolumeRenderProcess(QVTKWidget * qvtk_widget)
{
	this->my_vr_widget = qvtk_widget;

	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	nii_reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	
	volume_render = vtkSmartPointer<vtkRenderer>::New();

	volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
	volumeScalarOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeGradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volume = vtkSmartPointer<vtkVolume>::New();

	multi_volume = vtkSmartPointer<vtkMultiVolume>::New();
	multi_volume_mapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	multi_volume->SetMapper(multi_volume_mapper);

	origin_data = vtkSmartPointer<vtkImageData>::New();

	volume_port = 0;
}

VolumeRenderProcess::~VolumeRenderProcess()
{
}

void VolumeRenderProcess::dicomsVolumeRenderFlow(QString folder_path)
{
	QByteArray ba = folder_path.toLocal8Bit();
	const char *folderName_str = ba.data();

	//reader
	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();
	nii_reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	volume_render = vtkSmartPointer<vtkRenderer>::New();
	dicoms_reader->SetDirectoryName(folderName_str);
	dicoms_reader->Update();
	origin_data = dicoms_reader->GetOutput();

	//get images dimension
	int imageDims[3];
	origin_data->GetDimensions(imageDims);
	cout << "dimension[] :" << imageDims[0] << " " << imageDims[1] << " " << imageDims[2] << endl;
	if (imageDims[0] == 0 || imageDims[1] == 0 || imageDims[2] == 0)
		return;

	//Mapper
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> RcGpuMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	RcGpuMapper->SetInputData(origin_data);

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

void VolumeRenderProcess::niiVolumeRenderFlow(QString file_name)
{
	QByteArray ba = file_name.toLocal8Bit();
	const char *fileName_str = ba.data();

	//reader
	dicoms_reader = vtkSmartPointer<vtkDICOMImageReader>::New();

	nii_reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	nii_reader->SetFileName(fileName_str);
	nii_reader->Update();

	//get images dimension
	int imageDims[3];
	nii_reader->GetOutput()->GetDimensions(imageDims);
	cout << "dimension[] :" << imageDims[0] << " " << imageDims[1] << " " << imageDims[2] << endl;
	if (imageDims[0] == 0 || imageDims[1] == 0 || imageDims[2] == 0)
		return;

	//get range, and scale if necessary
	double range[2];
	nii_reader->GetOutput()->GetScalarRange(range);
	vtkSmartPointer<vtkImageMathematics> m = vtkSmartPointer<vtkImageMathematics>::New();
	if (range[0] == 0 && range[1] == 1)
	{
		cout << "Binary data!!" << endl;
		m->SetInput1Data(nii_reader->GetOutput());
		m->SetConstantK(255);
		m->SetOperationToMultiplyByK();
		m->Update();
		origin_data = m->GetOutput();
	}
	else
		origin_data = nii_reader->GetOutput();

	//Mapper
	vtkSmartPointer<vtkGPUVolumeRayCastMapper> RcGpuMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
	RcGpuMapper->SetInputData(origin_data);

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

	multi_volume->SetVolume(volume, volume_port);
	multi_volume->Update();

	//render
	volume_render->AddVolume(volume);
	my_vr_widget->GetRenderWindow()->AddRenderer(volume_render);
	my_vr_widget->GetRenderWindow()->Render();
}

void VolumeRenderProcess::addNiiVolume()
{
	//add data
	vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
	reader->SetFileName(nii_reader->GetFileName());
	reader->Update();

	double range[2];
	reader->GetOutput()->GetScalarRange(range);
	vtkSmartPointer<vtkImageMathematics> m = vtkSmartPointer<vtkImageMathematics>::New();
	if (range[0] == 0 && range[1] == 1)
	{
		cout << "Binary data!!" << endl;
		m->SetInput1Data(reader->GetOutput());
		m->SetConstantK(255);
		m->SetOperationToMultiplyByK();
		m->Update();
	}

	multi_volume_mapper->SetInputConnection(volume_port, m->GetOutputPort());

	//add volume property
	vtkNew<vtkColorTransferFunction> ctf;
	ctf->DeepCopy(volumeColor);

	vtkNew<vtkPiecewiseFunction> pf;
	pf->DeepCopy(volumeScalarOpacity);

	vtkNew<vtkPiecewiseFunction> gf;
	gf->DeepCopy(volumeGradientOpacity);

	vtkNew<vtkVolume> vol;
	vol->GetProperty()->SetScalarOpacity(pf);
	vol->GetProperty()->SetColor(ctf);
	vol->GetProperty()->SetGradientOpacity(gf);
	vol->GetProperty()->SetInterpolationType(VTK_LINEAR_INTERPOLATION);

	multi_volume->SetVolume(vol, volume_port);
	multi_volume->Update();
	cout << "Cache this volume, its port is " << volume_port << endl;
	volume_port += 2;
}

void VolumeRenderProcess::showAllVolumes()
{
	cout << "Show " << volume_port / 2 << " voulmes" << endl;
	volume_render->AddVolume(multi_volume);
	my_vr_widget->GetRenderWindow()->AddRenderer(volume_render);
	my_vr_widget->GetRenderWindow()->Render();
	my_vr_widget->update();
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

vtkImageData * VolumeRenderProcess::getNiiReaderOutput()
{
	return origin_data;
}

double VolumeRenderProcess::getMinGrayValue()
{
	double range[2];
	origin_data->GetScalarRange(range);
	
	return range[0];
}

double VolumeRenderProcess::getMaxGrayValue()
{
	double range[2];
	origin_data->GetScalarRange(range);

	return range[1];
}

void VolumeRenderProcess::setVRMapper(const char * str_mapper)
{
	if (str_mapper == "ray_cast")
	{
		vtkSmartPointer<vtkGPUVolumeRayCastMapper> RcGpuMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
		if (nii_reader->GetFileName() == NULL)
			RcGpuMapper->SetInputConnection(dicoms_reader->GetOutputPort());
		if (dicoms_reader->GetDirectoryName() == NULL)
			RcGpuMapper->SetInputConnection(nii_reader->GetOutputPort());

		volume->SetMapper(RcGpuMapper);
		update();
	}
	else if (str_mapper == "smart")
	{
		vtkSmartPointer<vtkSmartVolumeMapper> volumeMapperSmart = vtkSmartPointer<vtkSmartVolumeMapper>::New();
		if (nii_reader->GetFileName() == NULL)
			volumeMapperSmart->SetInputConnection(dicoms_reader->GetOutputPort());
		if (dicoms_reader->GetDirectoryName() == NULL)
			volumeMapperSmart->SetInputConnection(nii_reader->GetOutputPort());

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
	volume_render->ResetCamera();
	my_vr_widget->GetRenderWindow()->Render();
	my_vr_widget->update();
}

