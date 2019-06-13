#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "transferFunction.h"

vtkStandardNewMacro(myVtkInteractorStyleImage);
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
	connect(ui->gradient_reset_button, SIGNAL(released()), this, SLOT(onResetGradientTfSlot()));

	////dicom series reader
	ui->dicom_widget->installEventFilter(this);
	//dicom slider
	connect(ui->dicom_series_slider, SIGNAL(valueChanged(int)), this, SLOT(onDicomSeriesSlideMoveSlot(int)));
	//connect(ui->gradient_thresh_slider, SIGNAL(valueChanged(int)), this, SLOT(onGradientThreshSlideMoveSlot(int)));

	//roi gray range slider
	connect(ui->roi_range_slider, SIGNAL(lowerValueChanged(int)), this, SLOT(onRoiGrayMinChangeSlot(int)));
	connect(ui->roi_range_slider, SIGNAL(upperValueChanged(int)), this, SLOT(onRoiGrayMaxChangeSlot(int)));
	
	connect(ui->bound_extraction_button, SIGNAL(released()), this, SLOT(onRoiToBoundSlot()));
	connect(ui->kmeans_button, SIGNAL(released()), this, SLOT(onKMeansSlot()));

	//bound magnitude slider
	connect(ui->magnitude_thresh_slider, SIGNAL(lowerValueChanged(int)), this, SLOT(onMagThreshMinChangeSlot(int)));
	connect(ui->magnitude_thresh_slider, SIGNAL(upperValueChanged(int)), this, SLOT(onMagThreshMaxChangeSlot(int)));

	//roi button
	connect(ui->roi_render_button, SIGNAL(released()), this, SLOT(onRoiRenderSlot()));
	//connect(ui->roi_render_button_2, SIGNAL(released()), this, SLOT(onRoiIncreaseRenderSlot()));
	connect(ui->roiBound_render_button, SIGNAL(released()), this, SLOT(onRoiBoundRenderSlot()));
	//connect(ui->roiBound_render_button_2, SIGNAL(released()), this, SLOT(onRoiBoundIncreaseRenderSlot()));

	//*******************menu****************
	connect(ui->actionOpenDicoms, SIGNAL(triggered()), this, SLOT(onOpenDicomFolderSlot())); 
	connect(ui->actionOpenNII, SIGNAL(triggered()), this, SLOT(onOpenDiiFileSlot()));


	connect(ui->actionCacheVolume, SIGNAL(triggered()), this, SLOT(onCacheVolumeSlot())); 
	connect(ui->actionShowVolumes, SIGNAL(triggered()), this, SLOT(onShowVolumesSlot()));
	connect(ui->actionClearCache, SIGNAL(triggered()), this, SLOT(onClearCacheSlot()));

	connect(ui->actionBgColor, SIGNAL(triggered()), this, SLOT(onSetBgColorSlot()));

	//set render style
	connect(ui->actionBoneStyle, SIGNAL(triggered()), this, SLOT(onSetBoneStyle()));
	connect(ui->actionBone2Style, SIGNAL(triggered()), this, SLOT(onSetBone2Style()));
	connect(ui->actionSkinStyle, SIGNAL(triggered()), this, SLOT(onSetSkinStyle()));
	connect(ui->actionMuscleStyle, SIGNAL(triggered()), this, SLOT(onSetMuscleStyle()));

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
			double gray = dicomVisualizer->showPositionGray(mp.x(), ui->dicom_widget->geometry().height() - mp.y() - 1);
			double mag = boundVisualizer->getPositionMag(mp.x(), ui->dicom_widget->geometry().height() - mp.y() - 1);
			roiVisualizer->setKMeansInitPoint(gray, mag);
			cout << "clicked point, gray: " << gray << "mag: " << mag << endl;
			if (mag == -10000.0)
				dicomVisualizer->showPositionMag("None");
			else
				dicomVisualizer->showPositionMag(QString::number(mag, 10, 2));
		}
	}

	return QMainWindow::eventFilter(watched, event);
}

void MainWindow::onShowColorBpInfoAt(int idx)
{
	ui->colortf_bar->setFocus();
	colorTf->showTfBpInfoAt(idx);
}

void MainWindow::onShowOpacityBpInfoAt(int idx)
{
	ui->opacitytf_bar->setFocus();
	opacityTf->showTfBpInfoAt(idx);
}

void MainWindow::onShowGradientBpInfoAt(int idx)
{
	ui->gradienttf_bar->setFocus();
	gradientTf->showTfBpInfoAt(idx);
}

void MainWindow::onGradientThreshSlideMoveSlot(int pos)
{
}

void MainWindow::onBoundExtractionButton()
{
}

void MainWindow::onRoiGrayMinChangeSlot(int aMin)
{
	ui->roi_minGv_label->setText(QString::number(aMin));
	if (roiVisualizer->setRoiGrayRange(aMin, roiVisualizer->getRoiRangeMax()))
		roiVisualizer->updateVisualData();
}

void MainWindow::onRoiGrayMaxChangeSlot(int aMax)
{
	ui->roi_maxGv_label->setText(QString::number(aMax));
	if (roiVisualizer->setRoiGrayRange(roiVisualizer->getRoiRangeMin(), aMax))
		roiVisualizer->updateVisualData();
}

void MainWindow::onKMeansSlot()
{
	roiVisualizer->kMeansCalc();
}

void MainWindow::onRoiToBoundSlot()
{
	boundVisualizer->setOriginData(roiVisualizer->getTransferedData());
	boundVisualizer->visualizeData();
}

void MainWindow::onMagThreshMinChangeSlot(int aMin)
{
	ui->magnitude_min_label->setText(QString::number(aMin));
	if(boundVisualizer->setMagnitudeRange(aMin, boundVisualizer->getMagnitudeRangeMax()))
		boundVisualizer->updateVisualData();
}

void MainWindow::onMagThreshMaxChangeSlot(int aMax)
{
	ui->magnitude_max_label->setText(QString::number(aMax));
	if (boundVisualizer->setMagnitudeRange(boundVisualizer->getMagnitudeRangeMin(), aMax))
		boundVisualizer->updateVisualData();
}

void MainWindow::onRoiRenderSlot()
{
	onResetGradientTfSlot();
	map<double, double> customized_gray_tf;

	double roi_gv_min = roiVisualizer->getRoiRangeMin();
	double roi_gv_max = roiVisualizer->getRoiRangeMax();
	customized_gray_tf.insert(pair<double, double>(roi_gv_min - 1, 0.0));
	customized_gray_tf.insert(pair<double, double>(roi_gv_min, 1.0));
	customized_gray_tf.insert(pair<double, double>(roi_gv_max, 1.0));
	customized_gray_tf.insert(pair<double, double>(roi_gv_max + 1, 0.0));

	double max_gv = vrProcess->getMaxGrayValue();
	double min_gv = vrProcess->getMinGrayValue();

	if (min_gv != roi_gv_min)
		customized_gray_tf.insert(pair<double, double>(min_gv, 0.0));
	if(max_gv != roi_gv_max)
		customized_gray_tf.insert(pair<double, double>(max_gv, 0.0));

	opacityTf->setCustomizedOpacityTf(vrProcess->getVolumeOpacityTf(), customized_gray_tf);
	vrProcess->update();
}

void MainWindow::onRoiIncreaseRenderSlot()
{
	map<double, double> gv_opacity_tf = opacityTf->getTfBpsMap();

	double roi_gv_min = roiVisualizer->getRoiRangeMin();
	double roi_gv_max = roiVisualizer->getRoiRangeMax();
	gv_opacity_tf.insert(pair<double, double>(roi_gv_min - 1, 0.0));
	gv_opacity_tf.insert(pair<double, double>(roi_gv_min, 1.0));
	gv_opacity_tf.insert(pair<double, double>(roi_gv_max, 1.0));
	gv_opacity_tf.insert(pair<double, double>(roi_gv_max + 1, 0.0));

	double max_gv = vrProcess->getMaxGrayValue();
	double min_gv = vrProcess->getMinGrayValue();

	if (min_gv != roi_gv_min)
		gv_opacity_tf.insert(pair<double, double>(min_gv, 0.0));
	if (max_gv != roi_gv_max)
		gv_opacity_tf.insert(pair<double, double>(max_gv, 0.0));

	opacityTf->setCustomizedOpacityTf(vrProcess->getVolumeOpacityTf(), gv_opacity_tf);
	vrProcess->update();
}

void MainWindow::onRoiBoundRenderSlot()
{
	gradientTf->setCustomizedOpacityTf(vrProcess->getVolumeGradientTf(), boundVisualizer->getRoiBoundMagBp());
	vrProcess->update();
 }

void MainWindow::onRoiBoundIncreaseRenderSlot()
{
	map<double, double> gd_opacity_tf = gradientTf->getTfBpsMap();
	map<double, double> new_gd_opacity_tf = boundVisualizer->getRoiBoundMagBp();

	gd_opacity_tf.insert(new_gd_opacity_tf.begin(), new_gd_opacity_tf.end());
	gradientTf->setCustomizedOpacityTf(vrProcess->getVolumeGradientTf(), gd_opacity_tf);
	vrProcess->update();
}

void MainWindow::onResetGradientTfSlot()
{
	map<double, double> customized_mag_tf;

	double roi_gd_min = boundVisualizer->getMinBoundGradientValue();
	double roi_gd_max = boundVisualizer->getMaxBoundGradientValue();

	customized_mag_tf.insert(pair<double, double>(roi_gd_min, 1.0));
	customized_mag_tf.insert(pair<double, double>(roi_gd_max, 1.0));

	gradientTf->setCustomizedOpacityTf(vrProcess->getVolumeGradientTf(), customized_mag_tf);
	vrProcess->update();
}

void MainWindow::onDicomSeriesSlideMoveSlot(int pos)
{
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

void MainWindow::onOpenDicomFolderSlot()
{
	// get folder path
	QString folder_path = QFileDialog::getExistingDirectory(this, tr("Open DICOM Folder"), 
		"C:\\Users\\13249\\Documents\\VTK_Related\\dataset", QFileDialog::ShowDirsOnly);

	//********************************************volume render********************************************
	ui->colortf_widget->setVisible(true);
	ui->opacitytf_widget->setVisible(true);

	//build the render pipeline
	vrProcess->dicomsVolumeRenderFlow(folder_path);

	double max_gv = vrProcess->getMaxGrayValue();
	double min_gv = vrProcess->getMinGrayValue();

	colorTf->setMaxKey(max_gv);
	colorTf->setMinKey(min_gv);
	opacityTf->setMaxKey(max_gv);
	opacityTf->setMinKey(min_gv);

	//set initial color ang gray-opacity render style and draw initial tf
	colorTf->setBoneColorTf(vrProcess->getVolumeColorTf());
	opacityTf->setBoneOpacityTf(vrProcess->getVolumeOpacityTf());

	vrProcess->update();

	//********************************************show dicoms series********************************************
	dicomVisualizer->setOriginData(vrProcess->getDicomReader()->GetOutput());
	dicomVisualizer->visualizeData();

	roiVisualizer->setOriginData(dicomVisualizer->getTransferedData());
	roiVisualizer->visualizeData();

	boundVisualizer->setOriginData(roiVisualizer->getTransferedData());
	boundVisualizer->transferData();
	boundVisualizer->visualizeData();

	//set initial gradient-opactiy render style  and draw initial tf
	ui->gradienttf_widget->setVisible(true);
	double max_gradient = boundVisualizer->getMaxBoundGradientValue();
	double min_gradient = boundVisualizer->getMinBoundGradientValue();

	gradientTf->setMaxKey(max_gradient);
	gradientTf->setMinKey(min_gradient);

	map<double, double> init_gradient_tf;
	init_gradient_tf.insert(pair<double, double>(min_gradient, 1.0));
	init_gradient_tf.insert(pair<double, double>(max_gradient, 1.0));
	gradientTf->setCustomizedOpacityTf(vrProcess->getVolumeGradientTf(), init_gradient_tf);
	vrProcess->update();

	niiOrDicom = 1;
}


void MainWindow::onOpenDiiFileSlot()
{
	QString filter;
	filter = "NII file (*.nii)";

	QDir dir;
	QString fileName = QFileDialog::getOpenFileName(this, QString(tr("Open NII File")), 
		"C:\\Users\\13249\\Documents\\VTK_Related\\dataset", filter);
	if (fileName.isEmpty()) return;

	//********************************************volume render********************************************
	ui->colortf_widget->setVisible(true);
	ui->opacitytf_widget->setVisible(true);

	//build the render pipeline
	vrProcess->niiVolumeRenderFlow(fileName);

	double max_gv = vrProcess->getMaxGrayValue();
	double min_gv = vrProcess->getMinGrayValue();

	colorTf->setMaxKey(max_gv);
	colorTf->setMinKey(min_gv);
	opacityTf->setMaxKey(max_gv);
	opacityTf->setMinKey(min_gv);

	//set initial color ang gray-opacity render style and draw initial tf
	colorTf->setBoneColorTf(vrProcess->getVolumeColorTf());
	opacityTf->setBoneOpacityTf(vrProcess->getVolumeOpacityTf());

	vrProcess->update();

	//********************************************show dicoms series********************************************
	dicomVisualizer->setOriginData(vrProcess->getNiiReaderOutput());
	dicomVisualizer->visualizeData();

	roiVisualizer->setOriginData(dicomVisualizer->getTransferedData());
	roiVisualizer->visualizeData();

	boundVisualizer->setOriginData(roiVisualizer->getTransferedData());
	boundVisualizer->visualizeData();

	//set initial gradient-opactiy render style  and draw initial tf
	ui->gradienttf_widget->setVisible(true);
	double max_gradient = boundVisualizer->getMaxBoundGradientValue();
	double min_gradient = boundVisualizer->getMinBoundGradientValue();

	gradientTf->setMaxKey(max_gradient);
	gradientTf->setMinKey(min_gradient);

	map<double, double> init_gradient_tf;
	init_gradient_tf.insert(pair<double, double>(min_gradient, 1.0));
	init_gradient_tf.insert(pair<double, double>(max_gradient, 1.0));
	gradientTf->setCustomizedOpacityTf(vrProcess->getVolumeGradientTf(), init_gradient_tf);
	vrProcess->update();

	niiOrDicom = 0;
}

void MainWindow::onCacheVolumeSlot()
{
	vrProcess->addVolume();
}

void MainWindow::onShowVolumesSlot()
{
	vrProcess->showAllVolumes();
}

void MainWindow::onClearCacheSlot()
{
	vrProcess->clearVolumesCache();
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

void MainWindow::onSetSkinStyle()
{
	colorTf->setSkinColorTf(vrProcess->getVolumeColorTf());
	opacityTf->setSkinOpacityTf(vrProcess->getVolumeOpacityTf());
	vrProcess->update();
}

void MainWindow::onSetMuscleStyle()
{
	colorTf->setMuscleColorTf(vrProcess->getVolumeColorTf());
	opacityTf->setMuscleOpacityTf(vrProcess->getVolumeOpacityTf());
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