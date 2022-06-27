// In the name of Allah

#ifndef __FDB_H
#define __FDB_H

#include "DiskDriver/DiskDriverBase.h"

namespace tinyelephant
{
    struct TinyElephantConfig
    {
        const bool root_layer_autoclean;
        const unsigned long long int auto_clean_disk_limit_KB;
        const size_t middle_layers_number;
        const size_t *const middle_layers_sizes;
        const size_t final_layer_size;
    };

    class TinyElephant
    {
    public:
        explicit TinyElephant(DiskDriverBase *diskdriver, const char *root_path, const TinyElephantConfig config) : dd(diskdriver), conf(config)
        {
            dd->init();
        }
        explicit TinyElephant(TinyElephant &) = delete;

    protected:
    private:
        bool _inited;
        DiskDriverBase *const dd;
        const TinyElephantConfig conf;
    };
};

#endif