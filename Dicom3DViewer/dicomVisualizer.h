#pragma once
#include "seriesVisualizer.h"

class DicomVisualizer : public SeriesVisualizer
{
public:
	DicomVisualizer(QFrame *, QString, QFrame*);
	~DicomVisualizer();

	void transferData();
	double showPositionGray(int x, int y);
	void showPositionMag(QString);

	vtkSmartPointer<vtkImageData> getOriginGrayData();
	vtkSmartPointer<vtkImageData> getOriginMagnitudeData();

private:
	QLabel * dicom_coords_label;
	QLabel * dicom_gray_label;
	QLabel * dicom_mag_label;
};

