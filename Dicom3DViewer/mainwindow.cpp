#include "mainwindow.h"
#include "ui_mainwindow.h"

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

	connect(ui->actionBoneStyle, SIGNAL(triggered()), this, SLOT(onSetBoneStyle()));
	connect(ui->actionBone2Style, SIGNAL(triggered()), this, SLOT(onSetBone2Style()));

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
		{
			colorTf->drawColorBpsBar();
		}
		else if (event->type() == QEvent::MouseButtonRelease)
		{
			QPoint mp = ui->colortf_bar->mapFromGlobal( QCursor::pos());
			cout << "mx: " << mp.x() << ", my: " << mp.y() << endl;
			colorTf->receiveClickedPosAt(mp.x());
		}
		return true;
	}
	if (watched == ui->colortf_curbp_color_label)
	{
		if (event->type() == QEvent::Paint)
		{
			colorTf->drawCurColorBpColor();
		}
		else if (event->type() == QEvent::MouseButtonRelease)
		{
			QColor init_color = QColor(Qt::blue);
			QColor color = QColorDialog::getColor(init_color, this, "select color");
			if (color.isValid())
			{

			}


		}
		return true;
	}

	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::onShowColorBpInfoAt(int idx)
{
	colorTf->showColorTfBpInfoAt(idx);
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
	colorTf->setMaxGrayValue(vrProcess->getMaxGrayValue());
	colorTf->setMinGrayValue(vrProcess->getMinGrayValue());

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