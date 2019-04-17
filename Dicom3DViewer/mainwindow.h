#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>  
#include <QDir>
#include <qcolordialog.h>

#include "ui_mainwindow.h"
#include "volumeRenderProcess.h"
#include "colorTransferFunction.h"
#include "opacityTransferFunction.h"
#include "dicomSeriesReader.h"

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
	VolumeRenderProcess * vrProcess;
	ColorTransferFunction * colorTf;
	OpacityTransferFunctioin * opacityTf;
	OpacityTransferFunctioin * gradientTf;
	DicomSeriesReader * dicomSeriesReader;
	

private slots:
	void onOpenFolderSlot();
	void onSetBgColorSlot();

	void onSetBoneStyle();
	void onSetBone2Style();

	void onSetSmartMapper();
	void onSetRayCastMapper();

	bool eventFilter(QObject *, QEvent *);
	void onShowColorBpInfoAt(int);
	void onShowOpacityBpInfoAt(int);
	void onShowGradientBpInfoAt(int);

	void onSlideMoveSlot(int);

};

#endif // MAINWINDOW_H
