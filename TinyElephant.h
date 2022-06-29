// In the name of Allah

#ifndef __FDB_H
#define __FDB_H

#include "DiskDriver/DiskDriverBase.h"
#include "Classes/_Folder.h"
#include "setup.h"

namespace elephant
{
    class TinyElephant
    {
    public:
        explicit TinyElephant(
            DiskDriverBase *diskdriver, const char *root_path, const size_t N_layers);
        explicit TinyElephant(TinyElephant &) = delete;
        explicit TinyElephant() = delete;
        bool is_inited();
        operator bool();
        TinyOperationResult insert(DocWriter &document);
        Doc fetch(tenum id, unsigned char *const buf, const size_t buf_size);
        Doc fetch_default_buf(tenum id);
        // TinyOperationResult replace(tenum id, DocWriter &replace_with);

    protected:
        void _path_of_to_buf(const tenum id);

    private:
        bool _inited;
        DiskDriverBase *const dd;
        const char *const root_path;
        const size_t N_layers;
        Folder root;
        char path_buf[TE_PATH_BUF_LEN];
        bool _is_inited;
        tenum cursor_first;
        tenum cursor_next;
    };
};

namespace elephant
{
    TinyElephant::TinyElephant(
        DiskDriverBase *diskdriver,
        const char *root_path,
        const size_t N_layers) : dd(diskdriver),
                                 root_path(root_path),
                                 N_layers(N_layers),
                                 root(root_path, N_layers, 0, diskdriver)
    {
        _is_inited = false;

        strcpy(path_buf, root_path);

        if (!dd->init())
            return;

        cursor_first = root.calc_first_doc_sub_id();

        if (root.is_empty())
        {
            cursor_next = 0;
            writer.clean();
            writer.append_field("Tiny", "Elephant");
            writer.append_field("Version", TE_VERSION);
            writer.append_field("By", "Abolfazl Danayi");
            writer.append_field("Repo", "https://github.com/adanayi/TinyElephant");
            writer.append_field("Good", "Luck");
            if (!insert(writer))
            {
                return;
            }
        }
        else
        {
            cursor_next = root.calc_last_doc_sub_id() + 1;
        }

        _is_inited = true;
    }

    bool TinyElephant::is_inited()
    {
        return _is_inited;
    }

    TinyElephant::operator bool()
    {
        return is_inited();
    }

    TinyOperationResult TinyElephant::insert(DocWriter &document)
    {
        TinyOperationResult tor;
        tor.status = false;

        sprintf(path_buf, "%llu", cursor_next);
        if (!document.append_field("_id", path_buf))
        {
            return tor;
        }
        _path_of_to_buf(cursor_next);
        if (!root.commit_configs_for_inc(cursor_next))
        {
            return tor;
        }
        if (!document.save_to_file(dd, path_buf))
        {
            return tor;
        }

        tor.status = true;
        tor.id = cursor_next;

        cursor_next++;

        return tor;
    }

    // Doc TinyElephant::fetch_default_buf(tenum id)
    // {
    //     if (id < cursor_first || id >= cursor_next)
    //         return Doc();
    //     _path_of_to_buf(id);
    //     return DefaultDoc(path_buf, dd);
    // }

    Doc TinyElephant::fetch(tenum id, unsigned char *const buf, const size_t buf_size)
    {
        if (id < cursor_first || id >= cursor_next)
            return Doc();
        _path_of_to_buf(id);
        return Doc(path_buf, dd, buf, buf_size);
    }

    void TinyElephant::_path_of_to_buf(const tenum id)
    {
        strcpy(path_buf, root_path);
        char idstr[TE_PATH_BUF_LEN];
        sprintf(idstr, "%llu", id);
        size_t L = strlen(idstr);
        size_t target_ctr = strlen(path_buf);
        for (size_t l = 0; l < N_layers; l++)
        {
            size_t s;
            path_buf[target_ctr] = '/';
            target_ctr++;
            if (L < N_layers - l)
            {
                path_buf[target_ctr] = '0';
                target_ctr++;
            }
            else
            {
                if (l == 0)
                {
                    size_t j;
                    for (j = 0; j <= L - N_layers; j++)
                    {
                        path_buf[target_ctr] = idstr[j];
                        target_ctr++;
                    }
                }
                else
                {
                    path_buf[target_ctr] = idstr[L - N_layers + l];
                    target_ctr++;
                }
            }
            if (l == N_layers - 1)
            {
                const char *ext = ".ted";
                for (size_t j = 0; j < 4; j++)
                {
                    path_buf[target_ctr] = ext[j];
                    target_ctr++;
                }
            }
        }
        path_buf[target_ctr] = '\0';
    }
}

#endif