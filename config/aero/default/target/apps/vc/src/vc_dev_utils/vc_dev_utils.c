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

/************************************************************************
** Includes
*************************************************************************/
/*
 * Include file for users of JPEG library.
 * You will need to have included system headers that define at least
 * the typedefs FILE and size_t before you can include jpeglib.h.
 * (stdio.h is sufficient on ANSI-conforming systems.)
 * You may also wish to include "jerror.h".
 */
#include <stdio.h>
#include "string.h"
#include <inttypes.h>
#include <jpeglib.h>
#include <setjmp.h>

#include "vc_platform_cfg.h"

/************************************************************************
** Local Defines
*************************************************************************/
#define VC_UTILS_JPEG_QUALITY       (100)
#define VC_UTILS_IMAGE_WIDTH        VC_FRAME_WIDTH
#define VC_UTILS_IMAGE_HEIGHT       VC_FRAME_HEIGHT
#define VC_UTILS_COLOR_COMP_PER_PIX (3)
/* libJPEG expects YUV 3bytes, 24bit */
#define VC_UTILS_COLOR_SPACE        JCS_YCbCr
#define VC_UTILS_BUFFER_SIZE        VC_MAX_BUFFER_SIZE 


/************************************************************************
** Structure Declarations
*************************************************************************/
/* TODO setup memory pool. The memory manager back end needs to be 
 * modified to not use malloc and free see jmemnobs.c 
 */

/* libjpeg will call exit() if the default error handler is used
 * so we override the standard error routine. C's setjmp/longjmp is
 * used to return control to the caller. See libjpeg example.c error
 * handling for more info. 
 */

struct jpegErrorManager
{
    /* "public" fields */
    struct jpeg_error_mgr pub;
    /* for return to caller */
    jmp_buf setjmp_buffer;
};

typedef struct jpegErrorManager * my_error_ptr;


/************************************************************************
** External Global Variables
*************************************************************************/


/************************************************************************
** Function Prototypes
*************************************************************************/
void jpegErrorExit(j_common_ptr cinfo)
{
  /* cinfo->err actually points to a jpegErrorManager struct */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  /* note : *(cinfo->err) is now equivalent to myerr->pub */
  
  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


int VC_Custom_Utils_YUVY2JPG(const char *Input, char *Output, int *SizeOut)
{
    struct jpeg_compress_struct cinfo;
    struct jpegErrorManager jerr;
    int row_stride = 0;
    uint32_t outlen = VC_UTILS_BUFFER_SIZE;
    int returnCode = 0;

    /* Null check */
    if(0 == Input || 0 == Output)
    {
        returnCode = -1;
        goto end_of_function;
    }

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpegErrorExit;
    
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer)) 
    {
        /* If we get here, the JPEG code has signaled an error. */
        returnCode = -2;
        goto end_of_function;
    }
    
    /* Initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    /* Set destination */
    jpeg_mem_dest(&cinfo, (unsigned char **)&Output, (long unsigned int *)&outlen);

    cinfo.image_width = VC_UTILS_IMAGE_WIDTH;
    cinfo.image_height = VC_UTILS_IMAGE_HEIGHT;
    cinfo.input_components = VC_UTILS_COLOR_COMP_PER_PIX;
    cinfo.in_color_space = VC_UTILS_COLOR_SPACE;

    jpeg_set_defaults(&cinfo);
    /* Quantization table scaling */
    jpeg_set_quality(&cinfo, VC_UTILS_JPEG_QUALITY, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    /* JSAMPLEs per row in image_buffer */
    char tmprowbuf[VC_UTILS_IMAGE_WIDTH * 3];

    JSAMPROW row_pointer[1];
    row_pointer[0] = &tmprowbuf[0];
    
    while (cinfo.next_scanline < cinfo.image_height) 
    {
        unsigned i, j;
        /* offset to the correct row */
        unsigned offset = cinfo.next_scanline * cinfo.image_width * 2;
        /* input strides by 4 bytes, output strides by 6 (2 pixels) */
        for (i = 0, j = 0; i < cinfo.image_width * 2; i += 4, j += 6) { 
            /* Y (unique to this pixel) */
            tmprowbuf[j + 0] = Input[offset + i + 0];
            /* U (shared between pixels) */
            tmprowbuf[j + 1] = Input[offset + i + 1];
            /* V (shared between pixels) */
            tmprowbuf[j + 2] = Input[offset + i + 3];
            /* Y (unique to this pixel) */
            tmprowbuf[j + 3] = Input[offset + i + 2];
            /* U (shared between pixels) */
            tmprowbuf[j + 4] = Input[offset + i + 1];
            /* V (shared between pixels) */
            tmprowbuf[j + 5] = Input[offset + i + 3];
        }
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);

    jpeg_destroy_compress(&cinfo);

    *SizeOut = outlen;

end_of_function:

    return returnCode;
}
