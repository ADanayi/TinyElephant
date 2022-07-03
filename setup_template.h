// In the name of Allah

#ifndef __TINY_H_SETUP
#define __TINY_H_SETUP

/*  The buffer length for path strings.
    This depends on the number of layers, estimated number of documents, root path and ...
    Usually 128 or at least 64 are preferred.
*/
#define TE_PATH_BUF_LEN 128

/*  Default buffers allow you to work with tiny elephant,
    without allocating static or dynamic memories, outside the code.

    Note:   For small and cheap hardware sets it's more rational to disable this feature.
*/
#define TE_USE_DEFAULT_BUFFERS

#ifdef TE_USE_DEFAULT_BUFFERS
/*
    If DEFAULT BUFFERS are enabled, then DEFAULT_WRITER_BUF_SIZE
    determines the maximum buffer given to a default doc writer object.

    Note:   It is different than data size.
            You can check the output of "append_field" to be sure your data still fits the buffer.
            The amount of this buffer depends on your application.
*/
#define TE_DEFAULT_WRITER_BUF_SIZE 512
#endif

#endif
