#pragma once
#include<map>
#include <iostream>
using namespace std;

template<typename T>
class BreakPoints
{
public:
	BreakPoints();
	~BreakPoints();

	void insertBreakPoint(double key, T value);
	void insertBreakPoint(double key);
	void removeAllPoints();
	map<double, T> getBreakPointsMap();
	int getMapLength();
	int findElementInApprox(double, double);

	T getBpValueAt(int idx);
	double getBpKeyAt(int idx, int flag);
	void deleteBpAt(int idx);
	void changeBpValueAt(int idx, T new_value);

private:
	map<double, T> breakpoints_map;
};

template<typename T>
BreakPoints<T>::BreakPoints()
{
}

template<typename T>
BreakPoints<T>::~BreakPoints()
{
}

//insert a new color bp with specific color
template<typename T>
inline void BreakPoints<T>::insertBreakPoint(double key, T value)
{
	breakpoints_map.insert(pair<double, T>(key, value));
}

//insert a new color bp with interpolation color
template<typename T>
inline void BreakPoints<T>::insertBreakPoint(double key)
{
	typename map<double, T>::iterator iter = breakpoints_map.begin();
	double key_small = iter->first;
	for (; iter != breakpoints_map.end(); ++iter)
	{
		if (key >= iter->first)
			key_small = iter->first;
		else
			break;
	}
	double key_big = iter->first;

	//interpolate color
	T value_small = breakpoints_map.at(key_small);
	T value_big = breakpoints_map.at(key_big);

	double r = (key - key_small) / (key_big - key_small);

	breakpoints_map.insert(pair<double, T>(key, value_small + (value_big - value_small) * r));
}

template<typename T>
inline void BreakPoints<T>::removeAllPoints()
{
	breakpoints_map.clear();
}

template<typename T>
inline map<double, T> BreakPoints<T>::getBreakPointsMap()
{
	return breakpoints_map;
}

template<typename T>
inline int BreakPoints<T>::getMapLength()
{
	return breakpoints_map.size();
}

//return the idx of the gray value with which gv approximates, otherwise return -1
template<typename T>
inline int BreakPoints<T>::findElementInApprox(double key_to_find, double approx)
{
	map<double, T>::iterator iter;
	int f = 0;
	for (iter = breakpoints_map.begin(); iter != breakpoints_map.end(); ++iter, ++f)
	{
		if (iter->first <= key_to_find + approx && iter->first >= key_to_find - approx)
			return f;
	}
	return -1;
}

template<typename T>
inline T BreakPoints<T>::getBpValueAt(int idx)
{
	map<double, T>::iterator iter = breakpoints_map.begin();
	while (idx)
	{
		iter++;
		--idx;
	}
	if (iter == breakpoints_map.end())
		abort();

	return iter->second;
}

//return the gray value at idx+flag, flag can only be -1, 0, 1
template<typename T>
inline double BreakPoints<T>::getBpKeyAt(int idx, int flag)
{
	map<double, T>::iterator iter = breakpoints_map.begin();
	while (idx)
	{
		iter++;
		--idx;
	}
	if (iter == breakpoints_map.end())
		abort();

	if (flag == 0)
		return iter->first;
	else if (flag == -1)
	{
		return iter == breakpoints_map.begin() ? iter->first : (--iter)->first;
	}
	else if (flag == 1)
	{
		double temp = iter->first;
		return (++iter) == breakpoints_map.end() ? temp : iter->first;
	}
	else
		abort();
}

template<typename T>
inline void BreakPoints<T>::deleteBpAt(int idx)
{
	double key = getBpKeyAt(idx, 0);
	breakpoints_map.erase(key);
}

template<typename T>
inline void BreakPoints<T>::changeBpValueAt(int idx, T new_value)
{
	double key = getBpKeyAt(idx, 0);
	breakpoints_map.erase(key);
	breakpoints_map.insert(pair<double, T>(key, new_value));
}

