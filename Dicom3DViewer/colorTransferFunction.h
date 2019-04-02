#pragma once
#include<tuple>
#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qscrollbar.h>
#include <qtextedit.h>
#include <QPainter>
#include <QMouseEvent>

#include <vtkColorTransferFunction.h>
#include <vtkSmartPointer.h>

#include "volumeRenderProcess.h"
#include "colorTfBreakPoints.h"

using namespace std;

class ColorTransferFunction
{
public:
	explicit ColorTransferFunction(QWidget * );
	~ColorTransferFunction();

	void setBoneColorTf(vtkColorTransferFunction *);
	void setBone2ColorTf(vtkColorTransferFunction *);
	void updateVolumeColor(vtkColorTransferFunction *);

	void setMinGrayValue(double min_gv);
	void setMaxGrayValue(double max_gv);

	QColor getCurColorBpColor();
	tuple<int, int> getCurColorBpBorder();
	void setCurColorBpColor(QColor);
	void setCurColorBpGv(int);

	void drawColorBpsBar();
	void drawCurColorBpColor();
	void showColorTfBpInfoAt(int bar_idx);

	void receiveClickedPosAt(int px);
	void deleteClickedColorBp();

private:
	ColorTfBreakPoints* my_colortf_bps;
	double min_gray_value;
	double max_gray_value;
	int cur_color_bp_idx;

	QWidget* my_colortf_widget;
	QFrame* my_colortf_bar;
	QLabel* my_minGV_label;
	QLabel* my_maxGV_label;
	QLabel* my_curBpIdx_label;
	QLabel* my_curBpColor_label;
	QLabel* my_curBpX_label;
	QScrollBar* my_colortf_scrollBar;

private:
	int d; //the diameter of cirlces in breakpoints
	int w;
	int h;
public:
	int getD();
};