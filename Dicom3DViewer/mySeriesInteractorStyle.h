#pragma once
//
// This example demonstrates how to read a series of dicom images
// and how to scroll with the mousewheel or the up/down keys
// through all slices
//
// some standard vtk headers
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkImageViewer2.h>
#include <vtkDICOMImageReader.h>
#include <vtkInteractorStyleImage.h>
#include <vtkActor2D.h>
#include <vtkTextProperty.h>
#include <vtkTextMapper.h>
#include <sstream>
#include <qslider.h>
#include <qlabel.h>

#include <vtkWorldPointPicker.h>


// Define own interaction style
class myVtkInteractorStyleImage : public vtkInteractorStyleImage
{
public:
	static myVtkInteractorStyleImage* New();
	vtkTypeMacro(myVtkInteractorStyleImage, vtkInteractorStyleImage);

protected:
	vtkImageViewer2* _ImageViewer;
	vtkTextMapper* _StatusMapper;
	vtkRenderWindowInteractor* interactor;
	QSlider* _mySliderSlices;
	QLabel* _myCurSliceLabel;
	int _Slice;
	int _MinSlice;
	int _MaxSlice;

public:
	void SetImageViewer(vtkImageViewer2* imageViewer) {
		_ImageViewer = imageViewer;
		_MinSlice = imageViewer->GetSliceMin();
		_MaxSlice = imageViewer->GetSliceMax();
		_Slice = _MinSlice;
		cout << "Slicer: Min = " << _MinSlice << ", Max = " << _MaxSlice << std::endl;

		interactor = _ImageViewer->GetRenderWindow()->GetInteractor();
	}

	void SetStatusMapper(vtkTextMapper* statusMapper) {
		_StatusMapper = statusMapper;
	}

	void SetSliderSlices(QSlider* sliderSlices, QLabel *label_min_slice, QLabel *label_max_slice, QLabel * label_cur_slice)
	{
		//set slider value
		_mySliderSlices = sliderSlices;
		_myCurSliceLabel = label_cur_slice;
		_mySliderSlices->setMaximum(_MaxSlice);
		_mySliderSlices->setMinimum(_MinSlice);

		//set label value
		label_min_slice->setText(QString::number(_MinSlice, 10));
		label_max_slice->setText(QString::number(_MaxSlice, 10));
		label_cur_slice->setText(QString::number(_MinSlice, 10));
	}

protected:
	void MoveSliceForward() {
		if (_Slice < _MaxSlice) {
			_Slice += 1;

			_myCurSliceLabel->setText(QString::number(_Slice, 10));
			_mySliderSlices->setValue(_Slice);

			_ImageViewer->SetSlice(_Slice);
			_ImageViewer->Render();
		}
	}

	void MoveSliceBackward() {
		if (_Slice > _MinSlice) {
			_Slice -= 1;

			_myCurSliceLabel->setText(QString::number(_Slice, 10));
			_mySliderSlices->setValue(_Slice);

			_ImageViewer->SetSlice(_Slice);
			_ImageViewer->Render();
		}
	}

	virtual void OnKeyDown() 
	{
		std::string key = this->GetInteractor()->GetKeySym();
		if (key.compare("Up") == 0) {
			MoveSliceForward();
		}
		else if (key.compare("Down") == 0) {
			MoveSliceBackward();
		}
		// forward event
		vtkInteractorStyleImage::OnKeyDown();
	}

	
	virtual void OnMouseWheelForward() {
		//std::cout << "Scrolled mouse wheel forward." << std::endl;
		MoveSliceForward();
		// don't forward events, otherwise the image will be zoomed 
		// in case another interactorstyle is used (e.g. trackballstyle, ...)
		// vtkInteractorStyleImage::OnMouseWheelForward();
	}


	virtual void OnMouseWheelBackward() {
		//std::cout << "Scrolled mouse wheel backward." << std::endl;
		if (_Slice > _MinSlice) {
			MoveSliceBackward();
		}
		// don't forward events, otherwise the image will be zoomed 
		// in case another interactorstyle is used (e.g. trackballstyle, ...)
		// vtkInteractorStyleImage::OnMouseWheelBackward();
	}
};