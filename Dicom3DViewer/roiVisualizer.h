#pragma once
#include "seriesVisualizer.h"

class RoiVisualizer : public SeriesVisualizer
{
public:
	RoiVisualizer(QFrame *, QString, QFrame*);
	~RoiVisualizer();

	void transferData();

	void setRoiGrayRange(float, float);
private:
	float roi_min;
	float roi_max;
};
