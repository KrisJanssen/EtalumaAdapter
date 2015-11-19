///////////////////////////////////////////////////////////////////////////////
// FILE:          Etaluma.h
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

#ifndef _ETALUMA_H_
#define _ETALUMA_H_

#include "DeviceBase.h"
#include "ImgBuffer.h"
#include "DeviceThreads.h"
#include "ELumaUSB.h"

//////////////////////////////////////////////////////////////////////////////
// Error codes
//
#define ERR_UNKNOWN_MODE         102

class SequenceThread;

class Etaluma : public CCameraBase<Etaluma>
{
public:
	Etaluma();
	~Etaluma();

	// MMDevice API
	// ------------
	int Initialize();
	int Shutdown();

	void GetName(char* name) const;

	// MMCamera API
	// ------------
	int SnapImage();
	const unsigned char* GetImageBuffer();
	unsigned GetImageWidth() const;
	unsigned GetImageHeight() const;
	unsigned GetImageBytesPerPixel() const;
	unsigned GetBitDepth() const;
	long GetImageBufferSize() const;
	double GetExposure() const;
	void SetExposure(double exp);
	int SetROI(unsigned x, unsigned y, unsigned xSize, unsigned ySize);
	int GetROI(unsigned& x, unsigned& y, unsigned& xSize, unsigned& ySize);
	int ClearROI();
	int PrepareSequenceAcqusition();
	int StartSequenceAcquisition(double interval);
	int StartSequenceAcquisition(long numImages, double interval_ms, bool stopOnOverflow);
	int StopSequenceAcquisition();
	bool IsCapturing();
	int GetBinning() const;
	int SetBinning(int binSize);
	int IsExposureSequenceable(bool& seq) const { seq = false; return DEVICE_OK; }

	// action interface
	// ----------------
	//int OnBinning(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnGain(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnExposure(MM::PropertyBase* pProp, MM::ActionType eAct);
	int OnPixelClock(MM::PropertyBase* pProp, MM::ActionType eAct);

private:
	friend class SequenceThread;
	int IMAGE_WIDTH;
	int IMAGE_HEIGHT;
	int MAX_BIT_DEPTH;

	SequenceThread* thd_;
	int binning_;
	int bytesPerPixel_;
	double gain_;
	double exposureMs_;
	vector<string> clockFreqMHz_;
	string currentClockFreqMHz_;
	bool initialized_;
	ImgBuffer img_;
	int roiX_, roiY_;
	bool busy_;

	int ResizeImageBuffer();
	void GenerateImage();
	int InsertImage();

	ELumaUSB lumaUSB;
	signed int PID_FX2_DEV;
	signed int PID_LSCOPE;
	signed int VID_CYPRESS;
	unsigned char IMAGE_SENSOR_BLUE_GAIN;
	unsigned char IMAGE_SENSOR_GLOBAL_GAIN;
	unsigned char IMAGE_SENSOR_GREEN1_GAIN;
	unsigned char IMAGE_SENSOR_GREEN2_GAIN;
	unsigned char IMAGE_SENSOR_RED_GAIN;
	unsigned char IMAGE_SENSOR_RESET;
	unsigned char IMAGE_SENSOR_SHUTTER_WIDTH_LOWER;
	unsigned char MAX_GLOBAL_GAIN_PARAMETER_VALUE;
	signed int MAX_IMAGE_SENSOR_EXPOSURE;
	signed int RECOMMENDED_MIN_GLOBAL_GAIN_PARAMETER_VALUE;
};

class SequenceThread : public MMDeviceThreadBase
{
public:
	SequenceThread(Etaluma* pCam);
	~SequenceThread();
	void Stop();
	void Start(long numImages, double intervalMs);
	bool IsStopped();
	double GetIntervalMs() { return intervalMs_; }
	void SetLength(long images) { numImages_ = images; }
	long GetLength() const { return numImages_; }
	long GetImageCounter() { return imageCounter_; }

private:
	int svc(void) throw();
	Etaluma* camera_;
	bool stop_;
	long numImages_;
	long imageCounter_;
	double intervalMs_;
};

#endif //_MMCAMERA_H_
