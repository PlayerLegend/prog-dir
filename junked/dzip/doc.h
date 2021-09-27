
/** @file dzip/common/common.h
 This file describes the public interface for dzip.

 The workflow for deflating a range of bytes into dzip chunks is as follows: First, allocate a dzip state with dzip_deflate_state_new, then pass this state along with your input bytes and output buffer to dzip_deflate. Your output buffer will then have one or more new deflated chunks appended to it.

 To inflate a chunk back into its original bytes, simply pass the chunk and an output buffer to dzip_inflate.

 Some quality of life functions are defined separately in extensions.c, their declarations can be found at the end of this file.
 */
