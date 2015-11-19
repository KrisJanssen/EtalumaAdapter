///////////////////////////////////////////////////////////////////////////////
// FILE:          Etaluma.cpp
// PROJECT:       Micro-Manager
// SUBSYSTEM:     DeviceAdapters
//-----------------------------------------------------------------------------
// DESCRIPTION:   Adapter code for the Etaluma 600 and 700 series of
//				  microscopes. This only controls the camera, so a separate
//				  adapter is required for the automated stage for the 700
//				  series.
//                
// AUTHOR:        Nicholas Schaub, nicholas.schaub@nist.gov
//				  http://www.nist.gov/mml/bbd/biomaterials/nicholas-schaub.cfm
//
// LICENSE:       This file is distributed under the BSD license.
//                License text is included with the source distribution.
//
//                This file is distributed in the hope that it will be useful,
//                but WITHOUT ANY WARRANTY; without even the implied warranty
//                of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//                IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//                CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//                INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//

#include "Etaluma.h"
#include "MMDevice.h"
#include "MMDeviceConstants.h"
#include "ModuleInterface.h"
#include <iostream>

using namespace std;

const char* g_CameraName = "LS600";

const char* g_CameraModelProperty = "Model";

const char* g_CameraModel_600 = "Lumascope 600/700";

const char* g_PixelType_8bit = "8bit";

const char* g_PixelClockMHz = "Pixel Clock MHz";


int main() {
	cout << "Initializing LumaUSB...";
	ELumaUSB lumaUSB;
	cout << "[SUCCESSFUL]" << endl;

	cout << "Getting PID_LSCOPE...";
	signed int PID_LSCOPE = lumaUSB.PID_LSCOPE();
	cout << "[SUCCESSFUL] --> " << PID_LSCOPE << endl;
}
// Required function. NJS 2015-11-16
MODULE_API void InitializeModuleData()
{
	RegisterDevice(g_CameraName, MM::CameraDevice, "Etaluma 600/700 Series Camera");
}


MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
	if (deviceName == 0)
		return 0;

	// decide which device class to create based on the deviceName parameter
	if (strcmp(deviceName, g_CameraName) == 0)
	{
		// create camera
		return new Etaluma();
	}

	// ...supplied name not recognized
	return 0;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
	delete pDevice;
}

/********************************************************************************
*						ETALUMA CONSTRUCTOR NJS 2015-11-16						*
* Setup default all variables and create device properties required to exist	*
* before intialization. In this case, the registers and default values are		*
* obtained from LumaUSB.dll.													*
********************************************************************************/
Etaluma::Etaluma() :
	CCameraBase<Etaluma>(),
	binning_(1),
	gain_(0),
	bytesPerPixel_(1),
	initialized_(false),
	exposureMs_(10.0),
	roiX_(0),
	roiY_(0),
	thd_(0),
	IMAGE_HEIGHT(1200),
	IMAGE_WIDTH(1200),
	MAX_BIT_DEPTH(8),
	busy_(false)
{
	// call the base class method to set-up default error codes/messages
	InitializeDefaultErrorMessages();

	// Description property
	int ret = CreateProperty(MM::g_Keyword_Description, "MMCamera example adapter", MM::String, true);
	assert(ret == DEVICE_OK);

	// camera type pre-initialization property
	ret = CreateProperty(g_CameraModelProperty, g_CameraModel_600, MM::String, false, 0, true);
	assert(ret == DEVICE_OK);

	vector<string> modelValues;
	modelValues.push_back(g_CameraModel_600);

	ret = SetAllowedValues(g_CameraModelProperty, modelValues);
	assert(ret == DEVICE_OK);

	// create live video thread
	thd_ = new SequenceThread(this);
}

/********************************************************************************
*						ETALUMA DESTRUCTOR NJS 2015-11-16						*
********************************************************************************/
Etaluma::~Etaluma()
{
	if (initialized_)
		Shutdown();

	delete thd_;
}

// Get the name of the camera. NJS 2015-11-16
void Etaluma::GetName(char* name) const
{
	// We just return the name we use for referring to this
	// device adapter.
	CDeviceUtils::CopyLimitedString(name, g_CameraName);
}

/********************************************************************************
*				INITIALIZE ETALUMA 600/700 CAMERA NJS 2015-11-16				*
*																				*
* This function initializes an Etaluma 600 or 700 series camera. The library	*
* for the camera was created in a C# library, so this function creates an		*
* external camera object (lumaUSB) that can be controlled with this device		*
* adapter. According to the way the device was created, the physical camera		*
* needs to be initialized independent of the current software, so a search for	*
* both initialized and uninitialized devices is performed. The current			*
* implementation should permit multiple Lumascopes to be controlled from a		*
* single computer.																*
********************************************************************************/
int Etaluma::Initialize()
{
	if (initialized_)
		return DEVICE_OK;

	// Constants for the Etaluma microscope.
	PID_LSCOPE = lumaUSB.PID_LSCOPE();
	VID_CYPRESS = lumaUSB.VID_CYPRESS();
	IMAGE_SENSOR_BLUE_GAIN = lumaUSB.IMAGE_SENSOR_BLUE_GAIN();
	IMAGE_SENSOR_GLOBAL_GAIN = lumaUSB.IMAGE_SENSOR_GLOBAL_GAIN();
	IMAGE_SENSOR_GREEN1_GAIN = lumaUSB.IMAGE_SENSOR_GREEN1_GAIN();
	IMAGE_SENSOR_GREEN2_GAIN = lumaUSB.IMAGE_SENSOR_GREEN2_GAIN();
	IMAGE_SENSOR_RED_GAIN = lumaUSB.IMAGE_SENSOR_RED_GAIN();
	IMAGE_SENSOR_RESET = lumaUSB.IMAGE_SENSOR_RESET();
	IMAGE_SENSOR_SHUTTER_WIDTH_LOWER = lumaUSB.IMAGE_SENSOR_SHUTTER_WIDTH_LOWER();
	MAX_GLOBAL_GAIN_PARAMETER_VALUE = lumaUSB.MAX_GLOBAL_GAIN_PARAMETER_VALUE();
	MAX_IMAGE_SENSOR_EXPOSURE = lumaUSB.MAX_IMAGE_SENSOR_EXPOSURE();
	RECOMMENDED_MIN_GLOBAL_GAIN_PARAMETER_VALUE = lumaUSB.RECOMMENDED_MIN_GLOBAL_GAIN_PARAMETER_VALUE();

	// Grab the range of pixel clock frequencies
	for (int i = 0; i < lumaUSB.GetPixelClockDescriptionCount(); i++) {
		clockFreqMHz_.push_back(lumaUSB.GetPixelClockDescription(i));
	}

	// Create the external Lumascope camera object.
	lumaUSB = ELumaUSB(VID_CYPRESS, PID_LSCOPE, IMAGE_WIDTH, IMAGE_HEIGHT);

	// Search for uninitialized cameras first, then search for initialized
	// cameras.
	if (!lumaUSB.findUninitializedCamera()) {
		if (!lumaUSB.findInitializedCamera()) {
			return DEVICE_NOT_CONNECTED;
		}
	}

	//----------------------------------------------//
	// Etaluma adapter property list NJS 2015-11-17 //
	// ---------------------------------------------//

	// GAIN
	CPropertyAction* pAct = new CPropertyAction(this, &Etaluma::OnGain);
	int ret = CreateProperty(MM::g_Keyword_Gain, reinterpret_cast<char*>(&RECOMMENDED_MIN_GLOBAL_GAIN_PARAMETER_VALUE), MM::Integer, false, pAct);
	assert(ret == DEVICE_OK);
	SetPropertyLimits(MM::g_Keyword_Gain, 0, 222);

	// EXPOSURE
	pAct = new CPropertyAction(this, &Etaluma::OnExposure);
	ret = CreateProperty(MM::g_Keyword_Exposure, "10", MM::Float, false, pAct);
	assert(ret == DEVICE_OK);
	SetPropertyLimits(MM::g_Keyword_Exposure, 0, MAX_IMAGE_SENSOR_EXPOSURE);

	// PIXEL CLOCK FREQUENCY
	pAct = new CPropertyAction(this, &Etaluma::OnPixelClock);
	ret = CreateProperty(g_PixelClockMHz,clockFreqMHz_[0].c_str(), MM::String, false, pAct);
	assert(ret == DEVICE_OK);
	ret = SetAllowedValues(g_PixelClockMHz, clockFreqMHz_);
	assert(ret == DEVICE_OK);

	// BINNING - not yet implemented
	/*CPropertyAction *pAct = new CPropertyAction(this, &Etaluma::OnBinning);
	int ret = CreateProperty(MM::g_Keyword_Binning, "1", MM::Integer, false, pAct);
	assert(ret == DEVICE_OK);

	vector<string> binningValues;
	binningValues.push_back("1");
	binningValues.push_back("2");

	ret = SetAllowedValues(MM::g_Keyword_Binning, binningValues);
	assert(ret == DEVICE_OK);*/

	// PIXEL CLOCK FREQUENCY


	//-------------------------------------------//
	// Synchronize all properties NJS 2015-11-17 //
	//-------------------------------------------//
	int ret = UpdateStatus();
	if (ret != DEVICE_OK)
		return ret;

	//---------------------------------//
	// Setup the buffer NJS 2015-11-17 //
	//---------------------------------//
	ret = ResizeImageBuffer();
	if (ret != DEVICE_OK)
		return ret;

	initialized_ = true;
	return DEVICE_OK;
}

int Etaluma::Shutdown() {
	initialized_ = false;
	return DEVICE_OK;
}

/************************************************************************************************
* Performs exposure and grabs a single image. NJS 2015-11-17									*
* This function should block during the actual exposure and return immediately afterwards		*
* (i.e., before readout).  This behavior is needed for proper synchronization with the shutter.	*
* Required by the MM::Camera API.																*
************************************************************************************************/
int Etaluma::SnapImage()
{
	if (IsCapturing() || busy_)
		return DEVICE_CAMERA_BUSY_ACQUIRING;

	busy_ = true;
	
	if (true)
	{

	}

	GenerateImage();
	return DEVICE_OK;
}


const unsigned char* Etaluma::GetImageBuffer()
{
	return const_cast<unsigned char*>(img_.GetPixels());
}

unsigned Etaluma::GetImageWidth() const
{
	return img_.Width();
}

unsigned Etaluma::GetImageHeight() const
{
	return img_.Height();
}

unsigned Etaluma::GetImageBytesPerPixel() const
{
	return img_.Depth();
}

unsigned Etaluma::GetBitDepth() const
{
	return img_.Depth() == 1 ? 8 : MAX_BIT_DEPTH;
}

long Etaluma::GetImageBufferSize() const
{
	return img_.Width() * img_.Height() * GetImageBytesPerPixel();
}

int Etaluma::SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize)
{
	if (xSize == 0 && ySize == 0)
	{
		// effectively clear ROI
		ResizeImageBuffer();
		roiX_ = 0;
		roiY_ = 0;
	}
	else
	{
		// apply ROI
		img_.Resize(xSize, ySize);
		roiX_ = x;
		roiY_ = y;
	}
	return DEVICE_OK;
}

int Etaluma::GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize)
{
	x = roiX_;
	y = roiY_;

	xSize = img_.Width();
	ySize = img_.Height();

	return DEVICE_OK;
}

int Etaluma::ClearROI()
{
	ResizeImageBuffer();
	roiX_ = 0;
	roiY_ = 0;

	return DEVICE_OK;
}

double Etaluma::GetExposure() const
{
	return exposureMs_;
}

void Etaluma::SetExposure(double exp)
{
	exposureMs_ = exp;
}

int Etaluma::GetBinning() const
{
	return binning_;
}

int Etaluma::SetBinning(int binF)
{
	return SetProperty(MM::g_Keyword_Binning, CDeviceUtils::ConvertToString(binF));
}

int Etaluma::PrepareSequenceAcqusition()
{
	if (IsCapturing())
		return DEVICE_CAMERA_BUSY_ACQUIRING;

	int ret = GetCoreCallback()->PrepareForAcq(this);
	if (ret != DEVICE_OK)
		return ret;

	return DEVICE_OK;
}

int Etaluma::StartSequenceAcquisition(double interval) {

	return StartSequenceAcquisition(LONG_MAX, interval, false);
}

int Etaluma::StopSequenceAcquisition()
{
	if (!thd_->IsStopped()) {
		thd_->Stop();
		thd_->wait();
	}

	return DEVICE_OK;
}

int Etaluma::StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow)
{
	if (IsCapturing())
		return DEVICE_CAMERA_BUSY_ACQUIRING;

	int ret = GetCoreCallback()->PrepareForAcq(this);
	if (ret != DEVICE_OK)
		return ret;

	return DEVICE_OK;
}

int Etaluma::InsertImage()
{

	MM::MMTime timeStamp = this->GetCurrentMMTime();
	char label[MM::MaxStrLength];
	this->GetLabel(label);

	// Important:  metadata about the image are generated here:
	Metadata md;

	return DEVICE_OK;
}

bool Etaluma::IsCapturing() {
	return !thd_->IsStopped();
}

// BINNING CALLBACK - not implemented
/*int Etaluma::OnBinning(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		long binSize;
		pProp->Get(binSize);
		binning_ = (int)binSize;
		return ResizeImageBuffer();
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set((long)binning_);
	}

	return DEVICE_OK;
}*/

// Handler for the Gain property for Etaluma adapter. This method constrains the gain values
// to the allowable values.
// NJS 2015-11-17
int Etaluma::OnGain(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		double gain;
		pProp->Get(gain);

		// Check to see if parameter was set within 
		if (gain > pProp->GetUpperLimit()) {
			gain_ = pProp->GetUpperLimit();
		}
		else if (gain < pProp->GetLowerLimit()) {
			gain_ = pProp->GetLowerLimit();
		}
		else {
			gain_ = gain;
		}

		if (!lumaUSB.SetGlobalGain(gain)) {
			return DEVICE_CAN_NOT_SET_PROPERTY;
		}
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(gain_);
	}

	return DEVICE_OK;
}

// Handler for the Exposure property for Etaluma adapter. This method constrains the exposure values
// to the allowable values.
// NJS 2015-11-17

int Etaluma::OnExposure(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		double gain;
		pProp->Get(gain);

		// Check to see if parameter was set within ranage
		if (gain > pProp->GetUpperLimit()) {
			gain_ = pProp->GetUpperLimit();
		}
		else if (gain < pProp->GetLowerLimit()) {
			gain_ = pProp->GetLowerLimit();
		}
		else {
			gain_ = gain;
		}

		if (!lumaUSB.SetGlobalGain(gain)) {
			return DEVICE_CAN_NOT_SET_PROPERTY;
		}
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(gain_);
	}

	return DEVICE_OK;
}

int Etaluma::OnPixelClock(MM::PropertyBase* pProp, MM::ActionType eAct)
{
	if (eAct == MM::AfterSet)
	{
		pProp->Get(currentClockFreqMHz_);

		int freqIndex = currentClockFreqMHz_.find(currentClockFreqMHz_);
		
		if (!lumaUSB.SetImageSensorPixelClockFrequency(freqIndex)) {
			return DEVICE_CAN_NOT_SET_PROPERTY;
		}
	}
	else if (eAct == MM::BeforeGet)
	{
		pProp->Set(currentClockFreqMHz_.c_str());
	}

	return DEVICE_OK;
}

int Etaluma::ResizeImageBuffer()
{
	img_.Resize(IMAGE_WIDTH / binning_, IMAGE_HEIGHT / binning_, bytesPerPixel_);

	return DEVICE_OK;
}

/**
* Generate an image with fixed value for all pixels
*/
void Etaluma::GenerateImage()
{
	const int maxValue = (1 << MAX_BIT_DEPTH) - 1; // max for the 8 bit camera
	const double maxExp = MAX_IMAGE_SENSOR_EXPOSURE;
	double step = maxValue / maxExp;
	unsigned char* pBuf = const_cast<unsigned char*>(img_.GetPixels());
	memset(pBuf, (int)(step * max(exposureMs_, maxExp)), img_.Height()*img_.Width()*img_.Depth());
}
