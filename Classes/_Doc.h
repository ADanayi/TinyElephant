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
        explicit Doc(const char *path, DiskDriverBase *diskdriver, unsigned char *const data_buf, const size_t data_buf_size);
        bool is_inited() const;
        operator bool() const;
        size_t data_len() const;
        ReadField operator[](const char *name) const;
        const unsigned char *data() const;
        unsigned char *_edit_data();
        bool _edit_data_len(size_t set_to);
        bool commit();
        bool replace(DocWriter &new_doc);
        ReadField next(size_t &data_offset) const;

    protected:
        unsigned char *_data;
        size_t _data_size;
        bool _is_inited;
        char _path[TE_PATH_BUF_LEN];
        DiskDriverBase *_dd;
        size_t _data_len;
    };

#ifdef TE_USE_DEFAULT_BUFFERS
    class DefaultDoc : public Doc
    {
    private:
        static unsigned char *allocate_memory()
        {
            return new unsigned char[TE_DEFAULT_WRITER_BUF_SIZE];
        }
        static unsigned char *steal_memory(DefaultDoc *d)
        {
            if (d->_is_inited)
            {
                unsigned char *ret = d->_data;
                d->_data = nullptr;
                return ret;
            }
            else
                return nullptr;
        }
        static unsigned char *copy_memory(const DefaultDoc *d)
        {
            if (d->_is_inited)
            {
                unsigned char ret[TE_DEFAULT_WRITER_BUF_SIZE];
                for (size_t i = 0; i < TE_DEFAULT_WRITER_BUF_SIZE; i++)
                    ret[i] = d->_data[i];
                return ret;
            }
            else
                return nullptr;
        }

    public:
        explicit DefaultDoc() : Doc(){};
        explicit DefaultDoc(const char *path, DiskDriverBase *diskdriver) : Doc(path, diskdriver, allocate_memory(), TE_DEFAULT_WRITER_BUF_SIZE){};
        DefaultDoc(const DefaultDoc &d) : Doc()
        {
            this->_data = copy_memory(&d);
            this->_data_len = d._data_len;
            this->_data_size = d._data_size;
            this->_is_inited = d._is_inited;
            strcpy(this->_path, d._path);
            this->_dd = d._dd;
        }
        ~DefaultDoc()
        {
            if (_data != nullptr)
            {
                delete _data;
            }
        }
    };
#endif
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

    Doc::Doc() : _dd(nullptr), _data(nullptr), _data_size(0)
    {
        _is_inited = false;
        _data_len = 0;
    }

    Doc::Doc(
        const char *path,
        DiskDriverBase *diskdrive,
        unsigned char *const data_buf,
        const size_t data_buf_size) : _dd(diskdrive),
                                      _data(data_buf),
                                      _data_size(data_buf_size)
    {
        _is_inited = false;
        _data_len = 0;

        if (_dd == nullptr)
        {
            return;
        }

        strcpy(_path, path);

        if (!_dd->read(_path, _data, _data_len, _data_size))
        {
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
        if (set_to <= _data_size)
        {
            _data_len = set_to;
            return true;
        }
        _data_len = _data_size;
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

    ReadField Doc::operator[](const char *name) const
    {
        if (!_is_inited)
            return ReadField();
        size_t d_ctr = 0;
        while (1)
        {
            ReadField R = next(d_ctr);
            if (!R)
                return R;
            if (R == name)
                return R;
        }
    }

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