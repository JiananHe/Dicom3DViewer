#pragma once
#include <assert.h>
#include <vtkType.h>
#include <qframe.h>
#include <qslider.h>
#include <qlabel.h>
#include <QVTKWidget.h>

#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <vtkImageThreshold.h>
#include <vtkImageCast.h>
#include <vtkImageViewer2.h>
#include <vtkImageActor.h>
#include <vtkPicker.h>
#include <vtkPointData.h>
#include <vtkCell.h>
#include <vtkDataArray.h>

#include "mySeriesInteractorStyle.h"

class SeriesVisualizer
{
public:
	SeriesVisualizer(QFrame *, QString, QFrame* );
	~SeriesVisualizer();

	void setOriginData(vtkSmartPointer<vtkImageData>);
	void visualizeData();
	void setVisualData(vtkSmartPointer<vtkImageData>);

	virtual void transferData() = 0;

	vtkSmartPointer<vtkImageData> getVisualData();
	vtkSmartPointer<vtkImageData> getOriginData();
	
	void sliceMove(int);

protected:
	QVTKWidget * visual_widget;

	QSlider * dicom_slider;
	QLabel * slider_max_label;
	QLabel * slider_min_label;
	QLabel * slider_cur_label;

private:
	vtkSmartPointer<vtkImageData> origin_data;
	vtkSmartPointer<vtkImageData> visual_data;
protected:
	vtkSmartPointer<vtkImageViewer2> viewer;
};

inline SeriesVisualizer::SeriesVisualizer(QFrame * vtk_frame, QString name, QFrame * slider_frame)
{
	origin_data = vtkSmartPointer<vtkImageData>::New();
	visual_data = vtkSmartPointer<vtkImageData>::New();
	viewer = vtkSmartPointer<vtkImageViewer2>::New();

	visual_widget = vtk_frame->findChild< QVTKWidget *>(name + "_widget");

	dicom_slider = slider_frame->findChild< QSlider *>("dicom_series_slider");
	slider_max_label = slider_frame->findChild< QLabel *>("slider_max_label");
	slider_min_label = slider_frame->findChild< QLabel *>("slider_min_label");
	slider_cur_label = slider_frame->findChild< QLabel *>("slider_cur_label");
}

inline SeriesVisualizer::~SeriesVisualizer()
{
}

inline void SeriesVisualizer::setOriginData(vtkSmartPointer<vtkImageData> input_data)
{
	origin_data = input_data;
}

inline void SeriesVisualizer::visualizeData()
{
	transferData();

	viewer->SetInputData(visual_data);
	visual_widget->SetRenderWindow(viewer->GetRenderWindow());
	viewer->SetupInteractor(visual_widget->GetInteractor());

	vtkSmartPointer<myVtkInteractorStyleImage> myInteractorStyle = vtkSmartPointer<myVtkInteractorStyleImage>::New();
	myInteractorStyle->SetImageViewer(viewer);
	myInteractorStyle->SetSliderSlices(dicom_slider, slider_min_label, slider_max_label, slider_cur_label);
	visual_widget->GetInteractor()->SetInteractorStyle(myInteractorStyle);
	visual_widget->GetInteractor()->Initialize();

	viewer->SetSlice(0);
	viewer->GetRenderer()->ResetCamera();
	viewer->Render();
	visual_widget->GetInteractor()->Start();
}

inline vtkSmartPointer<vtkImageData> SeriesVisualizer::getVisualData()
{
	return visual_data;
}

inline vtkSmartPointer<vtkImageData> SeriesVisualizer::getOriginData()
{
	return origin_data;
}

inline void SeriesVisualizer::sliceMove(int pos)
{
	viewer->SetSlice(pos);
	viewer->Render();
	slider_cur_label->setText(QString::number(pos));
}

inline void SeriesVisualizer::setVisualData(vtkSmartPointer<vtkImageData> input_data)
{
	visual_data = input_data;
}

