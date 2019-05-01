#pragma once
#include <vtkColorTransferFunction.h>
#include <vtkSmartPointer.h>

#include "volumeRenderProcess.h"
#include "transferFunction.h"
#include "myQColor.h"

using namespace std;

class ColorTransferFunction : public TransferFunction<MyQColor>
{
public:
	explicit ColorTransferFunction(QWidget * );
	~ColorTransferFunction();

	void setBoneColorTf(vtkColorTransferFunction *);
	void setBone2ColorTf(vtkColorTransferFunction *);
	void setSkinColorTf(vtkColorTransferFunction *);
	void setMuscleColorTf(vtkColorTransferFunction *);

	void updateVolumeColor(vtkColorTransferFunction *);

	void showTfDiagram();
	void showCurBpValue();
};