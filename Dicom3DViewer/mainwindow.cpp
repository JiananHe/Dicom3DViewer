#include "mainwindow.h"
#include "ui_mainwindow.h"

int left_border = 10000;
int right_border = 0;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	ui->colortf_widget->setVisible(false);

	vrProcess = new VolumeRenderProcess(ui->volumeRenderWidget);
	colorTf = new ColorTransferFunction(ui->colortf_widget);

	ui->colortf_bar->installEventFilter(this);
	ui->colortf_curbp_color_label->installEventFilter(this);
	
	connect(ui->actionOpenFolder, SIGNAL(triggered()), this, SLOT(onOpenFolderSlot()));	
	connect(ui->actionBgColor, SIGNAL(triggered()), this, SLOT(onSetBgColorSlot()));

	//set render style
	connect(ui->actionBoneStyle, SIGNAL(triggered()), this, SLOT(onSetBoneStyle()));
	connect(ui->actionBone2Style, SIGNAL(triggered()), this, SLOT(onSetBone2Style()));

	//set mapper
	connect(ui->actionRayCastMapper, SIGNAL(triggered()), this, SLOT(onSetRayCastMapper()));
	connect(ui->actionSmartMapper, SIGNAL(triggered()), this, SLOT(onSetSmartMapper()));

	//save as stl
	connect(ui->actionSaveAsSTL, SIGNAL(triggered()), this, SLOT(onSaveAsSTL()));
	connect(ui->colortf_verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(onShowColorBpInfoAt(int)));
	
	
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == ui->colortf_bar)
	{
		if (event->type() == QEvent::Paint)
		{//draw color tf bar
			colorTf->showTfDiagram();
		}

		if (event->type() == QEvent::MouseButtonPress)
		{//choose or create a color tf bp
			QPoint mp = ui->colortf_bar->mapFromGlobal( QCursor::pos());
			if (mp.x() > colorTf->getD() && mp.x() < ui->colortf_bar->geometry().width() - colorTf->getD())
			{
				colorTf->chooseOrAddBpAt(mp.x());
				auto border = colorTf->getCurBpBorder();
				left_border = get<0>(border);
				right_border = get<1>(border);
			}
		}
		if (event->type() == QEvent::KeyPress)
		{//delete the checked color tf bp
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if (keyEvent->key() == Qt::Key_Delete)
			{
				colorTf->deleteCurTfBp();
				colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
				vrProcess->update();
			}
		}
		if (event->type() == QEvent::MouseMove)
		{//change the position of the current color tf bp
			int pos_x = ui->colortf_bar->mapFromGlobal(QCursor::pos()).x();
			if(pos_x > left_border && pos_x <right_border)
			{
				colorTf->changeCurBpKey(pos_x);
				colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
				vrProcess->update();
			}
		}
		return true;
	}

	if (watched == ui->colortf_curbp_color_label)
	{
		if (event->type() == QEvent::Paint)
		{//draw the color of checked color tf bp
			colorTf->showCurBpValue();
		}
		else if (event->type() == QEvent::MouseButtonRelease)
		{//change the color of the checked color tf bp
			QColor cur_color = colorTf->getCurBpValue();
			QColor new_color = QColorDialog::getColor(cur_color, this, "select color");
			if (new_color.isValid() && new_color != cur_color)
			{
				colorTf->changeCurBpValue(new_color);
				colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
				vrProcess->update();
			}
		}
		return true;
	}

	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::onShowColorBpInfoAt(int idx)
{
	colorTf->showTfBpInfoAt(idx);
}

void MainWindow::onSaveAsSTL()
{
	vrProcess->saveAsSTL();
}


void MainWindow::onSetBgColorSlot()
{
	QColor init_color = QColor(Qt::blue);
	QColor color = QColorDialog::getColor(init_color, this, "select background color");

	if (color.isValid())
	{
		vrProcess->setBgColor(color);
	}
}

void MainWindow::onOpenFolderSlot()
{
	QString filter;
	filter = "DCM image file (*.dcm)";

	// get folder path
	QString folder_path = QFileDialog::getExistingDirectory(this, tr("Open DICOM Folder"), 
		"C:\\Users\\13249\\Documents\\VTK_Related\\dataset", QFileDialog::ShowDirsOnly);

	ui->colortf_widget->setVisible(true);
	vrProcess->volumeRenderFlow(folder_path);
	colorTf->setMaxKey(vrProcess->getMaxGrayValue());
	colorTf->setMinKey(vrProcess->getMinGrayValue());

	colorTf->setBoneColorTf(vrProcess->getVolumeColorTf());
	vrProcess->update();
}

void MainWindow::onSetBoneStyle()
{
	colorTf->setBoneColorTf(vrProcess->getVolumeColorTf());
	vrProcess->update();
}

void MainWindow::onSetBone2Style()
{
	colorTf->setBone2ColorTf(vrProcess->getVolumeColorTf());
	vrProcess->update();
}

void MainWindow::onSetSmartMapper()
{
	vrProcess->setVRMapper("smart");
}

void MainWindow::onSetRayCastMapper()
{
	vrProcess->setVRMapper("ray_cast");
}
