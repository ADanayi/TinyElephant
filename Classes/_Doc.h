// In the name of Allah

#ifndef _FDB_DOC_H
#define _FDB_DOC_H

#include "../DiskDriver/DiskDriverBase.h"
#include "../setup.h"
#include "_ReadField.h"
#include "_DocWriter.h"

namespace elephant
{
    class Doc
    {
    public:
        explicit Doc();
        explicit Doc(const char *path, DiskDriverBase *diskdriver);
        bool is_inited() const;
        operator bool() const;
        size_t data_len() const;
        ReadField operator[](const char *name) const;
        ReadField operator[](const size_t ind) const;
        const unsigned char *data() const;
        unsigned char *_edit_data();
        bool _edit_data_len(size_t set_to);
        bool commit();
        bool replace(DocWriter &new_doc);
        ReadField next(size_t &data_offset) const;

    private:
        char _path[TE_PATH_BUF_LEN];
        DiskDriverBase *const _dd;
        bool _is_inited;
        unsigned char _data[TE_DOC_MAX_SIZE];
        size_t _data_len;
    };
}

namespace elephant
{
    ReadField Doc::next(size_t &data_offset) const
    {
        if (!_is_inited)
            return ReadField();
        if (data_offset > _data_len - 4)
            return ReadField();
        ReadField f(_data + data_offset, _data_len - data_offset + 1);
        data_offset += f.pure_field_len();
        return f;
    }

    Doc::Doc() : _dd(nullptr)
    {
        _is_inited = false;
        _data_len = 0;
    }

    Doc::Doc(
        const char *path,
        DiskDriverBase *diskdrive) : _dd(diskdrive)
    {
        _is_inited = false;
        _data_len = 0;

        if (_dd == nullptr)
        {
            // Serial.println("Error 1");
            return;
        }

        strcpy(_path, path);

        if (!_dd->read(_path, _data, _data_len, TE_DOC_MAX_SIZE)) {
            // Serial.println(_path);
            // Serial.println("Error 3");
            return;
        }

        _is_inited = true;
    }

    size_t Doc::data_len() const
    {
        return _data_len;
    }

    const unsigned char *Doc::data() const
    {
        return (const unsigned char *)_data;
    }

    unsigned char *Doc::_edit_data()
    {
        return _data;
    }

    bool Doc::_edit_data_len(size_t set_to)
    {
        if (set_to <= TE_DOC_MAX_SIZE)
        {
            _data_len = set_to;
            return true;
        }
        _data_len = TE_DOC_MAX_SIZE;
        return false;
    }

    bool Doc::commit()
    {
        return _dd->write(_path, _data, _data_len);
    }

    bool Doc::replace(DocWriter &dw)
    {
        return _dd->write(_path, dw.data(), dw.data_len());
    }

    // ReadField Doc::operator[](const char *name) const
    // {
    //     size_t d_ctr = 0;
    //     while (d_ctr < _data_len)
    //     {
    //         ReadField R(_data + d_ctr, TE_DOC_MAX_SIZE - d_ctr);
    //         if (!R)
    //             return R;
    //         if (R == name)
    //             return R;
    //         d_ctr += R.pure_field_len();
    //     }
    //     return ReadField();
    // }

    ReadField Doc::operator[](const char *name) const
    {
        size_t d_ctr = 0;
        while (1)
        {
            ReadField R = next(d_ctr);
            if (!R)
                return R;
            if (R == name)
                return R;
            d_ctr += R.pure_field_len();
        }
    }

    // ReadField Doc::operator[](const size_t ind) const
    // {
    //     size_t d_ctr = 0;
    //     size_t ind_ctr = 0;
    //     while (d_ctr < _data_len)
    //     {
    //         ReadField R(_data + d_ctr, TE_DOC_MAX_SIZE - d_ctr);
    //         if (!R)
    //             return R;
    //         if (ind_ctr == ind)
    //             return R;
    //         ind_ctr++;
    //         d_ctr += R.pure_field_len();
    //     }
    //     return ReadField();
    // }

    bool Doc::is_inited() const
    {
        return _is_inited;
    }

    Doc::operator bool() const
    {
        return _is_inited;
    }
}

#endif