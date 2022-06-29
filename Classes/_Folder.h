// In the name of Allah

#ifndef _FDB_FOLDER_H
#define _FDB_FOLDER_H

#include "../DiskDriver/DiskDriverBase.h"
#include "../setup.h"
#include "./_Doc.h"
#include "../types.h"

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
        Folder get_child_folder(const size_t child_number) const; // Note: Use carefully with len consideration!
        // Doc get_child_doc(const size_t child_number) const;
        size_t child_first() const;
        size_t child_count() const;
        size_t child_last() const; // Warning: It returns invalid data if the folder is empty
        const size_t N_layers;
        const size_t layer_index;
        const bool is_root;
        const bool is_endpoint;
        bool is_inited() const;
        operator bool() const;
        bool is_full() const;
        bool is_empty() const;
        tenum docs_count() const;
        bool _increase_first();

        void child_path(char *child_path_buf, size_t child_number) const;
        size_t load_config_file(const char *config, const bool resave_and_synch = true, const size_t default_value = 0); // Creates the len file if not existing and sets it to 0 (also tries to load the backup (.dbb) too.)
        bool save_config_file(const char *config, const size_t val, bool save_backup = true);
        bool save_backup_config_file(const char *config, const size_t val);
        size_t scan_for_len(); // Not recommended and I try not to use it at all!
        size_t get_child_number_for_id(tenum id) const;
        size_t get_child_number_for_id(const char *id, size_t id_str_len = 0) const;
        tenum calc_last_doc_sub_id() const; // Warning: It returns invalid data if the folder is empty
        tenum calc_first_doc_sub_id() const;
        bool commit_configs_for_inc(tenum id);
        bool commit_configs_for_inc(const char *id, size_t id_str_len = 0);

    private:
        char _path[TE_PATH_BUF_LEN];
        size_t _child_count;
        size_t _child_first;
        DiskDriverBase *const _dd;
        bool _is_inited;
    };
};

namespace elephant
{
    bool Folder::_increase_first()
    {
        if (!is_root)
            return false;
        if (is_empty())
            return false;
        _child_first += 1;
        save_config_file("first", _child_first, true);
    }

    bool Folder::commit_configs_for_inc(tenum id)
    {
        char buf[TE_PATH_BUF_LEN];
        itoa(id, buf, 10);
        return commit_configs_for_inc(buf, strlen(buf));
    }

    bool Folder::commit_configs_for_inc(const char *id, size_t id_str_len)
    {
        if (id_str_len == 0)
            id_str_len = strlen(id);

        size_t chid = get_child_number_for_id(id, id_str_len);

        if (chid < _child_first) // This effectively only happens for root
            return false;

        if ((_child_count > 0) && chid < child_last()) // Has been set before
            return true;

        if ((_child_count == 0) || (_child_count > 0 && chid == child_last()) || (_child_count > 0 && chid == child_last() + 1))
        {
            // Is the count different? (for example when we have an empty folder this happens.)
            size_t first = _child_first;
            size_t count = chid - first + 1;
            if (_child_count > 0 && count < _child_count)
                return true;
            if (count != _child_count)
                save_config_file("count", count, true);

            if (!is_endpoint)
                return get_child_folder(chid).commit_configs_for_inc(id, id_str_len);
            else
                return true;
        }

        // If none of above happened, there is id violation! return false!
        return false;
    }

    tenum Folder::calc_first_doc_sub_id() const
    {
        if (is_root && is_empty())
            return 0;
        if (is_endpoint)
            return child_first();
        tenum p = 1;
        for (size_t i = 0; i < N_layers - 1 - layer_index; i++)
            p *= 10;
        return child_first() * p + get_child_folder(child_first()).calc_first_doc_sub_id();
    }

    tenum Folder::calc_last_doc_sub_id() const
    {
        if (is_endpoint)
            return child_last();
        tenum p = 1;
        for (size_t i = 0; i < N_layers - 1 - layer_index; i++)
            p *= 10;
        return child_last() * p + get_child_folder(child_last()).calc_last_doc_sub_id();
    }

    size_t Folder::get_child_number_for_id(tenum id) const
    {
        char buf[TE_PATH_BUF_LEN];
        itoa(id, buf, 10);
        return get_child_number_for_id(buf, strlen(buf));
    }

    size_t Folder::get_child_number_for_id(const char *id, size_t id_str_len) const
    {
        // Formula: Except the root one, for layer l -> s = id_str_len - N_layer + l
        //          Child number = id[s] (atoi)
        //          For the root one, we have the string [0, 1, ..., id_str_len - N_layer (formula with l=0)]
        if (id_str_len == 0)
            id_str_len = strlen(id);

        // s >= 0. otherwise it has been a zero-pad!
        if (id_str_len < this->N_layers - this->layer_index)
            return 0;

        if (is_root)
        {
            char buf[TE_PATH_BUF_LEN];
            size_t i;
            for (i = 0; i <= id_str_len - N_layers; i++)
            {
                buf[i] = id[i];
            }
            buf[i + 1] = '\0';
            return atoi(buf);
        }
        else
        {
            return id[id_str_len - this->N_layers + this->layer_index] - '0';
        }
    }

    tenum Folder::docs_count() const
    {
        if (is_endpoint)
            return _child_count;
        else
        {
            tenum val = 0;
            for (size_t ch = 0; ch < _child_count; ch++)
            {
                Folder child = get_child_folder(ch);
                if (child)
                    val += child.child_count();
            }
            return val;
        }
    }

    size_t Folder::child_first() const
    {
        return _child_first;
    }
    size_t Folder::child_count() const
    {
        return _child_count;
    }

    size_t Folder::child_last() const
    {
        return _child_first + _child_count - 1;
    }

    Folder::Folder() : _dd(nullptr), N_layers(0), layer_index(0), is_root(false), is_endpoint(false)
    {
        _is_inited = false;
    }

    bool Folder::is_inited() const
    {
        return _is_inited;
    }

    bool Folder::is_empty() const
    {
        if (!_is_inited)
            return false;
        return _child_count == 0;
    }

    bool Folder::is_full() const
    {
        if (!_is_inited)
            return true;
        if (is_root)
            return false;
        if (is_empty())
            return false;
        if (child_last() < 9)
            return false;
        else
        {
            if (is_endpoint)
                return true;
            else
            {
                Folder child = get_child_folder(9);
                if (!child)
                    return true;
                return child.is_full();
            }
        }
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
        if (N_layers <= layer_index)
            return;
        if (_dd == nullptr)
            return;

        // Ensuring the folder exists!
        _dd->ensure_folder_exists(path);

        strcpy(_path, path);

        // Finding len
        _child_count = load_config_file("count", true, 0);

        // Finding first
        if (!is_root)
            _child_first = 0;
        else
            _child_first = load_config_file("first", true, 0);

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

    /*Doc Folder::get_child_doc(const size_t child_number) const
    {
        if (!is_endpoint)
            return Doc();
        if (!_is_inited)
            return Doc();

        char chbuf[TE_PATH_BUF_LEN];
        child_path(chbuf, child_number);
        return Doc(chbuf, _dd);
    }*/

    bool Folder::save_backup_config_file(const char *config, const size_t val)
    {
        char confile_path[TE_PATH_BUF_LEN];
        char valstr[64];
        sprintf(confile_path, "%s/%s.teb", _path, config);
        sprintf(valstr, "%u", val);
        return _dd->write(confile_path, (unsigned char *)valstr, strlen(valstr) + 1);
    }

    bool Folder::save_config_file(const char *config, const size_t val, bool save_backup)
    {
        char confile_path[TE_PATH_BUF_LEN];
        char valstr[64];
        sprintf(confile_path, "%s/%s.tec", _path, config);
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

        sprintf(confile_path, "%s/%s.tec", _path, config);
        bool original_exists = _dd->is_file(confile_path);
        if (!original_exists)
            sprintf(confile_path, "%s/%s.teb", _path, config);

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
        return val;
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
        if (!_is_inited)
            return false;
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
};

#endif