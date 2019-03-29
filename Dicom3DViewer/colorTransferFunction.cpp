#include "colorTransferFunction.h"

ColorTransferFunction::ColorTransferFunction(QWidget * color_tf_widget)
{
	my_colortf_widget = color_tf_widget;
	my_colortf_bar = my_colortf_widget->findChild<QFrame *>("colortf_bar");
	my_minGV_label = my_colortf_widget->findChild<QLabel*>("colortf_min_label");
	my_maxGV_label = my_colortf_widget->findChild<QLabel*>("colortf_max_label");
	my_curBpIdx_label = my_colortf_widget->findChild<QLabel*>("colortf_curbp_idx_label");
	my_curBpColor_label = my_colortf_widget->findChild<QLabel*>("colortf_curbp_color_label");
	my_curBpX_text = my_colortf_widget->findChild<QTextEdit*>("colortf_curbp_x_textEdit");
	my_colortf_scrollBar = my_colortf_widget->findChild< QScrollBar*>("colortf_verticalScrollBar");
	
	d = 15;
	w = my_colortf_bar->geometry().width();
	h = my_colortf_bar->geometry().height();

	my_colortf_bps = new ColorTfBreakPoints();

	
}

ColorTransferFunction::~ColorTransferFunction()
{
}

void ColorTransferFunction::setBoneColorTf(vtkColorTransferFunction * volumeColor)
{
	//bone
	int max_point = 3071 > max_gray_value ? max_gray_value : 3071;
	int min_point = -3024 < min_gray_value ? min_gray_value : -3024;

	my_colortf_bps->removeAllPoints();
	my_colortf_bps->insertColorTfBp(min_point, 0, 0, 0);
	my_colortf_bps->insertColorTfBp(-16, 0.73, 0.25, 0.30);
	my_colortf_bps->insertColorTfBp(641, .90, .82, .56);
	my_colortf_bps->insertColorTfBp(max_point, 1, 1, 1);

	updateVolumeColor(volumeColor);

	//show info of the first color bp 
	my_colortf_scrollBar->setValue(0);
	cur_color_bp_idx = 0;
	showColorTfBpInfoAt(0);
}

void ColorTransferFunction::setBone2ColorTf(vtkColorTransferFunction* volumeColor)
{
	//bone2
	int max_point = 3071 > max_gray_value ? max_gray_value : 3071;
	int min_point = -3024 < min_gray_value ? min_gray_value : -3024;

	my_colortf_bps->removeAllPoints();
	my_colortf_bps->insertColorTfBp(min_point, 0.0, 0.0, 0.0);
	my_colortf_bps->insertColorTfBp(143.56, 157 / 255.0, 91 / 255.0, 47 / 255.0);
	my_colortf_bps->insertColorTfBp(166.22, 1.0, 154 / 255.0, 74 / 255.0);
	my_colortf_bps->insertColorTfBp(214.39, 1.0, 1.0, 1.0);
	my_colortf_bps->insertColorTfBp(419.74, 1.0, 239 / 255.0, 244 / 255.0);
	my_colortf_bps->insertColorTfBp(max_point, 211 / 255.0, 168 / 255.0, 1);

	updateVolumeColor(volumeColor);

	//show info of the first color bp 
	my_colortf_scrollBar->setValue(0);
	cur_color_bp_idx = 0;
	showColorTfBpInfoAt(0);
}

void ColorTransferFunction::updateVolumeColor(vtkColorTransferFunction * volumeColor)
{
	volumeColor->RemoveAllPoints();
	map<double, QColor> cur_colortf_bps = my_colortf_bps->getColorBpsMap();
	map<double, QColor>::iterator iter;

	for (iter = cur_colortf_bps.begin(); iter != cur_colortf_bps.end(); ++iter)
	{
		volumeColor->AddRGBPoint(iter->first, (iter->second).red() / 255.0, (iter->second).green() / 255.0, (iter->second).blue() / 255.0);
	}
}

void ColorTransferFunction::setMinGrayValue(double min_gv)
{
	min_gray_value = min_gv;
	my_minGV_label->setText(QString::number(min_gv, 10, 2));
}

void ColorTransferFunction::setMaxGrayValue(double max_gv)
{
	max_gray_value = max_gv;
	my_maxGV_label->setText(QString::number(max_gv, 10, 2));
}

QColor ColorTransferFunction::getCurColorBpColor()
{
	return my_colortf_bps->getColorBpColorAt(cur_color_bp_idx);
}

void ColorTransferFunction::setCurColorBpColor(QColor new_color)
{
	my_colortf_bps->setColorBpColorAt(cur_color_bp_idx, new_color);
	showColorTfBpInfoAt(cur_color_bp_idx);
}

void ColorTransferFunction::drawColorBpsBar()
{
	if (my_colortf_bps->getColorBpsMapLen() == 0)
		return;

	QPainter painter(my_colortf_bar);
	painter.setRenderHint(QPainter::Antialiasing, true);

	//linear gradient color
	map<double, QColor> cur_colortf_bps = my_colortf_bps->getColorBpsMap();
	map<double, QColor>::iterator iter;
	QLinearGradient linearGradient(d, h / 2, w - d, h / 2);
	
	int n = cur_colortf_bps.size(), t = 0;
	double* points = new double[n];

	for (iter = cur_colortf_bps.begin(); iter !=cur_colortf_bps.end(); ++iter)
	{
		double gray_value = iter->first;
		QColor color = iter->second;
		double point = (gray_value - min_gray_value) / (max_gray_value - min_gray_value);
		points[t++] = point;
		linearGradient.setColorAt(point, color);
	}
	painter.setBrush(QBrush(linearGradient));
	painter.drawRect(d, 0, w - 2 * d, h);
	
	//draw color tf breakpoints
	for (int i = 0; i < t; ++i)
	{
		double centre_x = (w - 2 * d)*points[i] + d;
		double centre_y = h / 2;
		QRadialGradient radialGradiant(centre_x, centre_y, d / 2, centre_x + d / 4, centre_y + d / 4);
		radialGradiant.setColorAt(0, Qt::white);
		radialGradiant.setColorAt(1, Qt::darkGray);
		painter.setBrush(radialGradiant);

		//the first and second params is the left top coordiantes of ellipse
		painter.drawEllipse((w - 2 * d)*points[i] + d - d / 2, h / 2 - d / 2, d, d);
	}

	//draw a circle for current color tf bp
	double cur_bp_gv = my_colortf_bps->getColorBpGvAt(cur_color_bp_idx);
	double cur_point = (cur_bp_gv - min_gray_value) / (max_gray_value - min_gray_value);

	int out_cr = 2;
	painter.setPen(QPen(Qt::darkBlue, 2*out_cr));
	painter.setBrush(Qt::NoBrush);
	painter.drawEllipse((w - 2 * d)*cur_point + d - d / 2 - out_cr, h / 2 - d / 2 - out_cr, d + 2 * out_cr, d + 2 * out_cr);
}

void ColorTransferFunction::drawCurColorBpColor()
{
	if (my_colortf_bps->getColorBpsMapLen() == 0)
		return;

	QPainter painter(my_curBpColor_label);
	painter.setRenderHint(QPainter::Antialiasing, true);

	int lw = my_curBpColor_label->geometry().width();
	int lh = my_curBpColor_label->geometry().height();
	int l = 20; 

	QColor idx_color = getCurColorBpColor();
	painter.setBrush(QBrush(QColor(idx_color)));
	painter.drawRect((lw - l) / 2, (lh - l) / 2, l, l);
}

void ColorTransferFunction::showColorTfBpInfoAt(int bar_idx)
{
	int bp_len = my_colortf_bps->getColorBpsMapLen();
	if (bar_idx >= bp_len)
	{
		my_colortf_scrollBar->setValue(bp_len - 1);
		return;
	}
	else
	{
		cur_color_bp_idx = bar_idx % bp_len;
		my_colortf_scrollBar->setValue(cur_color_bp_idx);
		my_curBpIdx_label->setText(QString::number(cur_color_bp_idx));
		my_curBpColor_label->repaint();
		my_curBpX_text->setText(QString::number(my_colortf_bps->getColorBpGvAt(cur_color_bp_idx), 10, 2));
		my_colortf_bar->repaint();
	}
}

void ColorTransferFunction::receiveClickedPosAt(int px)
{
	int rx = px - d;
	double gv_click = (rx / (double)(w - 2 * d)) * (max_gray_value - min_gray_value) + min_gray_value;
	double gv_gap = (d / (2.0 * (w - 2 * d))) * (max_gray_value - min_gray_value);

	int flag = my_colortf_bps->findElementInApprox(gv_click, gv_gap);
	cout << "the flag is: " << flag << endl;
	if (flag == -1)
	{
		//add a new color tf bp
	}
	else
	{
		//choose an existing color tf bp
		cur_color_bp_idx = flag;
		showColorTfBpInfoAt(cur_color_bp_idx);
	}
}