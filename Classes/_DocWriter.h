// In the name of Allah

#ifndef __DOC_WRITER_H_ELEPHANT
#define __DOC_WRITER_H_ELEPHANT

#include "../setup.h"
#include "../DiskDriver/DiskDriverBase.h"
#include "./_ReadField.h"

namespace elephant
{
    class DocWriter
    {
    public:
        explicit DocWriter(unsigned char *const buffer, const size_t buffer_size);
        explicit DocWriter(DocWriter &) = delete;
        bool append_field(const char *name, size_t data_len, const unsigned char *data);
        bool append_field(const char *name, const char *str_data);
        bool append_field(const ReadField &rf);
        const unsigned char *data() const;
        size_t data_len() const;
        bool save_to_file(DiskDriverBase *dd, const char *path) const;
        void clean();

    private:
        unsigned char *const _buf;
        const size_t _bufSize;
        size_t _len;
    };

    static unsigned char __writer_singleton_buf[TE_DEFAULT_WRITER_BUF_SIZE];
#ifdef TE_USE_DEFAULT_WRITER_SINGLETON
    DocWriter writer(__writer_singleton_buf, TE_DEFAULT_WRITER_BUF_SIZE);
#endif
};

namespace elephant
{
    DocWriter::DocWriter(unsigned char *const buffer, const size_t buffer_size) : _buf(buffer), _bufSize(buffer_size)
    {
        _len = 0;
    }

    void DocWriter::clean()
    {
        _len = 0;
    }

    const unsigned char *DocWriter::data() const
    {
        return (const unsigned char *)_buf;
    }

    size_t DocWriter::data_len() const
    {
        return _len;
    }

    bool DocWriter::save_to_file(DiskDriverBase *dd, const char *path) const
    {
        return dd->write(path, _buf, _len);
    }

    bool DocWriter::append_field(const char *name, const char *str_data)
    {
        return append_field(name, strlen(str_data) + 1, (const unsigned char *)str_data);
    }

    bool DocWriter::append_field(const ReadField &rf)
    {
        size_t rf_len = rf.pure_field_len();
        if (rf_len + _len > _bufSize)
            return false;
        for (size_t i = 0; i < rf_len; i++)
        {
            _buf[_len] = rf.data()[i];
            _len++;
        }
        return true;
    }

    bool DocWriter::append_field(const char *name, size_t data_len, const unsigned char *data)
    {
        char len_str[32];
        ltoa(data_len, len_str, 10);
        size_t name_len = strlen(name);
        size_t len_str_len = strlen(len_str);
        size_t field_len = name_len + len_str_len + data_len + 2;
        if (field_len + _len > _bufSize)
            return false;

        for (size_t i = 0; i < name_len; i++)
        {
            _buf[_len] = name[i];
            _len++;
        }

        _buf[_len] = '\0';
        _len++;

        for (size_t i = 0; i < len_str_len; i++)
        {
            _buf[_len] = len_str[i];
            _len++;
        }

        _buf[_len] = '\0';
        _len++;

        for (size_t i = 0; i < data_len; i++)
        {
            _buf[_len] = data[i];
            _len++;
        }

        return true;
    }
};

#endif