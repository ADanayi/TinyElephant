// In the name of Allah

#include "../setup.h"

#ifndef _READ_FIELD_TINY_H
#define _READ_FIELD_TINY_H

namespace elephant
{
    class ReadField
    {
    public:
        ReadField(const unsigned char *const plain_data, size_t maxLen);
        explicit ReadField();
        const char *name() const;
        const size_t data_len() const;
        const size_t pure_field_len() const;
        const unsigned char *data() const;
        operator bool() const;
        bool operator==(const char *name) const;
        const char *str(bool &conversion_ok) const;

    private:
        const unsigned char *const _pdata;
        const unsigned char *_data;
        size_t _len;
        size_t _pure_len;
    };
}

namespace elephant
{
    const char *ReadField::str(bool &conversion_ok) const
    {
        conversion_ok = true;
        if (_data[_len - 1] != '\0')
            conversion_ok = false;
        return (const char *)_data;
    }

    ReadField::ReadField() : _pdata(nullptr)
    {
        _data = nullptr;
    }

    ReadField::ReadField(const unsigned char *const plain_data, size_t maxLen) : _pdata(plain_data)
    {
        _len = 0;
        _pure_len = 0;
        _data = nullptr;

        // Let's skip the name part!
        _pure_len += strlen((const char *const)_pdata) + 1;
        if (_pure_len >= maxLen - 2)
        {
            _pure_len = maxLen;
            _len = 0;
            return;
        }

        // Let's read the len
        _len = atol((const char *)_pdata + _pure_len);

        // Now, let's skip the len part!
        _pure_len += strlen((const char *const)_pdata + _pure_len) + 1;
        if (_pure_len >= maxLen - _len)
        {
            _pure_len = maxLen;
            _len = 0;
            return;
        }

        // Let's set the data pointer
        _data = _pdata + _pure_len;

        // Let's set the pure len
        _pure_len += _len;
    }

    const char *ReadField::name() const
    {
        if (_data == nullptr)
            return "";
        return (const char *)_pdata;
    }

    const size_t ReadField::pure_field_len() const
    {
        return _pure_len;
    }

    const size_t ReadField::data_len() const
    {
        return _len;
    }

    const unsigned char *ReadField::data() const
    {
        return _data;
    }
    ReadField::operator bool() const
    {
        return _data != nullptr;
    }
    bool ReadField::operator==(const char *name) const
    {
        if (_data == nullptr) {
            // Serial.println("here!!!");
            return false;
        }
        size_t ctr = 0;
        while (1)
        {
            if (name[ctr] != _pdata[ctr])
                return false;
            if (name[ctr] == '\0')
                return true;
            ctr++;
        }
        return false;
    }
}

#endif
