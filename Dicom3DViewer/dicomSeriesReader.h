#ifndef DICOM_SERIES_READER_H
#define DICOM_SERIES_READER_H

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageViewer2.h>
#include <vtkImageActor.h>  
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkCell.h>
#include <vtkDataArray.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageGradient.h>
#include <vtkImageMagnitude.h>
#include <vtkImageNonMaximumSuppression.h>
#include <vtkImageConstantPad.h>
#include <vtkImageToStructuredPoints.h>
#include <vtkLinkEdgels.h>
#include <vtkThreshold.h>
#include <vtkGeometryFilter.h>
#include <vtkSubPixelPositionEdgels.h>
#include <vtkStripper.h>
#include <vtkImageReslice.h>
#include <vtkMatrix4x4.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>

#include <QVTKWidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qslider.h>
#include "mySeriesInteractorStyle.h"

class vtkEventQtSlotConnect;
class vtkRenderer;

class DicomSeriesReader
{
public:
	explicit DicomSeriesReader(QFrame *);
	~DicomSeriesReader();
	
	void drawDicomSeries(QString folder_path);
	void calcGradientMagnitude();
	double getMinGradientValue();
	double getMaxGradientValue();
	double getPositionGvAndGd(int x, int y);
	void cannyEdgeExtraction();
	void slideMove(int);

private:
	vtkSmartPointer<vtkDICOMImageReader> dicoms_reader;
	vtkSmartPointer<vtkImageViewer2> img_viewer; 
	vtkSmartPointer<vtkImageViewer2> edge_viewer;
	vtkSmartPointer<vtkImageGradient> imgGradient;
	vtkSmartPointer<vtkImageMagnitude> imgMagnitude;


	QLabel * dicom_coords_label;
	QLabel * dicom_gray_label;
	QLabel * dicom_gradient_label;
	QVTKWidget * dicom_reader_widget;
	QVTKWidget * dicom_edge_widget;

	QSlider * dicom_series_slider;
	QLabel * slice_max_label;
	QLabel * slice_min_label;
	QLabel * slice_cur_label;


};

#endif // DICOM_SERIES_READER_H
