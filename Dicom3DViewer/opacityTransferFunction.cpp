#include "opacityTransferFunction.h"

OpacityTransferFunctioin::OpacityTransferFunctioin(QWidget * widget, QString name)
{
	tf_widgets = widget;
	tf_diagram = tf_widgets->findChild<QFrame *>(name + "tf_bar");
	min_key_label = tf_widgets->findChild<QLabel*>(name + "tf_min_label");
	max_key_label = tf_widgets->findChild<QLabel*>(name + "tf_max_label");
	cur_bpIdx_label = tf_widgets->findChild<QLabel*>(name + "tf_curbp_idx_label");
	cur_bpValue_label = tf_widgets->findChild<QLabel*>(name + "tf_curbp_" + name + "_label");
	cur_bpKey_label = tf_widgets->findChild<QLabel*>(name + "tf_curbp_x_label");
	tf_scrollBar = tf_widgets->findChild< QScrollBar*>(name + "tf_verticalScrollBar");

	d = 11;
	w = tf_diagram->geometry().width();
	h = tf_diagram->geometry().height();
}

OpacityTransferFunctioin::~OpacityTransferFunctioin()
{
}

void OpacityTransferFunctioin::setBoneOpacityTf(vtkPiecewiseFunction * volumeOpacity)
{
	//bone
	int max_point = 3071 > max_key ? max_key : 3071;
	int min_point = -3024 < min_key ? min_key : -3024;

	tf_bps->removeAllPoints();
	tf_bps->insertBreakPoint(min_point, 0.0);
	tf_bps->insertBreakPoint(-16, 0.0);
	tf_bps->insertBreakPoint(641, .72);
	tf_bps->insertBreakPoint(max_point, .71);

	updateVolumeOpacity(volumeOpacity);

	//show info of the first opacity bp
	tf_scrollBar->setValue(0);
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void OpacityTransferFunctioin::setBone2OpacityTf(vtkPiecewiseFunction * volumeOpacity)
{
	//bone2
	int max_point = 3071 > max_key ? max_key : 3071;
	int min_point = -3024 < min_key ? min_key : -3024;

	tf_bps->removeAllPoints();
	tf_bps->insertBreakPoint(min_point, 0.0);
	tf_bps->insertBreakPoint(143.56, 0.0);
	tf_bps->insertBreakPoint(166.22, 0.69);
	tf_bps->insertBreakPoint(214.39, 0.70);
	tf_bps->insertBreakPoint(419.74, 0.83);
	tf_bps->insertBreakPoint(max_point, 0.80);

	updateVolumeOpacity(volumeOpacity);

	//show info of the first opacity bp
	tf_scrollBar->setValue(0);
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void OpacityTransferFunctioin::setCustomizedOpacityTf(vtkPiecewiseFunction * volumeOpacity, map<double, double> my_tf_bps)
{
	tf_bps->removeAllPoints();
	map<double, double>::iterator iter;
	for (iter = my_tf_bps.begin(); iter != my_tf_bps.end(); ++iter)
	{
		this->tf_bps->insertBreakPoint(iter->first, iter->second);
	}
	updateVolumeOpacity(volumeOpacity);

	//show info of the first opacity bp
	tf_scrollBar->setValue(0);
	cur_bp_idx = 0;
	showTfBpInfoAt(0);
}

void OpacityTransferFunctioin::updateVolumeOpacity(vtkPiecewiseFunction * volumeOpacity)
{
	volumeOpacity->RemoveAllPoints();
	map<double, double> cur_tf_bps = tf_bps->getBreakPointsMap();
	map<double, double>::iterator iter;

	for (iter = cur_tf_bps.begin(); iter != cur_tf_bps.end(); ++iter)
	{
		volumeOpacity->AddPoint(iter->first, iter->second, 0.5, 0);
	}

	//volumeOpacity->AddPoint(-16, 0, .49, .61);
}

void OpacityTransferFunctioin::showTfDiagram()
{
	if (tf_bps->getMapLength() == 0)
		return;

	QPainter painter(tf_diagram);
	painter.setRenderHint(QPainter::Antialiasing, true);
	map<double, double> cur_opacitytf_bps = tf_bps->getBreakPointsMap();
	map<double, double>::iterator iter;

	painter.setPen(QPen(Qt::darkGray));
	painter.drawRect(d, d, w - 2 * d, h - 2 * d);
	//draw opacity tf breakpoints
	int n = cur_opacitytf_bps.size(), t = 0;
	double prior_centre_x = .0, prior_centre_y = .0;

	for (iter = cur_opacitytf_bps.begin(); iter != cur_opacitytf_bps.end(); ++iter)
	{
		double gray_value = iter->first;
		double ratio_x = (gray_value - min_key) / (max_key - min_key);
		double centre_x = (w - 2 * d)*ratio_x + d;

		double opacity = iter->second;
		double centre_y = opacityToY(opacity);

		QRadialGradient radialGradiant(centre_x, centre_y, d / 2, centre_x + d / 4, centre_y + d / 4);
		radialGradiant.setColorAt(0, Qt::white);
		radialGradiant.setColorAt(1, Qt::darkGray);
		painter.setBrush(radialGradiant);

		painter.drawEllipse(centre_x - d / 2, centre_y - d / 2, d, d);

		if (iter != cur_opacitytf_bps.begin())
		{
			painter.setPen(QPen(Qt::black));
			painter.drawLine(centre_x, centre_y, prior_centre_x, prior_centre_y);
		}
		prior_centre_x = centre_x;
		prior_centre_y = centre_y;
	}

	//draw a circle for current color tf bp
	double cur_bp_gv = tf_bps->getBpKeyAt(cur_bp_idx, 0);
	double cur_point = (cur_bp_gv - min_key) / (max_key - min_key);
	double cur_opacity = tf_bps->getBpValueAt(cur_bp_idx);

	int out_cr = 1;
	painter.setPen(QPen(Qt::darkBlue, 2 * out_cr));
	painter.setBrush(Qt::NoBrush);
	painter.drawEllipse((w - 2 * d)*cur_point + d - d / 2 - out_cr, opacityToY(cur_opacity) - d / 2 - out_cr, d + 2 * out_cr, d + 2 * out_cr);
}

void OpacityTransferFunctioin::showCurBpValue()
{
	if (tf_bps->getMapLength() == 0)
		return;

	double cur_opacity = tf_bps->getBpValueAt(cur_bp_idx);

	cur_bpValue_label->setText(QString::number(cur_opacity, 10, 2));

	QPainter painter(cur_bpValue_label);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush(Qt::lightGray); 
	painter.setOpacity(cur_opacity);
	int lw = cur_bpValue_label->geometry().width();
	int lh = cur_bpValue_label->geometry().height();
	QRect rect(0, 0, lw, lh);
	painter.drawRect(rect);

	QFont font;
	font.setPointSize(10);
	font.setBold(true);
	painter.setPen(QPen(Qt::darkBlue));
	painter.setFont(font);
	painter.setOpacity(1);
	painter.drawText(rect, Qt::AlignCenter, QString::number(cur_opacity, 10, 2));
}

void OpacityTransferFunctioin::changeCurBpValue(int y)
{
	double opacity = YToOpacity(y);
	TransferFunction::changeCurBpValue(opacity);
}

void OpacityTransferFunctioin::chooseOrAddBpAt(int x, int y)
{
	int tf_x = x - d;
	double key_click = (tf_x / (double)(w - 2 * d)) * (max_key - min_key) + min_key;
	double value_click = YToOpacity(y);
	double key_gap = (d / (2.0 * (w - 2 * d))) * (max_key - min_key);

	int flag = tf_bps->findElementInApprox(key_click, key_gap);
	cout << "the flag is: " << flag << endl;
	if (flag == -1)
	{
		//add a new tf bp
		tf_bps->insertBreakPoint(key_click, value_click);
		cur_bp_idx = tf_bps->findElementInApprox(key_click, 0.0);
		showTfBpInfoAt(cur_bp_idx);
	}
	else
	{
		//choose an existing color tf bp
		cur_bp_idx = flag;
		showTfBpInfoAt(cur_bp_idx);
	}
}

//change currrent opacity bp opacity according to falg: downward(-1), upward(1)
void OpacityTransferFunctioin::changeCurBpValueByboard(int flag)
{
	double cur_key = tf_bps->getBpKeyAt(cur_bp_idx, 0);
	double cur_value = tf_bps->getBpValueAt(cur_bp_idx);
	double move_gap = 0.1;
	if (flag == -1)
	{
		tf_bps->deleteBpAt(cur_bp_idx);
		tf_bps->insertBreakPoint(cur_key, cur_value - move_gap < 0 ? 0 : cur_value - move_gap);
		showTfBpInfoAt(cur_bp_idx);
	}
	if (flag == 1)
	{
		tf_bps->deleteBpAt(cur_bp_idx);
		tf_bps->insertBreakPoint(cur_key, cur_value + move_gap > 1 ? 1 : cur_value + move_gap);
		showTfBpInfoAt(cur_bp_idx);
	}
}
