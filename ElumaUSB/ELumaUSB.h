////////////////////////////////////////////////////////////////////////////////
// FILE:		ELumaUSB.h
// PROJECT:		Etaluma
//------------------------------------------------------------------------------
// DESCRIPTION:	This is a wrapper for LumaUSB.dll. Since LumaUSB.dll is a C#
//				library, this wrapper is intended to expose the components and
//				methods so that they may be used in native C/C++. In order for
//				this wrapper to work, the libusbK drivers must be installed.
//
// AUTHOR:		Nicholas Schaub, nicholas.schaub@nist.gov
//				http://www.nist.gov/mml/bbd/biomaterials/nicholas-schaub.cfm
//
// LICENSE:     This file is distributed under the BSD license.
//              License text is included with the source distribution.
//
//              This file is distributed in the hope that it will be useful,
//              but WITHOUT ANY WARRANTY; without even the implied warranty
//              of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//              IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//              CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//              INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES.
//

#include <vector>
#include <string>

#pragma once
#ifdef __cplusplus
extern "C"
{
	using namespace std;

	class ELumaUSBPrivate;

	class __declspec(dllexport) ELumaUSB {
	private: ELumaUSBPrivate* _private;
	public: ELumaUSB();
	public: ELumaUSB(signed int vid, signed int pid, signed int width, signed int height);
	public: ~ELumaUSB();
	public: vector<string> GetDeviceDescriptionList();
	public: bool GetLatest24bppBuffer(unsigned char* cBuffer, int* count);
	public: bool GetNumBytesReceived(unsigned long long& numBytesReceived);
	public: string GetPixelClockDescription(int speed);
	public: int GetPixelClockDescriptionCount();
	public: bool ImageSensorRegisterRead(unsigned short registerId, unsigned short& value);
	public: bool ImageSensorRegisterWrite(unsigned short registerId, unsigned short value);
	public: bool InitializeGPIF();
	public: void InitImageSensor();
	public: bool ISOStreamStart();
	public: void ISOStreamStop();
	public: bool LedControllerWrite(unsigned char ledId, unsigned char brightness);
	public: void ResetNumBytesReceived();
	public: bool SetGlobalGain(unsigned short value);
	public: bool SetImageSensorPixelClockFrequency(int speed);
	public: bool SetWindowSize(int pixelCountSide);
	public: bool SetWindowSizeMethod(int width, int height);
	public: bool StartStreaming();
	public: bool StopStreaming();
	public: bool findUninitializedCamera();
	public: bool findInitializedCamera();
	public: signed int PID_FX2_DEV();
	public: signed int PID_LSCOPE();
	public: signed int VID_CYPRESS();
	public: unsigned char IMAGE_SENSOR_BLUE_GAIN();
	public: unsigned char IMAGE_SENSOR_GLOBAL_GAIN();
	public: unsigned char IMAGE_SENSOR_GREEN1_GAIN();
	public: unsigned char IMAGE_SENSOR_GREEN2_GAIN();
	public: unsigned char IMAGE_SENSOR_RED_GAIN();
	public: unsigned char IMAGE_SENSOR_RESET();
	public: unsigned char IMAGE_SENSOR_SHUTTER_WIDTH_LOWER();
	public: signed int MAX_GLOBAL_GAIN_PARAMETER_VALUE();
	public: signed int MAX_IMAGE_SENSOR_EXPOSURE();
	public: signed int RECOMMENDED_MIN_GLOBAL_GAIN_PARAMETER_VALUE();
	public: string HexPath();
	public: void HexPath(string hexPath);
	public: unsigned short ProductID();
	public: string ProductName();
	public: unsigned short VendorID();
	};
}
#endif
