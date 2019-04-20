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
#include <vtkType.h>
#include <vtkImageCast.h>
#include <vtkThresholdPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkVertexGlyphFilter.h>
#include <vtkPointSet.h>
#include <vtkImageThreshold.h>

#include <QVTKWidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qslider.h>
#include "mySeriesInteractorStyle.h"

#include<vector>

class vtkEventQtSlotConnect;
class vtkRenderer;
using namespace std;

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
	void traverseImageData(vtkImageData *);

	void dicomSeriseSlideMove(int);
	void gradientThreshSlideMove(int);

	float getRoiGray();
	vtkImageData * getImageGradientData();
	vtkImageData * getImageMagnitudeData();
	vtkImageData * getImageGrayData();
	vtkThresholdPoints * getBoundMagnitudePoly();

	void findROIBound();

private:
	vtkSmartPointer<vtkDICOMImageReader> dicoms_reader;
	vtkSmartPointer<vtkImageViewer2> img_viewer; 
	vtkSmartPointer<vtkImageViewer2> edge_viewer;
	vtkSmartPointer<vtkImageGradient> imgGradient;
	vtkSmartPointer<vtkImageMagnitude> imgMagnitude; 
	vtkSmartPointer<vtkImageCast> ic;
	vtkSmartPointer<vtkImageThreshold> max_thresh_img;
	vtkSmartPointer<vtkThresholdPoints> max_thresh_poly;


	float roi_gv;
	float roi_gv_offset;
	int dims[3];	
	vector<float> roi_bound_gv;
	vector<float> roi_bound_gd;

	QLabel * dicom_coords_label;
	QLabel * dicom_gray_label;
	QLabel * dicom_gradient_label;
	QVTKWidget * dicom_reader_widget;
	QVTKWidget * dicom_edge_widget;

	QSlider * dicom_series_slider;
	QLabel * slice_max_label;
	QLabel * slice_min_label;
	QLabel * slice_cur_label;

	double magnitude_range[2];
	QSlider * gradient_thresh_slider;
	QLabel * gradient_min_label;
	QLabel * gradient_max_label;
	QLabel * gradient_cur_label;
	int gradient_thresh;

	int * calcMaxGradientAxisAndOrient(float * gradient, float cur_gv);
	bool isOutOfImage(int * coords);
};

#endif // DICOM_SERIES_READER_H
