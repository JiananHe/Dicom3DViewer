#pragma once
#include "breakPoints.h"
#include <tuple>
#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qscrollbar.h>
#include <qtextedit.h>
#include <QPainter>
#include <QMouseEvent>

template<typename T>
class TransferFunction
{
public:
	TransferFunction();
	~TransferFunction();

	void setMinKey(double key);
	void setMaxKey(double key);

	T getCurBpValue();
	tuple<int, int> getCurBpBorder();
	void changeCurBpValue(T new_value);
	void changeCurBpKey(int coord);

	void chooseOrAddBpAt(int coord);
	void deleteCurTfBp();

	void showTfBpInfoAt(int idx);
	virtual void showTfDiagram() = 0;
	virtual void showCurBpValue() = 0;

protected:
	BreakPoints<T>* tf_bps;
	double min_key;
	double max_key;
	int cur_bp_idx;

	QWidget* tf_widgets;
	QFrame* tf_diagram;
	QScrollBar* tf_scrollBar;
	QLabel* min_key_label;
	QLabel* max_key_label;
	QLabel* cur_bpIdx_label;
	QLabel* cur_bpKey_label;
	QLabel* cur_bpValue_label;


protected:
	int d; //the diameter of cirlces in breakpoints
	int w;
	int h;
public:
	int getD();
};

template<typename T>
TransferFunction<T>::TransferFunction()
{
	tf_bps = new BreakPoints<T>();
}

template<typename T>
TransferFunction<T>::~TransferFunction()
{
}

template<typename T>
inline void TransferFunction<T>::setMinKey(double key)
{
	min_key = key;
	min_key_label->setText(QString::number(min_key, 10, 2));
}

template<typename T>
inline void TransferFunction<T>::setMaxKey(double key)
{
	max_key = key;
	max_key_label->setText(QString::number(max_key, 10, 2));
}

template<typename T>
inline T TransferFunction<T>::getCurBpValue()
{
	return tf_bps->getBpValueAt(cur_bp_idx);
}

//return the left and right bp coordinates
template<typename T>
inline tuple<int, int> TransferFunction<T>::getCurBpBorder()
{
	double left_key = tf_bps->getBpKeyAt(cur_bp_idx, -1);
	double right_key = tf_bps->getBpKeyAt(cur_bp_idx, 1);

	int left_border = (left_key - min_key) / (max_key - min_key) * (w - 2 * d) + d + 0.5;
	int right_border = (right_key - min_key) / (max_key - min_key) * (w - 2 * d) + d + 0.5;
	
	return make_tuple(left_border, right_border);
}

template<typename T>
inline void TransferFunction<T>::changeCurBpValue(T new_value)
{
	tf_bps->changeBpValueAt(cur_bp_idx, new_value);
	showTfBpInfoAt(cur_bp_idx);
}

//change an exists bp key, used this function when user move a bp
template<typename T>
inline void TransferFunction<T>::changeCurBpKey(int coord)
{
	int tf_coord = coord - d;
	double key_move = (tf_coord / (double)(w - 2 * d)) * (max_key - min_key) + min_key;

	int flag = tf_bps->findElementInApprox(key_move, 0.01);
	if (flag == -1)
	{
		T old_value = tf_bps->getBpValueAt(cur_bp_idx);
		tf_bps->deleteBpAt(cur_bp_idx);
		tf_bps->insertBreakPoint(key_move, old_value);

		showTfBpInfoAt(cur_bp_idx);
	}
}

template<typename T>
inline void TransferFunction<T>::chooseOrAddBpAt(int coord)
{
	int tf_coord = coord - d;
	double key_click = (tf_coord / (double)(w - 2 * d)) * (max_key - min_key) + min_key;
	double key_gap = (d / (2.0 * (w - 2 * d))) * (max_key - min_key);

	int flag = tf_bps->findElementInApprox(key_click, key_gap);
	cout << "the flag is: " << flag << endl;
	if (flag == -1)
	{
		//add a new tf bp
		tf_bps->insertBreakPoint(key_click);
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

template<typename T>
inline void TransferFunction<T>::deleteCurTfBp()
{
	tf_bps->deleteBpAt(cur_bp_idx);
	cur_bp_idx = 0;
	showTfBpInfoAt(cur_bp_idx);
}

template<typename T>
inline void TransferFunction<T>::showTfBpInfoAt(int idx)
{
	int bp_len = tf_bps->getMapLength();
	if (idx >= bp_len)
	{
		tf_scrollBar->setValue(bp_len - 1);
		return;
	}
	else
	{
		cur_bp_idx = idx % bp_len;
		tf_scrollBar->setValue(cur_bp_idx);
		cur_bpIdx_label->setText(QString::number(cur_bp_idx));
		cur_bpValue_label->repaint();
		cur_bpKey_label->setText(QString::number(tf_bps->getBpKeyAt(cur_bp_idx, 0), 10, 2));
		tf_diagram->repaint();
	}

}

template<typename T>
inline int TransferFunction<T>::getD()
{
	return d;
}
