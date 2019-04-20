#pragma once
#include "seriesVisualizer.h"

class DicomVisualizer : public SeriesVisualizer
{
public:
	DicomVisualizer(QFrame *, QString, QFrame*);
	~DicomVisualizer();

	void transferData();
private:

};

