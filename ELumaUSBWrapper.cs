////////////////////////////////////////////////////////////////////////////////
// FILE:		ELumaUSBWrapper.cs
// PROJECT:		Etaluma
//------------------------------------------------------------------------------
// DESCRIPTION:	This is a C# file that grabs and returns public fields from the
//              LumaUSB.dll, so that the C++/CLI wrapper can access them.
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

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LumaUSB_ns;
using EtalumaStage;

namespace ELumaUSBWrapper
{
    public class LumaUSBPub
    {
        public int getPID_FX2_DEV()
        {
            return LumaUSB.PID_FX2_DEV;
        }

        public int getPID_LSCOPE()
        {
            return LumaUSB.PID_LSCOPE;
        }

        public int getVID_CYPRESS()
        {
            return LumaUSB.VID_CYPRESS;
        }

        public byte getIMAGE_SENSOR_BLUE_GAIN()
        {
            return LumaUSB.IMAGE_SENSOR_BLUE_GAIN;
        }

        public byte getIMAGE_SENSOR_GLOBAL_GAIN()
        {
            return LumaUSB.IMAGE_SENSOR_GLOBAL_GAIN;
        }

        public byte getIMAGE_SENSOR_GREEN1_GAIN()
        {
            return LumaUSB.IMAGE_SENSOR_GREEN1_GAIN;
        }

        public byte getIMAGE_SENSOR_GREEN2_GAIN()
        {
            return LumaUSB.IMAGE_SENSOR_GREEN2_GAIN;
        }

        public byte getIMAGE_SENSOR_RED_GAIN()
        {
            return LumaUSB.IMAGE_SENSOR_RED_GAIN;
        }

        public byte getIMAGE_SENSOR_RESET()
        {
            return LumaUSB.IMAGE_SENSOR_RESET;
        }

        public byte getIMAGE_SENSOR_SHUTTER_WIDTH_LOWER()
        {
            return LumaUSB.IMAGE_SENSOR_SHUTTER_WIDTH_LOWER;
        }

        public int getMAX_GLOBAL_GAIN_PARAMETER_VALUE()
        {
            return LumaUSB.MAX_GLOBAL_GAIN_PARAMETER_VALUE;
        }

        public int getMAX_IMAGE_SENSOR_EXPOSURE()
        {
            return LumaUSB.MAX_IMAGE_SENSOR_EXPOSURE;
        }

        public int getRECOMMENDED_MIN_GLOBAL_GAIN_PARAMETER_VALUE()
        {
            return LumaUSB.RECOMMENDED_MIN_GLOBAL_GAIN_PARAMETER_VALUE;
        }

        /*public LumaUSB DeviceAdded(libusbK.KLST_DEVINFO_HANDLE deviceInfo)
        {
            LumaUSB lumaUSB;
            lumaUSB.DeviceAdded(deviceInfo);
        }

        public bool GetLatest24bppBuffer(LumaUSB lumaUSB, out byte[] buffer)
        {
            return lumaUSB.GetLatest24bppBuffer(out buffer);
        }

        public string GetPixelClockDescription(int speed)
        {
            return LumaUSB.GetPixelClockDescription(speed);
        }

        public string getHexPath(LumaUSB lumaUSB)
        {
            return lumaUSB.HexPath;
        }

        public string getProductName(LumaUSB lumaUSB)
        {
            return lumaUSB.ProductName;
        }*/

    }

    public class VideoParameterPub
    {
        public byte[] getFrameDelimiter(VideoParameters videoParameters)
        {
            return videoParameters.frameDelimiter;
        }

        public bool setFrameDelimiter(VideoParameters videoParameters, byte[] frameDelimiter)
        {
            videoParameters.frameDelimiter = frameDelimiter;
            return true;
        }
    }

    public class StageControllerPub
    {
        public float getZ_MAX_TRAVEL_MILLIMETERS()
        {
            return StageController.Z_MAX_TRAVEL_MILLIMETERS;
        }
    }
}
