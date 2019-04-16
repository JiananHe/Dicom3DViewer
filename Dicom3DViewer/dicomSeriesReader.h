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

#include <QVTKWidget.h>
#include <qframe.h>
#include <qlabel.h>
#include "mySeriesInteractorStyle.h"

class vtkEventQtSlotConnect;
class vtkRenderer;

class DicomSeriesReader
{
public:
	explicit DicomSeriesReader(QFrame *);
	~DicomSeriesReader();
	
	void drawDicomSeries(QString folder_path);
	double getPositionGv(int x, int y);
private:
	vtkSmartPointer<vtkImageViewer2> img_viewer;
	vtkSmartPointer<vtkDICOMImageReader> dicoms_reader;

	QLabel * dicom_coords_label;
	QLabel * dicom_gray_label;
	QLabel * dicom_gradient_label;
	QVTKWidget * dicom_reader_widget;

};

#endif // DICOM_SERIES_READER_H
