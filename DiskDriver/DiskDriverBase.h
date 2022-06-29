// In the name of Allah

#ifndef __SD_DRIVER_B_H_ADANAYI
#define __SD_DRIVER_B_H_ADANAYI

#include "string.h"

namespace elephant
{
    class DiskDriverBase
    {
    public:
        explicit DiskDriverBase(
            char *const path_str_buf,
            const size_t path_str_buf_max_len,
            const char *path_prefix = "") : path_str_buf(path_str_buf),
                                            path_str_buf_max_len(path_str_buf_max_len),
                                            path_prefix(path_prefix),
                                            path_prefix_len(strlen(path_prefix))
        {
            __inited = false;
        }
        explicit DiskDriverBase(DiskDriverBase &) = delete;
        bool inited()
        {
            return __inited;
        }
        bool init()
        {
            // Note: This function only works first time.
            if (__inited)
                return true;
            if (_init())
            {
                __inited = true;
                return true;
            }
            return false;
        }

        unsigned long long int disk_capacity_KB() { return _disk_capacity_KB(); }
        unsigned long long int disk_used_KB() { return _disk_used_KB(); }
        bool exists(const char *path, size_t path_len = 0)
        {
            /*
            if path_len is present (for example when working with string objects, you can give it to save more time. otherwise leave it to be 0)
            */
            if (!ready_path_str_buf(path, path_len))
                return false;
            return _exists(path_str_buf);
        }
        bool is_dir(const char *path, size_t path_len = 0)
        {
            /*
            if path_len is present (for example when working with string objects, you can give it to save more time. otherwise leave it to be 0)
            */
            if (!ready_path_str_buf(path, path_len))
                return false;
            if (!_exists(path_str_buf))
                return false;
            return _is_dir(path_str_buf);
        }
        bool is_file(const char *path, size_t path_len = 0)
        {
            /*
            if path_len is present (for example when working with string objects, you can give it to save more time. otherwise leave it to be 0)
            */
            if (!ready_path_str_buf(path, path_len))
                return false;
            if (!_exists(path_str_buf))
                return false;
            return !_is_dir(path_str_buf);
        }
        bool rmtree(const char *path, size_t path_len = 0)
        {
            /*
            if path_len is present (for example when working with string objects, you can give it to save more time. otherwise leave it to be 0)
            */
            if (!ready_path_str_buf(path, path_len))
                return false;
            if (!_exists(path_str_buf))
                return false;
            if (!_is_dir(path_str_buf))
                return false;
            return _rmtree(path_str_buf);
        }
        bool mkdir(const char *path, size_t path_len = 0)
        {
            /*
            if path_len is present (for example when working with string objects, you can give it to save more time. otherwise leave it to be 0)
            */
            if (!ready_path_str_buf(path, path_len))
                return false;
            return _mkdir(path_str_buf);
        }
        bool rmdir(const char *path, size_t path_len = 0)
        {
            /*
            if path_len is present (for example when working with string objects, you can give it to save more time. otherwise leave it to be 0)
            */
            if (!ready_path_str_buf(path, path_len))
                return false;
            return _rmdir(path_str_buf);
        }
        bool remove(const char *path, size_t path_len = 0)
        {
            /*
            if path_len is present (for example when working with string objects, you can give it to save more time. otherwise leave it to be 0)
            */
            if (!ready_path_str_buf(path, path_len))
                return false;
            return _remove(path_str_buf);
        }
        bool read(const char *path, unsigned char *buf, size_t &len, const size_t max_buf_len, size_t path_len = 0)
        {
            /*
            if path_len is present (for example when working with string objects, you can give it to save more time. otherwise leave it to be 0)
            */
            if (!ready_path_str_buf(path, path_len))
                return false;
            if (!_exists(path))
                return false;
            return _read(path, buf, len, max_buf_len);
        }
        virtual bool write(const char *path, const unsigned char *buf, const size_t buf_len, size_t path_len = 0)
        {
            /*
            if path_len is present (for example when working with string objects, you can give it to save more time. otherwise leave it to be 0)
            */
            if (!ready_path_str_buf(path, path_len))
                return false;
            return _write(path, buf, buf_len);
        }
        bool rename(const char *old_path, const char *new_path, size_t old_path_len = 0, size_t new_path_len = 0)
        {
            /*
            if path_len is present (for example when working with string objects, you can give it to save more time. otherwise leave it to be 0)
            */
            if ((old_path_len == 0) && old_path[0] != '\0')
                old_path_len = strlen(old_path);
            if ((new_path_len == 0) && new_path[0] != '\0')
                new_path_len = strlen(new_path);
            if (old_path_len + new_path_len < path_str_buf_max_len - 2 * path_prefix_len - 2)
                return false;
            char *const old_path_buf = path_str_buf;
            const size_t old_path_buf_len = old_path_len + path_prefix_len + 1;
            if (!__ready_path_str_buf(old_path, old_path_len, old_path_buf, old_path_buf_len))
                return false;
            char *const new_path_buf = this->path_str_buf + old_path_len;
            const size_t new_path_buf_len = new_path_len + path_prefix_len + 1;
            if (!__ready_path_str_buf(new_path, new_path_len, new_path_buf, new_path_buf_len))
                return false;
            _rename(old_path_buf, new_path_buf);
        }
        bool ensure_folder_exists(const char *path, size_t path_len = 0)
        {
            if (path_len == 0)
                path_len = strlen(path);
            if (path_len > path_str_buf_max_len - 1 - path_prefix_len)
                return false;
            if (path_len < 2)
                return false;
            if (path[0] != '/')
                return false;

            size_t ctr = 1;
            strcpy(path_str_buf, path_prefix);
            path_str_buf[path_prefix_len] = '/';

            char ch;
            do
            {
                ch = path[ctr];

                if (ch == '/')
                {
                    path_str_buf[ctr + path_prefix_len] = '\0';
                    if (!_exists(path_str_buf))
                    {
                        if (!_mkdir(path_str_buf))
                        {
                            return false;
                        }
                    }
                    else
                    {
                    }
                }

                path_str_buf[ctr + path_prefix_len] = ch;
                ctr += 1;
            } while (ctr <= path_len);
            if (ch != '/')
            {
                if (!exists(path, path_len))
                {
                    if (!mkdir(path, path_len))
                        return false;
                }
            }
            return true;
        }

    protected:
        virtual bool _init();
        virtual unsigned long int _disk_capacity_KB() = 0;
        virtual unsigned long int _disk_used_KB() = 0;
        virtual bool _exists(const char *path) = 0;
        virtual bool _mkdir(const char *path) = 0;
        virtual bool _rmdir(const char *path) = 0;
        virtual bool _remove(const char *path) = 0;
        virtual bool _rename(const char *path_from, const char *path_to) = 0;
        virtual bool _read(const char *path, unsigned char *buf, size_t &len, const size_t max_buf_len);
        virtual bool _write(const char *path, const unsigned char *buf, const size_t buf_len);
        virtual bool _is_dir(const char *path) = 0;
        virtual bool _rmtree(const char *path) = 0;

    private:
        bool __inited;
        const char *path_prefix;
        const size_t path_prefix_len;
        char *const path_str_buf;
        const size_t path_str_buf_max_len;

        bool __ready_path_str_buf(const char *path, size_t path_len, char *const buf, size_t buf_len)
        {
            // if path_len is present (for example when working with string objects, you can give it to save more time. otherwise leave it to be 0)
            if ((path_len == 0) && (path[0] != '\0'))
                path_len = strlen(path);
            if (path_len > buf_len - path_prefix_len - 1)
                return false;
            strcpy(buf, path_prefix);
            strcat(buf, path);
            return true;
        };

        bool ready_path_str_buf(const char *path, size_t path_len = 0)
        {
            return __ready_path_str_buf(path, path_len, this->path_str_buf, this->path_str_buf_max_len);
        };
    };
};

#endif
