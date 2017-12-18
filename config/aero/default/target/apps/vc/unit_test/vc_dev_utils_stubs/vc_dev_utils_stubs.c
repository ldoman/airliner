#include <stdio.h>
#include <jpeglib.h>

struct jpeg_error_mgr * jpeg_std_error (struct jpeg_error_mgr * err)
{
    return 0;
}


void jpeg_CreateCompress (j_compress_ptr cinfo, int version, size_t structsize)
{
    
}


void jpeg_mem_dest (j_compress_ptr cinfo, unsigned char ** outbuffer, unsigned long * outsize)
{
    
}


void jpeg_set_defaults (j_compress_ptr cinfo)
{
    
}


void jpeg_set_quality (j_compress_ptr cinfo, int quality, boolean force_baseline)
{
    
}


void jpeg_start_compress (j_compress_ptr cinfo, boolean write_all_tables)
{
    
}


GLOBAL(JDIMENSION)
jpeg_write_scanlines (j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines)
{
    
}


void jpeg_finish_compress (j_compress_ptr cinfo)
{
    
}


void jpeg_destroy_compress (j_compress_ptr cinfo)
{
    
}
