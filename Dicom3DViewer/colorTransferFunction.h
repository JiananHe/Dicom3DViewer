#pragma once
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
	void setCurColorBpColor(QColor);

	void drawColorBpsBar();
	void drawCurColorBpColor();
	void showColorTfBpInfoAt(int bar_idx);

	void receiveClickedPosAt(int px);

private:
	ColorTfBreakPoints* my_colortf_bps;
	double min_gray_value;
	double max_gray_value;
	int cur_color_bp_idx;

	int d; //the diameter of cirlces in breakpoints
	int w;
	int h;


	QWidget* my_colortf_widget;
	QFrame* my_colortf_bar;
	QLabel* my_minGV_label;
	QLabel* my_maxGV_label;
	QLabel* my_curBpIdx_label;
	QLabel* my_curBpColor_label;
	QTextEdit* my_curBpX_text;
	QScrollBar* my_colortf_scrollBar;
};