#include "boundVisulizer.h"

BoundVisualizer::BoundVisualizer(QFrame * frame)
{
	//widget
	bound_grayValue_label = frame->findChild<QTextEdit*>("bound_grayValue_label");
	bound_grayOffset_label = frame->findChild<QTextEdit*>("bound_grayOffset_label");

	roi_gv_offset = 100.0;
	bound_grayOffset_label->setText(QString::number(roi_gv_offset, 10, 2));

	boundMagnitudePoly = vtkSmartPointer<vtkPolyData>::New();
}

BoundVisualizer::~BoundVisualizer()
{
}

void BoundVisualizer::setRoiGrayValue(float roi_gv)
{
	this->roi_gv = roi_gv;
	bound_grayValue_label->setText(QString::number(roi_gv, 10, 2));
}

/*
Purpose: Find the ROI related points among all the edge points
Algorithm:
for ele in edge_points:
	   cur_point = ele;
	1. axis = max(cur_point.gd[3])
	   orient = axis * (roi.gv - ele.gv) > 0 ? -1 : 1;
	2. next_point = get next point according to axis and orient
	
	3. if next_point.gv in rio.range:
		 select ele;
	   else if next_point.gd < ele.gd/2
	     discard ele;
	   else:
	     cur_point = next_point;
		 repeat 1;

*/
void BoundVisualizer::findROIBound(vtkImageData * bgd, vtkImageData * bmd, vtkImageData *pgd, vtkThresholdPoints * bmp)
{
	//get value
	//change scalar to float
	vtkSmartPointer <vtkImageCast> ic = vtkSmartPointer<vtkImageCast>::New();
	ic->SetOutputScalarTypeToFloat();
	ic->SetInputData(bgd);
	ic->Update();
	imageGradientData = ic->GetOutput();

	ic->RemoveAllInputs();
	ic->SetInputData(bmd);
	ic->Update();
	imageMagnitudeData = ic->GetOutput();

	ic->RemoveAllInputs();
	ic->SetInputData(pgd);
	ic->Update();
	imageGrayData = ic->GetOutput();
	

	assert(imageGradientData->GetNumberOfScalarComponents() == 3 && imageMagnitudeData->GetNumberOfScalarComponents() == 1 && imageGrayData->GetNumberOfScalarComponents() == 1);

	int dims2[3], dims3[3];
	imageGradientData->GetDimensions(dims);
	imageMagnitudeData->GetDimensions(dims2);
	imageGrayData->GetDimensions(dims3);
	assert(equal(dims, dims+3, dims2, dims2+3) && equal(dims3, dims3 + 3, dims2, dims2 + 3));

	//calc
	double poly_bounds[6];
	bmp->GetOutput()->GetBounds(poly_bounds);
	double spacing_x = (poly_bounds[1] - poly_bounds[0]) / (dims[0] - 1);
	double spacing_y = (poly_bounds[3] - poly_bounds[2]) / (dims[1] - 1);
	double spacing_z = (poly_bounds[5] - poly_bounds[4]) / (dims[2] - 1);

	float end_mag_ratio = 0.5;

	for (int i = 0; i < bmp->GetOutput()->GetNumberOfPoints(); ++i)
	{
		//map poly coords to data coords
		double poly_coords[3];
		int data_coords[3];
		bmp->GetOutput()->GetPoint(i, poly_coords);
		data_coords[0] = (poly_coords[0] - poly_bounds[0]) / spacing_x;
		data_coords[1] = (poly_coords[1] - poly_bounds[2]) / spacing_x;
		data_coords[2] = (poly_coords[2] - poly_bounds[4]) / spacing_x;

		//get the gradient in three dimentions
		float * ele_gradient = (float *)imageGradientData->GetScalarPointer(data_coords);
		float * ele_magnitude = (float *)imageMagnitudeData->GetScalarPointer(data_coords);
		float * ele_gray = (float *)imageGrayData->GetScalarPointer(data_coords);

		float * cur_gradient = ele_gradient;
		float * cur_magnitude = ele_magnitude;
		float * cur_gray = ele_gray;
		
		while (*cur_magnitude > *ele_magnitude * end_mag_ratio && isOutOfImage(data_coords))
		{
			if (*cur_gray <= roi_gv + roi_gv_offset && *cur_gray >= roi_gv - roi_gv_offset)//the gv of cur point in the roi range, then the ele is a roi bound point.
			{
				roi_bound_gd.push_back(*ele_magnitude);
				roi_bound_gv.push_back(*ele_gray);
				break;
			}
			else//find next point along the max gradient axis and orientation
			{
				int * axis_orient = calcMaxGradientAxisAndOrient(cur_gradient, *cur_gray);
				int axis = axis_orient[0];
				int orient = axis_orient[1];

				data_coords[axis] = data_coords[axis] + orient;
				float * cur_gradient = (float *)imageGradientData->GetScalarPointer(data_coords);
				float * cur_magnitude = (float *)imageMagnitudeData->GetScalarPointer(data_coords);
				float * cur_gray = (float *)imageGrayData->GetScalarPointer(data_coords);
			}
		}
	}

	cout << "roi_bound_gd size: " << roi_bound_gd.size() << endl;
	cout << "roi_bound_gv size: " << roi_bound_gv.size() << endl;
}

int * BoundVisualizer::calcMaxGradientAxisAndOrient(float * gradient, float cur_gv)
{
	int axis = 0;
	int orient = 1;

	if (abs(gradient[0]) >= abs(gradient[1]) && abs(gradient[0]) >= abs(gradient[2]))
		axis = 0;
	else if (abs(gradient[1]) >= abs(gradient[0]) && abs(gradient[1]) >= abs(gradient[2]))
		axis = 1;
	else
		axis = 2;

	if (gradient[axis] > 0)
	{
		if (cur_gv < roi_gv - roi_gv_offset)
			orient = -1;
		else if (cur_gv > roi_gv + roi_gv_offset)
			orient = 1;
	}
	else
	{
		if (cur_gv < roi_gv - roi_gv_offset)
			orient = 1;
		else if (cur_gv > roi_gv + roi_gv_offset)
			orient = -1;
	}

	int res[2];
	res[0] = axis;
	res[1] = orient;

	return res;
}

bool BoundVisualizer::isOutOfImage(int * coords)
{
	if(coords[0] < 0 || coords[0] >= dims[0])
		return false;
	if (coords[1] < 0 || coords[1] >= dims[1])
		return false;
	if (coords[2] < 0 || coords[2] >= dims[2])
		return false;
	return true;
}

