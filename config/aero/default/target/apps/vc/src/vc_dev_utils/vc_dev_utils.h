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

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/** \brief Convert raw YUVY 4:2:2 buffer to JPEG.
**
**  \par Description
**       From V4L2-PIX-FMT-YUYV documenation: In this format each four
**       bytes is two pixels. Each four bytes is two Y's, a Cb and a Cr.
**       Each Y goes to one of the pixels, and the Cb and Cr belong to 
**       both pixels.
**       Example V4L2_PIX_FMT_YUYV 4 × 4 pixel image:
**
**       Byte Order. Each cell is one byte.
**       start + 0:  Y'00 Cb00 Y'01 Cr00 Y'02 Cb01 Y'03 Cr01
**       start + 8:  Y'10 Cb10 Y'11 Cr10 Y'12 Cb11 Y'13 Cr11
**       start + 16: Y'20 Cb20 Y'21 Cr20 Y'22 Cb21 Y'23 Cr21
**       start + 24: Y'30 Cb30 Y'31 Cr30 Y'32 Cb31 Y'33 Cr31
**
**       Color Sample Location.
**        0   1  2   3
**      0 Y C Y  Y C Y
**      1 Y C Y  Y C Y
**      2 Y C Y  Y C Y
**      3 Y C Y  Y C Y
**
**  \param [in]  Input    The pointer to the raw YUYV 4:2:2 buffer.
**
**  \param [out] Output   The pointer to the buffer to write the 
**                        converted JPEG.
**
**  \param [out] SizeOut  The output size of the compressed JPEG.
**
**  \returns int 0 for success, -1 for null pointer, -2 for an internal
**               libjpeg error. 
**
*************************************************************************/
int VC_Custom_Utils_YUVY2JPG(const char *Input, char *Output, int *SizeOut);

#ifdef __cplusplus
}
#endif 


