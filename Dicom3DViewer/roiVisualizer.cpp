#include "roiVisualizer.h"

RoiVisualizer::RoiVisualizer(QFrame * vtk_frame, QString name, QFrame * slider_frame) :
	SeriesVisualizer(vtk_frame, name, slider_frame)
{
	roi_thresh = vtkSmartPointer<vtkImageThreshold>::New();
	roi_ss = vtkSmartPointer<vtkImageShiftScale>::New();
	roi_range_slider = vtk_frame->findChild<RangeSlider*>("roi_range_slider");
	roi_min_label = vtk_frame->findChild<QLabel*>("roi_min_label");
	roi_max_label = vtk_frame->findChild<QLabel*>("roi_max_label");
}

RoiVisualizer::~RoiVisualizer()
{

}

void RoiVisualizer::setOriginData(vtkSmartPointer<vtkImageData> input_data)
{
	SeriesVisualizer::setOriginData(input_data);

	double init_range[2];
	getOriginData()->GetScalarRange(init_range);
	roi_min_label->setText(QString::number(init_range[0]));
	roi_max_label->setText(QString::number(init_range[1]));
	roi_range_slider->setMinimum(int(init_range[0]));
	roi_range_slider->setMaximum(int(init_range[1]));

	setRoiGrayRange(float(init_range[0]), float(init_range[1]));
}

bool RoiVisualizer::setRoiGrayRange(float rMin, float rMax)
{
	if (rMin == roi_min && rMax == roi_max)
		return false;
	else
	{
		this->roi_min = rMin;
		roi_min_label->setText(QString::number(rMin));
		this->roi_max = rMax;
		roi_max_label->setText(QString::number(rMax)); 
		return true;
	}
}

void RoiVisualizer::updateVisualData()
{
	roi_thresh->ThresholdBetween(roi_min, roi_max);
	roi_thresh->SetReplaceOut(roi_min);
	roi_thresh->Update();

	vtkSmartPointer<vtkImageCast> ic = vtkSmartPointer< vtkImageCast>::New();
	ic->SetInputConnection(roi_thresh->GetOutputPort());
	ic->SetOutputScalarTypeToDouble();
	ic->Update();
	setTransferedData(ic->GetOutput());

	roi_ss->Update();
	setVisualData(roi_ss->GetOutput());

	viewer->SetInputData(getVisualData());
	int slice = viewer->GetSlice();
	viewer->SetSlice(slice);
	viewer->Render();
}

float RoiVisualizer::getRoiRangeMin()
{
	return roi_min;
}

float RoiVisualizer::getRoiRangeMax()
{
	return roi_max;
}

void RoiVisualizer::transferData()
{
	double range[2];
	getOriginData()->GetScalarRange(range);
	cout << "src gray range: " << range[0] << " " << range[1] << endl;

	//thresh the gray data to get roi according to the roi range
	roi_thresh->SetInputData(getOriginData());
	roi_thresh->ThresholdBetween(roi_min, roi_max);
	roi_thresh->ReplaceInOff();
	roi_thresh->ReplaceOutOn();
	roi_thresh->SetReplaceOut(roi_min);
	roi_thresh->Update();

	vtkSmartPointer<vtkImageCast> ic = vtkSmartPointer< vtkImageCast>::New();
	ic->SetInputConnection(roi_thresh->GetOutputPort());
	ic->SetOutputScalarTypeToFloat();
	ic->Update();
	setTransferedData(ic->GetOutput());

	//shift and scale to 0 - 255 to visualize
	roi_ss->SetInputConnection(ic->GetOutputPort());
	roi_ss->SetShift(-roi_min);
	roi_ss->SetScale(255.0 / (roi_max - roi_min));
	roi_ss->Update();
	setVisualData(roi_ss->GetOutput());
}


void RoiVisualizer::setKMeansInitPoint(double gray, double mag)
{
	k_gray = gray;
	k_mag = mag;
}

void RoiVisualizer::kMeansCalc()
{

	vtkSmartPointer<vtkImageData> gray_data = vtkSmartPointer< vtkImageData>::New();
	gray_data = getOriginData();
	double gray_range[2];
	gray_data->GetScalarRange(gray_range);

	int dims[3];
	gray_data->GetDimensions(dims);

	//******************calculate the magnitude ***************************
	vtkSmartPointer<vtkImageGaussianSmooth> gs = vtkSmartPointer<vtkImageGaussianSmooth>::New();
	gs->SetInputData(gray_data);
	gs->SetDimensionality(3);
	gs->SetRadiusFactors(1, 1, 1);

	//gradient with centre difference in three dimentions
	vtkSmartPointer <vtkImageGradient> imgGradient = vtkSmartPointer<vtkImageGradient>::New();
	imgGradient->SetInputConnection(gs->GetOutputPort());
	imgGradient->SetDimensionality(3);
	//imgGradient->Update();

	//gradient magnitude
	vtkSmartPointer<vtkImageMagnitude> imgMagnitude  = vtkSmartPointer<vtkImageMagnitude>::New();
	imgMagnitude->SetInputConnection(imgGradient->GetOutputPort());
	imgMagnitude->Update();
	double mag_range[2];
	imgMagnitude->GetOutput()->GetScalarRange(mag_range);

	//normalize magnitude
	/*vtkSmartPointer<vtkImageShiftScale> normal_mag_data = vtkSmartPointer<vtkImageShiftScale>::New();
	normal_mag_data->SetInputData(imgMagnitude->GetOutput());
	normal_mag_data->SetShift(-mag_range[0]);
	normal_mag_data->SetScale(255.0 / (mag_range[1] - mag_range[0]));
	normal_mag_data->Update();*/

	cout << "magnitude calculation finish." << endl;

	//******************get roi points gray and mag *********************
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkMinimalStandardRandomSequence> sequence = vtkSmartPointer<vtkMinimalStandardRandomSequence>::New();

	//insert initial K-Means centro
	points->InsertNextPoint(k_gray, k_mag, 0.0);
	for (int i = 0; i < 5000; i++)
	{
		int x = sequence->GetValue() * dims[0];
		sequence->Next();
		int y = sequence->GetValue() * dims[1];
		sequence->Next();
		int z = sequence->GetValue() * dims[2];
		sequence->Next();

		/*double poly_coords[3];
		int data_coords[3];
		gray_thresh->GetOutput()->GetPoint(n, poly_coords);
		data_coords[0] = (poly_coords[0] - poly_bounds[0]) / spacing_x;
		data_coords[1] = (poly_coords[1] - poly_bounds[2]) / spacing_y;
		data_coords[2] = (poly_coords[2] - poly_bounds[4]) / spacing_z;*/

		double * ele_gray = (double *)gray_data->GetScalarPointer(x, y, z);
		if (*ele_gray < roi_min || *ele_gray > roi_max)
		{
			--i;
			continue;
		}
		double * ele_mag = (double *)imgMagnitude->GetOutput()->GetScalarPointer(x, y, z);

		//cout << (*ele_gray / 255.0) * (gray_range[1] - gray_range[0]) + gray_range[0] << " " << 
			//(*ele_mag / 255.0) * (mag_range[1] - mag_range[0]) + mag_range[0] << endl;
		//cout << *ele_gray << " " << *ele_mag << endl;

		points->InsertNextPoint(*ele_gray, *ele_mag, 0.0);
	}
	cout << "Got all roi points' gray and mag" << endl;
	

	//************************ K Means ********************************
	// Get the points into the format needed for KMeans
	vtkSmartPointer<vtkTable> inputData = vtkSmartPointer<vtkTable>::New();

	for (int c = 0; c < 3; ++c)
	{
		std::stringstream colName;
		colName << "coord " << c;
		vtkSmartPointer<vtkDoubleArray> doubleArray = vtkSmartPointer<vtkDoubleArray>::New();
		doubleArray->SetNumberOfComponents(1);
		doubleArray->SetName(colName.str().c_str());
		doubleArray->SetNumberOfTuples(points->GetNumberOfPoints());

		for (int r = 0; r < points->GetNumberOfPoints(); ++r)
		{
			double p[3];
			points->GetPoint(r, p);

			doubleArray->SetValue(r, p[c]);
		}

		inputData->AddColumn(doubleArray);
	}
	cout << "transfered k-means input data format." << endl;

	vtkSmartPointer<vtkKMeansDistanceFunctorCalculator> function = vtkSmartPointer<vtkKMeansDistanceFunctorCalculator>::New();
	function->SetDistanceExpression("abs(x0-y0) + abs(x1-y1)");

	vtkSmartPointer<vtkKMeansStatistics> kMeansStatistics = vtkSmartPointer<vtkKMeansStatistics>::New();
	kMeansStatistics->SetDistanceFunctor(function);

	kMeansStatistics->SetInputData(vtkStatisticsAlgorithm::INPUT_DATA, inputData);
	kMeansStatistics->SetColumnStatus(inputData->GetColumnName(0), 1);
	kMeansStatistics->SetColumnStatus(inputData->GetColumnName(1), 1);
	kMeansStatistics->SetColumnStatus(inputData->GetColumnName(2), 1);
	//kMeansStatistics->SetColumnStatus( "Testing", 1 );
	kMeansStatistics->RequestSelectedColumns();
	kMeansStatistics->SetAssessOption(true);
	kMeansStatistics->SetDefaultNumberOfClusters(4);
	kMeansStatistics->Update();

	cout << "K-Means finish." << endl;

	//get cluster centers
	vtkMultiBlockDataSet* outputMetaDS = vtkMultiBlockDataSet::SafeDownCast(kMeansStatistics->GetOutputDataObject(vtkStatisticsAlgorithm::OUTPUT_MODEL));
	vtkSmartPointer<vtkTable> outputMeta = vtkTable::SafeDownCast(outputMetaDS->GetBlock(0));
	//vtkSmartPointer<vtkTable> outputMeta = vtkTable::SafeDownCast( outputMetaDS->GetBlock( 1 ) );
	vtkDoubleArray* coord0 = vtkDoubleArray::SafeDownCast(outputMeta->GetColumnByName("coord 0"));
	vtkDoubleArray* coord1 = vtkDoubleArray::SafeDownCast(outputMeta->GetColumnByName("coord 1"));
	vtkDoubleArray* coord2 = vtkDoubleArray::SafeDownCast(outputMeta->GetColumnByName("coord 2"));

	for (unsigned int i = 0; i < coord0->GetNumberOfTuples(); ++i)
	{
		/*double center_gv = coord0->GetValue(i) / 255.0 * (gray_range[1] - gray_range[0]) + gray_range[0];
		double center_gd = coord1->GetValue(i) / 255.0 * (mag_range[1] - mag_range[0]) + mag_range[0];*/
		cout << coord0->GetValue(i) << " " << coord1->GetValue(i) << " " << coord2->GetValue(i) << std::endl;
	}

	// Display the results
	//kMeansStatistics->GetOutput()->Dump();
	vtkSmartPointer<vtkIntArray> clusterArray = vtkSmartPointer<vtkIntArray>::New();
	clusterArray->SetNumberOfComponents(1);
	clusterArray->SetName("ClusterId");

	cout << "kMeansStatistics rows: " << kMeansStatistics->GetOutput()->GetNumberOfRows() << endl;
	cout << "kMeansStatistics cols: " << kMeansStatistics->GetOutput()->GetNumberOfColumns() << endl;
	for (int r = 0; r < kMeansStatistics->GetOutput()->GetNumberOfRows(); r++)
	{
		vtkVariant v = kMeansStatistics->GetOutput()->GetValue(r, kMeansStatistics->GetOutput()->GetNumberOfColumns() - 1);
		//std::cout << "Point " << r << " is in cluster " << v.ToInt() << std::endl;
		clusterArray->InsertNextValue(v.ToInt());
	}

	// Create a lookup table to map point data to colors
	vtkSmartPointer<vtkLookupTable> lut =
		vtkSmartPointer<vtkLookupTable>::New();
	int tableSize = kMeansStatistics->GetDefaultNumberOfClusters();
	lut->SetNumberOfTableValues(tableSize);
	lut->Build();

	// Fill in the lookup table
	lut->SetTableValue(0, 0.0, 0.0, 0.0, 1.0);
	for (unsigned int i = 1; i < tableSize; ++i)
	{
		lut->SetTableValue(i,
			vtkMath::Random(.25, 1.0),
			vtkMath::Random(.25, 1.0),
			vtkMath::Random(.25, 1.0),
			1.0);
	}

	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
	polydata->SetPoints(points);
	polydata->GetPointData()->SetScalars(clusterArray);

	// Display
	vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
	glyphFilter->SetInputData(polydata);
	glyphFilter->Update();

	// Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> mapper =
		vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(glyphFilter->GetOutputPort());
	mapper->SetScalarRange(0, tableSize - 1);
	mapper->SetLookupTable(lut);

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetPointSize(3);

	// Create a renderer, render window, and interactor
	vtkSmartPointer<vtkRenderer> renderer =
		vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow =
		vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
		vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	// Add the actor to the scene
	renderer->AddActor(actor);
	renderer->SetBackground(1.0, 1.0, 1.0);

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
		vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	renderWindowInteractor->SetInteractorStyle(style);

	// Render and interact
	renderWindow->Render();
	renderWindowInteractor->Start();
}