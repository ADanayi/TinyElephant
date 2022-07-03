// In the name of Allah

#ifndef __TINY_TYPES_H
#define __TINY_TYPES_H

#define TE_VERSION "0.0.1"

namespace elephant
{
    typedef unsigned long long int tenum;

    enum class TinyOperationFailureReasonT
    {
        success = 0,
        commit_error = 1,
        write_error = 2,
        writer_error = 3
    };

    struct TinyOperationResult
    {
        bool status;
        tenum id;
        operator bool()
        {
            return status;
        };
        unsigned long long int time_loading_ms;
        unsigned long long int time_commit_ms;
        unsigned long long int time_doc_io_ms;
        TinyOperationFailureReasonT failure_reason;
    };

};

#endif
