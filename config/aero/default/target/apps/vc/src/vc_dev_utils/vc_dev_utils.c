/****************************************************************************
*
*   Copyright (c) 2017 Windhover Labs, L.L.C. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
* 3. Neither the name Windhover Labs nor the names of its 
*    contributors may be used to endorse or promote products derived 
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

//#include <opencv2/opencv.hpp>
#include "vc_dev_utils.h"

#include <inttypes.h>
#include <stdio.h>

#include "string.h"

#include <jpeglib.h>


//int VC_Custom_YUVY2JPG(const void *Input, void *Output, int *SizeOut, const int Width, const int Height)
//{
    //bool returnBool = false;
    //int returnCode = -1;
    //int params[2] = {0};
    ///* TODO */
    //cv::vector<uchar> outputBuffer;
    ///* TODO */
    //cv::Mat original(Width, Height, CV_8UC2, (uchar*)Input);
    //cv::Mat imgbgr;

    ///* Set quality */
    //params[0] = CV_IMWRITE_JPEG_QUALITY;
    //params[1] = 80;
    
    ///* Null check */
    //if (0 == Input || 0 == Output || 0 == SizeOut)
    //{
        //goto end_of_function;
    //}

    ///* Convert from YUYV to BGR (opencv format) */
    //cv::cvtColor(original, imgbgr, cv::COLOR_YUV2BGR_YUYV);
    ///* Encode to jpeg */
    //returnBool = cv::imencode(".jpg", imgbgr, outputBuffer, cv::vector<int>(params, params+1)); 

    //if(true == returnBool)
    //{
        ///* Copy to the output buffer */
        //*SizeOut = outputBuffer.size();
        //memcpy(Output, outputBuffer.data(), (size_t)SizeOut);
        //returnCode = 1;
    //}

//end_of_function:

    //return returnCode;
//}


int VC_Custom_Utils_YUVY2JPG(const char *Input, char *Output, int *SizeOut)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_ptr[1];
    int row_stride;

    uint8_t* outbuffer = NULL;
    uint32_t outlen = 0;

    /* Null check */
    if(0 == Input || 0 == Output)
    {

    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    /* TODO replace malloc */
    jpeg_mem_dest(&cinfo, &outbuffer, &outlen);

    // jrow is a libjpeg row of samples array of 1 row pointer
    /* TODO width */
    cinfo.image_width = 320 & -1;
    /* TODO height */
    cinfo.image_height = 240 & -1;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_YCbCr; //libJPEG expects YUV 3bytes, 24bit

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    /* TODO replace width */
    char tmprowbuf[320 * 3];

    JSAMPROW row_pointer[1];
    row_pointer[0] = &tmprowbuf[0];
    
    while (cinfo.next_scanline < cinfo.image_height) 
    {
        unsigned i, j;
        unsigned offset = cinfo.next_scanline * cinfo.image_width * 2; //offset to the correct row
        for (i = 0, j = 0; i < cinfo.image_width * 2; i += 4, j += 6) { //input strides by 4 bytes, output strides by 6 (2 pixels)
            tmprowbuf[j + 0] = Input[offset + i + 0]; // Y (unique to this pixel)
            tmprowbuf[j + 1] = Input[offset + i + 1]; // U (shared between pixels)
            tmprowbuf[j + 2] = Input[offset + i + 3]; // V (shared between pixels)
            tmprowbuf[j + 3] = Input[offset + i + 2]; // Y (unique to this pixel)
            tmprowbuf[j + 4] = Input[offset + i + 1]; // U (shared between pixels)
            tmprowbuf[j + 5] = Input[offset + i + 3]; // V (shared between pixels)
        }
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    *SizeOut = outlen;
    memcpy(Output, outbuffer, outlen);
}
