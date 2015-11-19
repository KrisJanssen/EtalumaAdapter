////////////////////////////////////////////////////////////////////////////////
// FILE:		ELumaUSB.cpp
// PROJECT:		Etaluma
//------------------------------------------------------------------------------
// DESCRIPTION:	This is a wrapper for LumaUSB.dll. Since LumaUSB.dll is a C#
//				library, this wrapper is intended to expose the components and
//				methods so that they may be used in native C/C++. In order for
//				this wrapper to work, the libusbK drivers and the Etaluma 600/700
//        drivers must be installed.
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
#include "stdafx.h"

#include <msclr\auto_gcroot.h>
#include <msclr\marshal_cppstd.h>

using namespace libusbK;
using namespace System::Runtime::InteropServices; //Marshall
using namespace std;

// This class provides access to managed classes in the C# library. Without
// this, ELumaUSB will not function property with dllexport.
class ELumaUSBPrivate {
	// Provides access to the LumaUSB class in LumaUSB.dll.
	public: msclr::auto_gcroot<LumaUSB_ns::LumaUSB^> lumaUSB;
	// Provides access to LumaUSBPub class in the ELumaUSBWrapper.
	// LumaUSBWrapper exposes the public static fields in the LumaUSB class.
	public: msclr::auto_gcroot<ELumaUSBWrapper::LumaUSBPub^> lumaUSBPub;
};

// This class provides the functionality for the LumaUSB class in LumaUSB.dll.
// Not all of the methods in the LumaUSB class were implemented, but the ones
// that were implemented are named identically. This permits the functions to
// be accessed using the exact same syntax as if the LumaUSB.dll were being
// used. The exception to this rule is the function findAndInitializeCamera().
// This function searchs for an uninitialized camera and uses the C# library
// to load the hex file and  initialize the Etaluma camera.
class __declspec(dllexport) ELumaUSB {
	// Provides access to LumaUSB and LumaUSBPub.
	private: ELumaUSBPrivate* _private;

	// CONSTRUCTORS
	public: ELumaUSB() {
		_private = new ELumaUSBPrivate();
	}

	public: ELumaUSB(signed int vid, signed int pid, signed int width, signed int height) {
		_private = new ELumaUSBPrivate();
		_private->lumaUSB = gcnew LumaUSB_ns::LumaUSB(vid, pid, width, height);
	}

	// METHODS
	public: vector<string> GetDeviceDescriptionList() {
		System::Collections::Generic::List<System::String^>^ descriptionList = _private->lumaUSB->GetDeviceDescriptionList();
		vector<string> descriptionVector;
		msclr::interop::marshal_context context;
		for (int i = 0; i < descriptionList->Count; i++) {
			descriptionVector.push_back(context.marshal_as<string>(descriptionList[i]));
		}
		return descriptionVector;
	}

	public: bool GetLatest24bppBuffer(unsigned char* cBuffer, int* count) {
		cli::array<unsigned char>^ buffer;
		bool isFull = _private->lumaUSB->GetLatest24bppBuffer(buffer);
		pin_ptr<unsigned char> pinnedArray = &buffer[0];
		cBuffer = pinnedArray;
		*count = buffer->Length;
		return isFull;
	}
	
	public: bool GetNumBytesReceived(unsigned long long& numBytesReceived) {
		if (_private == nullptr) {
			return false;
		} else {
			return _private->lumaUSB->GetNumBytesReceived(numBytesReceived);
		}
	}

	public: string GetPixelClockDescription(int speed) {
		msclr::interop::marshal_context context;
		System::String^ clockDescription = _private->lumaUSB->GetPixelClockDescription(speed);
		return context.marshal_as<string>(clockDescription);
	}

	public: int GetPixelClockDescriptionCount() {
		return _private->lumaUSB->GetPixelClockDescriptionCount();
	}

	public: bool ImageSensorRegisterRead(unsigned short registerId, unsigned short& value) {
		return _private->lumaUSB->ImageSensorRegisterRead(registerId, value);
	}

	public: bool ImageSensorRegisterWrite(unsigned short registerId, unsigned short value) {
		return _private->lumaUSB->ImageSensorRegisterWrite(registerId, value);
	}

	public: bool InitializeGPIF() {
		return _private->lumaUSB->InitializeGPIF();
	}

	public: void InitImageSensor() {
		_private->lumaUSB->InitImageSensor();
	}

	public: bool ISOStreamStart() {
		return _private->lumaUSB->ISOStreamStart();
	}

	public: void ISOStreamStop() {
		_private->lumaUSB->ISOStreamStart();
	}

	public: bool LedControllerWrite(unsigned char ledId, unsigned char brightness) {
		return _private->lumaUSB->LedControllerWrite(ledId, brightness);
	}

	public: void ResetNumBytesReceived() {
		_private->lumaUSB->ResetNumBytesReceived();
	}

	public: bool SetGlobalGain(unsigned short value) {
		return _private->lumaUSB->SetGlobalGain(value);
	}

	public: bool SetImageSensorPixelClockFrequency(int speed) {
		return _private->lumaUSB->SetImageSensorPixelClockFrequency(speed);
	}

	public: bool SetWindowSize(int pixelCountSide) {
		return _private->lumaUSB->SetWindowSize(pixelCountSide);
	}

	public: bool SetWindowSizeMethod(int width, int height) {
		return _private->lumaUSB->SetWindowSize(width, height);
	}

	public: bool StartStreaming() {
		return _private->lumaUSB->StartStreaming();
	}

	public: bool StopStreaming() {
		return _private->lumaUSB->StopStreaming();
	}

	public: bool findUninitializedCamera()
	{
		int uninitProductId = _private->lumaUSBPub->getPID_FX2_DEV();
		int initProductId = _private->lumaUSBPub->getPID_LSCOPE();
		KLST_DEVINFO_HANDLE deviceInfo;
		bool deviceFound = false;
		int deviceIndex = 0;
		int deviceCount = 0;
		LstK^ lst = gcnew LstK(KLST_FLAG::NONE);
		int usbVendorId = _private->lumaUSBPub->getVID_CYPRESS();

		lst->Count(deviceCount);
		while (lst->MoveNext(deviceInfo))
		{
			if ((deviceInfo.Common.Vid == usbVendorId) && (deviceInfo.Common.Pid == uninitProductId))
			{
				deviceFound = true;
				break; // Exit the loop so that we don't over-write the struct.
			}
		}

		if (deviceFound) {
			_private->lumaUSB->DeviceAdded(deviceInfo);
		}

		return deviceFound;
	}

	public: bool findInitializedCamera()
	{
		int uninitProductId = _private->lumaUSBPub->getPID_FX2_DEV();
		int initProductId = _private->lumaUSBPub->getPID_LSCOPE();
		KLST_DEVINFO_HANDLE deviceInfo;
		bool deviceFound = false;
		int deviceIndex = 0;
		int deviceCount = 0;
		LstK^ lst = gcnew LstK(KLST_FLAG::NONE);
		int usbVendorId = _private->lumaUSBPub->getVID_CYPRESS();

		lst->Count(deviceCount);
		while (lst->MoveNext(deviceInfo))
		{
			if ((deviceInfo.Common.Vid == usbVendorId) && (deviceInfo.Common.Pid == initProductId))
			{
				deviceFound = true;
				break; // Exit the loop so that we don't over-write the struct.
			}
		}

		if (deviceFound) {
			_private->lumaUSB->DeviceAdded(deviceInfo);
		}

		return deviceFound;
	}

	// Access to C# library public fields.
	public: signed int PID_FX2_DEV() {
		return _private->lumaUSBPub->getPID_FX2_DEV();
	}

	public: signed int PID_LSCOPE() {
		return _private->lumaUSBPub->getPID_LSCOPE();
	}

	public: signed int VID_CYPRESS() {
		return _private->lumaUSBPub->getVID_CYPRESS();
	}

	public: unsigned char IMAGE_SENSOR_BLUE_GAIN() {
		return _private->lumaUSBPub->getIMAGE_SENSOR_BLUE_GAIN();
	}

	public: unsigned char IMAGE_SENSOR_GLOBAL_GAIN() {
		return _private->lumaUSBPub->getIMAGE_SENSOR_GLOBAL_GAIN();
	}

	public: unsigned char IMAGE_SENSOR_GREEN1_GAIN() {
		return _private->lumaUSBPub->getIMAGE_SENSOR_GREEN1_GAIN();
	}

	public: unsigned char IMAGE_SENSOR_GREEN2_GAIN() {
		return _private->lumaUSBPub->getIMAGE_SENSOR_GREEN2_GAIN();
	}

	public: unsigned char IMAGE_SENSOR_RED_GAIN() {
		return _private->lumaUSBPub->getIMAGE_SENSOR_RED_GAIN();
	}

	public: unsigned char IMAGE_SENSOR_RESET() {
		return _private->lumaUSBPub->getIMAGE_SENSOR_RESET();
	}

	public: unsigned char IMAGE_SENSOR_SHUTTER_WIDTH_LOWER() {
		return _private->lumaUSBPub->getIMAGE_SENSOR_SHUTTER_WIDTH_LOWER();
	}

	public: signed int MAX_GLOBAL_GAIN_PARAMETER_VALUE() {
		return _private->lumaUSBPub->getMAX_GLOBAL_GAIN_PARAMETER_VALUE();
	}

	public: signed int MAX_IMAGE_SENSOR_EXPOSURE() {
		return _private->lumaUSBPub->getMAX_IMAGE_SENSOR_EXPOSURE();
	}

	public: signed int RECOMMENDED_MIN_GLOBAL_GAIN_PARAMETER_VALUE() {
		return _private->lumaUSBPub->getRECOMMENDED_MIN_GLOBAL_GAIN_PARAMETER_VALUE();
	}

	public: ~ELumaUSB() {
		delete _private;
	}

	// PROPERTIES
	public: string HexPath() { //get
		msclr::interop::marshal_context context;
		System::String^ hexPath = _private->lumaUSB->HexPath;
		return context.marshal_as<string>(hexPath);
	}

	public: void HexPath(string hexPath) { //set
		msclr::interop::marshal_context context;
		_private->lumaUSB->HexPath = context.marshal_as<System::String^>(hexPath);
	}

	public: unsigned short ProductID() { //get
		return _private->lumaUSB->ProductID;
	}

	public: string ProductName() { //get
		msclr::interop::marshal_context context;
		System::String^ productName = _private->lumaUSB->ProductName;
		return context.marshal_as<string>(productName);
	}

	public: unsigned short VendorID() { //get
		return _private->lumaUSB->VendorID;
	}
};
