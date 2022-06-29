// In the name of Allah

#ifndef __TINY_TYPES_H
#define __TINY_TYPES_H

#define TE_VERSION "0.0.1"

namespace elephant
{
    typedef unsigned long long int tenum;

    struct TinyOperationResult
    {
        bool status;
        tenum id;
        operator bool()
        {
            return status;
        };
    };

};

#endif