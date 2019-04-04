#pragma once
#include <qcolor.h>

class MyQColor : public QColor
{
public:
	MyQColor() {};
	MyQColor(QColor color)
	{
		this->setRgb(color.red(), color.green(), color.blue());
	}
	~MyQColor() {};

	MyQColor operator+(const MyQColor& color)
	{
		MyQColor my_color;
		my_color.setRgb(this->red() + color.red(), this->green() + color.green(), this->blue() + color.blue());
		return my_color;
	}

	MyQColor operator-(const MyQColor& color)
	{

		MyQColor my_color;
		my_color.setRgb(this->red() - color.red(), this->green() - color.green(), this->blue() - color.blue());
		return my_color;
	}

	MyQColor operator*(double ratio)
	{
		MyQColor my_color;
		my_color.setRgb(int(this->red() * ratio + 0.5), int(this->green() * ratio + 0.5), int(this->blue() * ratio + 0.5));
		return my_color;
	}
};
