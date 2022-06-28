// In the name of Allah

#ifndef _FDB_FOLDER_H
#define _FDB_FOLDER_H

#include "../DiskDriver/DiskDriverBase.h"
#include "../setup.h"
#include "./_Doc.h"

namespace elephant
{
    class Folder
    {
    public:
        explicit Folder();
        explicit Folder(const char *path, const size_t N_layers, const size_t layer_index, DiskDriverBase *diskdriver);
        const char *const path() const;
        DiskDriverBase *const dd() const;
        bool child_exists(size_t child_number) const;
        size_t len() const;  // Number of children
        size_t head() const; // Number of first child (head + len does not exist and head + len - 1 exists)
        size_t tail() const;
        Folder get_child_folder(const size_t child_number) const; // Note: Use carefully with len consideration!
        Doc get_child_doc(const size_t child_number) const;
        const size_t N_layers;
        const size_t layer_index;
        const bool is_root;
        const bool is_endpoint;
        bool is_inited() const;
        operator bool() const;

    protected:
        void child_path(char *child_path_buf, size_t child_number) const;
        size_t load_config_file(const char *config, const bool resave_and_synch = true, const size_t default_value = 0); // Creates the len file if not existing and sets it to 0 (also tries to load the backup (.dbb) too.)
        bool save_config_file(const char *config, const size_t val, bool save_backup = true);
        bool save_backup_config_file(const char *config, const size_t val);
        size_t scan_for_len(); // Not recommended and I try not to use it at all!

    private:
        char _path[TE_PATH_BUF_LEN];
        size_t _len;
        size_t _head;
        DiskDriverBase *const _dd;
        bool _is_inited;
    };
};

namespace elephant
{
    Folder::Folder() : _dd(nullptr), N_layers(0), layer_index(0), is_root(false), is_endpoint(false)
    {
        _is_inited = false;
    }

    bool Folder::is_inited() const
    {
        return _is_inited;
    }

    Folder::operator bool() const
    {
        return _is_inited;
    }

    Folder::Folder(
        const char *path,
        const size_t N_layers,
        const size_t layer_index,
        DiskDriverBase *diskdriver) : _dd(diskdriver),
                                      N_layers(N_layers),
                                      layer_index(layer_index),
                                      is_root(layer_index == 0),
                                      is_endpoint(layer_index == N_layers - 1)
    {
        _is_inited = false;

        // Some not inited cases!
        if (N_layers >= layer_index)
            return;
        if (_dd == nullptr)
            return;

        // Ensuring the folder exists!
        _dd->ensure_folder_exists(path);
        
        strcpy(_path, path);
        char pbuf[TE_PATH_BUF_LEN];


        // Finding len
        _len = load_config_file("len", true, 0);
        // Checking if a higher one exists (error while prev. writing len...)
        while (child_exists(_len))
        {
            _len++;
            save_config_file("len", _len, true);
        }

        // Finding head
        if (!is_root)
            _head = 0;
        else
        {
            _head = load_config_file("head", true, 0);
            // Checking if head is not saved prev...
            for (size_t ch = _head; ch < _head + _len; ch++)
            {
                if (child_exists(ch))
                {
                    _head = ch;
                    break;
                }
            }
        }

        _is_inited = true;
    }

    Folder Folder::get_child_folder(const size_t child_number) const
    {
        if (is_endpoint)
            return Folder();
        if (!_is_inited)
            return Folder();

        char chbuf[TE_PATH_BUF_LEN];
        child_path(chbuf, child_number);
        return Folder(chbuf, N_layers, layer_index + 1, _dd);
    }

    Doc Folder::get_child_doc(const size_t child_number) const
    {
        if (!is_endpoint)
            return Doc();
        if (!_is_inited)
            return Doc();

        char chbuf[TE_PATH_BUF_LEN];
        child_path(chbuf, child_number);
        return Doc(chbuf, _dd);
    }

    bool Folder::save_backup_config_file(const char *config, const size_t val)
    {
        char confile_path[TE_PATH_BUF_LEN];
        char valstr[64];
        sprintf(confile_path, "%s/%s.dbb", _path, config);
        sprintf(valstr, "%u", val);
        return _dd->write(confile_path, (unsigned char *)val, strlen(valstr) + 1);
    }

    bool Folder::save_config_file(const char *config, const size_t val, bool save_backup)
    {
        char confile_path[TE_PATH_BUF_LEN];
        char valstr[64];
        sprintf(confile_path, "%s/%s.db", _path, config);
        sprintf(valstr, "%u", val);
        if (!_dd->write(confile_path, (unsigned char *)valstr, strlen(valstr) + 1))
            return false;
        if (save_backup)
            return this->save_backup_config_file(config, val);
        return true;
    }

    size_t Folder::load_config_file(const char *config, const bool resave_and_synch, const size_t default_value)
    {
        size_t val = default_value;
        char confile_path[TE_PATH_BUF_LEN];
        char valstr[64];
        size_t valstr_len;

        sprintf(confile_path, "%s/%s.db", _path, config);
        bool original_exists = _dd->is_file(confile_path);
        if (!original_exists)
            strcat(confile_path, "b");

        if (_dd->is_file(confile_path))
        {
            _dd->read(confile_path, (unsigned char *)valstr, valstr_len, 64);
            valstr[valstr_len] = '\0';
            val = atoi(valstr);
        }

        if (resave_and_synch || !original_exists)
        {
            save_config_file(config, val, true);
        }
    }

    size_t Folder::scan_for_len()
    {
        if (is_root)
            return load_config_file("len", true, 0);
        else
        {
            size_t len = 0;
            for (len = 0; len < 10; len++)
            {
                if (!child_exists(len))
                    return len;
            }
        }
    }

    void Folder::child_path(char *child_path_buf, size_t child_number) const
    {
        sprintf(child_path_buf, "%s/%d", this->_path, child_number);
        if (is_endpoint)
        {
            strcat(child_path_buf, ".ted");
        }
    }

    bool Folder::child_exists(size_t child_number) const
    {
        if (!_is_inited) return false;
        char pbuf[TE_PATH_BUF_LEN];
        child_path(pbuf, child_number);
        if (is_endpoint)
        {
            return _dd->is_file(pbuf);
        }
        else
        {
            return _dd->is_dir(pbuf);
        }
    }

    const char *const Folder::path() const
    {
        return _path;
    }

    DiskDriverBase *const Folder::dd() const
    {
        return _dd;
    }

    size_t Folder::head() const
    {
        return _head;
    }

    size_t Folder::tail() const
    {
        return _head + _len - 1;
    }
};

#endif