#pragma once
#include <vtkSmartPointer.h>
#include <vtkPiecewiseFunction.h>
#include <qfont.h>
#include "volumeRenderProcess.h"
#include "transferFunction.h"
#include "myQColor.h"

using namespace std;

class OpacityTransferFunctioin : public TransferFunction<double>
{
public:
	explicit OpacityTransferFunctioin(QWidget *, QString);
	~OpacityTransferFunctioin();

	void setBoneOpacityTf(vtkPiecewiseFunction *);
	void setBone2OpacityTf(vtkPiecewiseFunction *);
	void setSkinOpacityTf(vtkPiecewiseFunction *);
	void setMuscleOpacityTf(vtkPiecewiseFunction *);

	void setCustomizedOpacityTf(vtkPiecewiseFunction *, map<double, double>);
	void updateVolumeOpacity(vtkPiecewiseFunction *);



	void showTfDiagram();
	void showCurBpValue();

	//overwrite
	void changeCurBpValue(int y);
	void chooseOrAddBpAt(int x, int y);

	void changeCurBpValueByboard(int flag);

private:
	double opacityToY(double opacity)
	{
		return (2 * d - h) * opacity + (h - d);
	}
	double YToOpacity(int coord)
	{
		return 1.0 - (coord - d) / ((h - 2 * d) * 1.0);
	}
};
