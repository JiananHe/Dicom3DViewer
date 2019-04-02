#pragma once

#include <iostream>
#include <map>
#include <qcolor.h>

using namespace std;

class ColorTfBreakPoints
{
public:
	ColorTfBreakPoints() {};
	void insertColorTfBp(double, double, double, double);
	void insertColorTfBp(double, int, int, int);
	void insertColorTfBp(double);
	void removeAllPoints();
	map<double, QColor> getColorBpsMap();
	int getColorBpsMapLen();
	int findElementInApprox(double, double);

	QColor getColorBpColorAt(int);
	double getColorBpGvAt(int, int);
	void deleteColorBp(int);

	void setColorBpColorAt(int, QColor);

private:
	map<double, QColor> colorBpsMap;
};

//insert a new color bp with specific color
inline void ColorTfBreakPoints::insertColorTfBp(double gv, double r, double g, double b)
{
	int rt = r * 255 + 0.5;
	int gt = g * 255 + 0.5;
	int bt = b * 255 + 0.5;
	colorBpsMap.insert(pair<double, QColor>(gv, QColor(rt, gt, bt)));
}

//insert a new color bp with specific color
inline void ColorTfBreakPoints::insertColorTfBp(double gv, int r, int g, int b)
{
	colorBpsMap.insert(pair<double, QColor>(gv, QColor(r, g, b)));
}

//insert a new color bp with interpolation color
inline void ColorTfBreakPoints::insertColorTfBp(double gv_new)
{
	map<double, QColor>::iterator iter = colorBpsMap.begin();
	double gv_small = iter->first;
	for (; iter != colorBpsMap.end(); ++iter)
	{
		if (gv_new >= iter->first)
			gv_small = iter->first;
		else
			break;
	}
	double gv_big = iter->first;

	//interpolate color
	QColor color_small = colorBpsMap.at(gv_small);
	QColor color_big = colorBpsMap.at(gv_big);
	QColor color_new = QColor((color_small.red() + color_big.red()) / 2, (color_small.green() + color_big.green()) / 2, (color_small.blue() + color_big.blue()) / 2);

	colorBpsMap.insert(pair<double, QColor>(gv_new, color_new));
}

inline void ColorTfBreakPoints::removeAllPoints()
{
	colorBpsMap.clear();
}

inline map<double, QColor> ColorTfBreakPoints::getColorBpsMap()
{
	return this->colorBpsMap;
}

inline int ColorTfBreakPoints::getColorBpsMapLen()
{
	return colorBpsMap.size();
}

//return the idx of the gray value with which gv approximates, otherwise return -1
inline int ColorTfBreakPoints::findElementInApprox(double gv_click, double gv_gap)
{
	map<double, QColor>::iterator iter;
	int f = 0;
	for (iter = colorBpsMap.begin(); iter != colorBpsMap.end(); ++iter, ++f)
	{
		if (iter->first <= gv_click + gv_gap && iter->first >= gv_click - gv_gap)
			return f;
	}
	return -1;
}

inline QColor ColorTfBreakPoints::getColorBpColorAt(int idx)
{
	map<double, QColor>::iterator iter = colorBpsMap.begin();
	while (idx)
	{
		iter++;
		--idx;
	}
	if (iter == colorBpsMap.end())
		abort();

	return iter->second;
}

//return the gray value at idx+flag, flag can only be -1, 0, 1
inline double ColorTfBreakPoints::getColorBpGvAt(int idx, int flag)
{
	map<double, QColor>::iterator iter = colorBpsMap.begin();
	while (idx)
	{
		iter++;
		--idx;
	}
	if (iter == colorBpsMap.end())
		abort();

	if (flag == 0)
		return iter->first;
	else if (flag == -1)
	{
		return iter == colorBpsMap.begin() ? iter->first : (--iter)->first;
	}
	else if (flag == 1)
	{
		double temp = iter->first;
		return (++iter) == colorBpsMap.end() ? temp : iter->first;
	}
	else
		abort();
}

inline void ColorTfBreakPoints::deleteColorBp(int idx)
{
	double gv = getColorBpGvAt(idx, 0);
	colorBpsMap.erase(gv);
}

inline void ColorTfBreakPoints::setColorBpColorAt(int idx, QColor new_color)
{
	double gv = getColorBpGvAt(idx, 0);
	colorBpsMap.erase(gv);
	colorBpsMap.insert(pair<double, QColor>(gv, new_color));
}
