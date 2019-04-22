#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "transferFunction.h"

int color_left_border = 10000, color_right_border = 0, opacity_left_border = 10000, opacity_right_border = 0, gradient_left_border = 10000, gradient_right_border = 0;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	setWindowState(Qt::WindowMaximized);
	ui->colortf_widget->setVisible(false);
	ui->opacitytf_widget->setVisible(false);
	ui->gradienttf_widget->setVisible(false);

	vrProcess = new VolumeRenderProcess(ui->volumeRenderWidget);
	colorTf = new ColorTransferFunction(ui->colortf_widget);
	opacityTf = new OpacityTransferFunctioin(ui->opacitytf_widget, "opacity");
	gradientTf = new OpacityTransferFunctioin(ui->gradienttf_widget, "gradient");

	dicomVisualizer = new DicomVisualizer(ui->dicom_frame, "dicom", ui->series_slider_frame);
	roiVisualizer = new RoiVisualizer(ui->roi_frame, "roi", ui->series_slider_frame);
	boundVisualizer = new BoundVisualizer(ui->bound_frame, "bound", ui->series_slider_frame);

	//color tf widget events
	ui->colortf_bar->installEventFilter(this);
	ui->colortf_curbp_color_label->installEventFilter(this);
	connect(ui->colortf_verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(onShowColorBpInfoAt(int)));

	//scalar opacity tf widget events
	ui->opacitytf_bar->installEventFilter(this);
	ui->opacitytf_curbp_opacity_label->installEventFilter(this);
	connect(ui->opacitytf_verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(onShowOpacityBpInfoAt(int)));

	//gradient opacity tf widget events
	ui->gradienttf_bar->installEventFilter(this);
	ui->gradienttf_curbp_gradient_label->installEventFilter(this);
	connect(ui->gradienttf_verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(onShowGradientBpInfoAt(int)));
	
	////dicom series reader
	ui->dicom_widget->installEventFilter(this);
	//dicom slider
	connect(ui->dicom_series_slider, SIGNAL(valueChanged(int)), this, SLOT(onDicomSeriesSlideMoveSlot(int)));
	//connect(ui->gradient_thresh_slider, SIGNAL(valueChanged(int)), this, SLOT(onGradientThreshSlideMoveSlot(int)));

	//roi gray range slider
	connect(ui->roi_range_slider, SIGNAL(lowerValueChanged(int)), this, SLOT(onRoiGrayMinChangeSlot(int)));
	connect(ui->roi_range_slider, SIGNAL(upperValueChanged(int)), this, SLOT(onRoiGrayMaxChangeSlot(int)));

	connect(ui->bound_extraction_button, SIGNAL(released()), this, SLOT(onRoiToBoundSlot()));

	//bound magnitude slider
	connect(ui->magnitude_thresh_slider, SIGNAL(valueChanged(int)), this, SLOT(onMagThreshChangeSlot(int)));

	//*******************menu****************
	connect(ui->actionOpenFolder, SIGNAL(triggered()), this, SLOT(onOpenFolderSlot()));
	connect(ui->actionBgColor, SIGNAL(triggered()), this, SLOT(onSetBgColorSlot()));

	//set render style
	connect(ui->actionBoneStyle, SIGNAL(triggered()), this, SLOT(onSetBoneStyle()));
	connect(ui->actionBone2Style, SIGNAL(triggered()), this, SLOT(onSetBone2Style()));

	//set mapper
	connect(ui->actionRayCastMapper, SIGNAL(triggered()), this, SLOT(onSetRayCastMapper()));
	connect(ui->actionSmartMapper, SIGNAL(triggered()), this, SLOT(onSetSmartMapper()));

	//bound extraction
	//connect(ui->bound_extraction_button, SIGNAL(released()), this, SLOT(onBoundExtractionButton()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
	//color tf diagram
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
				color_left_border = get<0>(border);
				color_right_border = get<1>(border);
			}
		}
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			//delete the checked color tf bp
			if (keyEvent->key() == Qt::Key_Delete)
			{
				if (colorTf->deleteCurTfBp())
				{
					colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
					vrProcess->update();
				}
			}
			//change current bp gray value through keyboard
			if (keyEvent->key() == Qt::Key_Left)
			{
				colorTf->changeCurBpKeyByKeyboard(-1);
				colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Right)
			{
				colorTf->changeCurBpKeyByKeyboard(1);
				colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
				vrProcess->update();
			}
		}
		if (event->type() == QEvent::MouseMove)
		{//change the position of the current color tf bp
			int pos_x = ui->colortf_bar->mapFromGlobal(QCursor::pos()).x();
			if(pos_x >= color_left_border && pos_x <= color_right_border)
			{
				colorTf->changeCurBpKey(pos_x);
				colorTf->updateVolumeColor(vrProcess->getVolumeColorTf());
				vrProcess->update();
			}
		}
		return true;
	}

	//color tf bp color
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

	//opacity tf diagram
	if (watched == ui->opacitytf_bar)
	{
		int d = opacityTf->getD();
		int x_max = ui->opacitytf_bar->geometry().width() - d;
		int y_max = ui->opacitytf_bar->geometry().height() - d;

		if (event->type() == QEvent::Paint)
		{//draw opacity tf bar
			opacityTf->showTfDiagram();
		}

		if (event->type() == QEvent::MouseButtonPress)
		{//choose or create a opacity tf bp
			QPoint mp = ui->opacitytf_bar->mapFromGlobal(QCursor::pos());
			if (mp.x() > d && mp.x() < x_max && mp.y() > d && mp.y() < y_max)
			{
				opacityTf->chooseOrAddBpAt(mp.x(), mp.y());
				auto border = opacityTf->getCurBpBorder();
				opacity_left_border = get<0>(border);
				opacity_right_border = get<1>(border);
			}
		}
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			//delete the checked opacity tf bp
			if (keyEvent->key() == Qt::Key_Delete)
			{
				if (opacityTf->deleteCurTfBp())
				{
					opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
					vrProcess->update();
				}
			}
			//change current bp gray value through keyboard
			if (keyEvent->key() == Qt::Key_Left)
			{
				opacityTf->changeCurBpKeyByKeyboard(-1);
				opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Right)
			{
				opacityTf->changeCurBpKeyByKeyboard(1);
				opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
				vrProcess->update();
			}
			//change current bp opacity through keyboard
			if (keyEvent->key() == Qt::Key_Down)
			{
				opacityTf->changeCurBpValueByboard(-1);
				opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Up)
			{
				opacityTf->changeCurBpValueByboard(1);
				opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
				vrProcess->update();
			}

		}
		if (event->type() == QEvent::MouseMove)
		{//change the position of the current opacity tf bp
			QPoint m_pos = ui->opacitytf_bar->mapFromGlobal(QCursor::pos());
			int pos_x = m_pos.x();
			int pos_y = m_pos.y();
			if (pos_x >= opacity_left_border && pos_x <= opacity_right_border && pos_y >= d && pos_y <= y_max)
			{
				opacityTf->changeCurBpKey(pos_x);
				opacityTf->changeCurBpValue(pos_y);
				opacityTf->updateVolumeOpacity(vrProcess->getVolumeOpacityTf());
				vrProcess->update();
			}
		}
	}

	//opacity tf bp opacity
	if (watched == ui->opacitytf_curbp_opacity_label)
	{
		if (event->type() == QEvent::Paint)
		{//show the opacity of checked opacity tf bp
			opacityTf->showCurBpValue();
		}
		return true;
	}

	//gradient tf diagram
	if (watched == ui->gradienttf_bar)
	{
		int d = gradientTf->getD();
		int x_max = ui->gradienttf_bar->geometry().width() - d;
		int y_max = ui->gradienttf_bar->geometry().height() - d;

		if (event->type() == QEvent::Paint)
		{//draw gradient tf bar
			gradientTf->showTfDiagram();
		}

		if (event->type() == QEvent::MouseButtonPress)
		{//choose or create a gradient tf bp
			QPoint mp = ui->gradienttf_bar->mapFromGlobal(QCursor::pos());
			if (mp.x() > d && mp.x() < x_max && mp.y() > d && mp.y() < y_max)
			{
				gradientTf->chooseOrAddBpAt(mp.x(), mp.y());
				auto border = gradientTf->getCurBpBorder();
				gradient_left_border = get<0>(border);
				gradient_right_border = get<1>(border);
			}
		}
		if (event->type() == QEvent::KeyPress)
		{
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			//delete the checked gradient tf bp
			if (keyEvent->key() == Qt::Key_Delete)
			{
				if (gradientTf->deleteCurTfBp())
				{
					gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
					vrProcess->update();
				}
			}
			//change current bp gray value through keyboard
			if (keyEvent->key() == Qt::Key_Left)
			{
				gradientTf->changeCurBpKeyByKeyboard(-1);
				gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Right)
			{
				gradientTf->changeCurBpKeyByKeyboard(1);
				gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
				vrProcess->update();
			}
			//change current bp gradient through keyboard
			if (keyEvent->key() == Qt::Key_Down)
			{
				gradientTf->changeCurBpValueByboard(-1);
				gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
				vrProcess->update();
			}
			if (keyEvent->key() == Qt::Key_Up)
			{
				gradientTf->changeCurBpValueByboard(1);
				gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
				vrProcess->update();
			}

		}
		if (event->type() == QEvent::MouseMove)
		{//change the position of the current gradient tf bp
			QPoint m_pos = ui->gradienttf_bar->mapFromGlobal(QCursor::pos());
			int pos_x = m_pos.x();
			int pos_y = m_pos.y();
			if (pos_x >= gradient_left_border && pos_x <= gradient_right_border && pos_y >= d && pos_y <= y_max)
			{
				gradientTf->changeCurBpKey(pos_x);
				gradientTf->changeCurBpValue(pos_y);
				gradientTf->updateVolumeOpacity(vrProcess->getVolumeGradientTf());
				vrProcess->update();
			}
		}
	}

	//gradient tf bp gradient
	if (watched == ui->gradienttf_curbp_gradient_label)
	{
		if (event->type() == QEvent::Paint)
		{//show the gradient of checked gradient tf bp
			gradientTf->showCurBpValue();
		}
		return true;
	}

	//dicom reader widget
	if (watched == ui->dicom_widget)
	{
		if (event->type() == QEvent::MouseButtonRelease)
		{
			QPoint mp = ui->dicom_widget->mapFromGlobal(QCursor::pos());
			dicomVisualizer->showPositionGray(mp.x(), ui->dicom_widget->geometry().height() - mp.y() - 1);
			dicomVisualizer->showPositionMag(boundVisualizer->getPositionMag(mp.x(), ui->dicom_widget->geometry().height() - mp.y() - 1));
			//boundVisualizer->setRoiGrayValue(dicomSeriesReader->getRoiGray());
		}
	}

	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::onShowColorBpInfoAt(int idx)
{
	colorTf->showTfBpInfoAt(idx);
}

void MainWindow::onShowOpacityBpInfoAt(int idx)
{
	opacityTf->showTfBpInfoAt(idx);
}

void MainWindow::onShowGradientBpInfoAt(int idx)
{
	gradientTf->showTfBpInfoAt(idx);
}

void MainWindow::onGradientThreshSlideMoveSlot(int pos)
{
	//dicomSeriesReader->gradientThreshSlideMove(pos);
}

void MainWindow::onBoundExtractionButton()
{
	/*boundVisualizer->findROIBound(
		dicomSeriesReader->getImageGradientData(), 
		dicomSeriesReader->getImageMagnitudeData(), 
		dicomSeriesReader->getImageGrayData(), 
		dicomSeriesReader->getBoundMagnitudePoly());*/
	//dicomSeriesReader->findROIBound();
}

void MainWindow::onRoiGrayMinChangeSlot(int aMin)
{
	if (roiVisualizer->setRoiGrayRange(aMin, roiVisualizer->getRoiRangeMax()))
		roiVisualizer->updateVisualData();
}

void MainWindow::onRoiGrayMaxChangeSlot(int aMax)
{
	if (roiVisualizer->setRoiGrayRange(roiVisualizer->getRoiRangeMin(), aMax))
		roiVisualizer->updateVisualData();
}

void MainWindow::onRoiToBoundSlot()
{
	boundVisualizer->setOriginData(roiVisualizer->getVisualData());
	boundVisualizer->visualizeData();
}

void MainWindow::onMagThreshChangeSlot(int pos)
{
	boundVisualizer->setMagnitudeThresh(pos);
	boundVisualizer->updateVisualData();
	boundVisualizer->setMagSliderValue();
}


void MainWindow::onDicomSeriesSlideMoveSlot(int pos)
{
	//dicomSeriesReader->dicomSeriseSlideMove(pos);
	dicomVisualizer->sliceMove(pos);
	roiVisualizer->sliceMove(pos);
	boundVisualizer->sliceMove(pos);
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

	//********************************************volume render********************************************
	ui->colortf_widget->setVisible(true);
	ui->opacitytf_widget->setVisible(true);

	//build the render pipeline
	vrProcess->volumeRenderFlow(folder_path);

	double max_gv = vrProcess->getMaxGrayValue();
	double min_gv = vrProcess->getMinGrayValue();

	colorTf->setMaxKey(max_gv);
	colorTf->setMinKey(min_gv);
	opacityTf->setMaxKey(max_gv);
	opacityTf->setMinKey(min_gv);

	//set initial render style and draw initial tf
	colorTf->setBoneColorTf(vrProcess->getVolumeColorTf());
	opacityTf->setBoneOpacityTf(vrProcess->getVolumeOpacityTf());
	vrProcess->update();

	//********************************************show dicoms series********************************************
	//dicomSeriesReader->drawDicomSeries(folder_path);
	dicomVisualizer->setOriginData(vrProcess->getDicomReader()->GetOutput());
	dicomVisualizer->visualizeData();

	roiVisualizer->setOriginData(dicomVisualizer->getVisualData());
	roiVisualizer->visualizeData();

	boundVisualizer->setOriginData(roiVisualizer->getVisualData());
	boundVisualizer->visualizeData();
	boundVisualizer->setMagSliderValue();

	/*ui->gradienttf_widget->setVisible(true);
	double max_gradient = dicomSeriesReader->getMaxGradientValue();
	double min_gradient = dicomSeriesReader->getMinGradientValue();

	gradientTf->setMaxKey(max_gradient);
	gradientTf->setMinKey(min_gradient);

	map<double, double> init_gradient_tf;
	init_gradient_tf.insert(pair<double, double>(min_gradient, 1.0));
	init_gradient_tf.insert(pair<double, double>(max_gradient, 1.0));
	gradientTf->setCustomizedOpacityTf(vrProcess->getVolumeGradientTf(), init_gradient_tf); */
	vrProcess->update();

	//********************************************show edge********************************************
	//dicomSeriesReader->cannyEdgeExtraction();
}

void MainWindow::onSetBoneStyle()
{
	colorTf->setBoneColorTf(vrProcess->getVolumeColorTf());
	opacityTf->setBoneOpacityTf(vrProcess->getVolumeOpacityTf());
	vrProcess->update();
}

void MainWindow::onSetBone2Style()
{
	colorTf->setBone2ColorTf(vrProcess->getVolumeColorTf());
	opacityTf->setBone2OpacityTf(vrProcess->getVolumeOpacityTf());
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