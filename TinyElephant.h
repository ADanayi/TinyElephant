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
        bool is_inited() const;
        operator bool() const;
        TinyOperationResult insert_pure_data_file(const unsigned char *data, size_t size, const char *extension = nullptr);
        TinyOperationResult insert(DocWriter &document);
#ifdef TE_USE_DEFAULT_BUFFERS
        TinyOperationResult insert();
#endif
        Doc fetch(tenum id, unsigned char *const buf, const size_t buf_size);
#ifdef TE_USE_DEFAULT_BUFFERS
        DefaultDoc fetch(tenum id);
#endif
        bool remove_last_docs(); // Can be used to free up space
        // TinyOperationResult replace(tenum id, DocWriter &replace_with);

        bool is_empty() const;

        // protected:
        void _path_of_to_buf(const tenum id, const char *extension = nullptr);

        // private:
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
#ifndef TE_USE_DEFAULT_BUFFERS
            unsigned char buf[64];
            DocWriter writer(buf, 64);
            writer.clean();
            writer.append_field("Tiny", "Elephant");
            writer.append_field("Version", TE_VERSION);
#else
            writer.clean();
            writer.append_field("Tiny", "Elephant");
            writer.append_field("Version", TE_VERSION);
            writer.append_field("By", "Abolfazl Danayi");
            writer.append_field("Repo", "https://github.com/adanayi/TinyElephant");
            writer.append_field("Good", "Luck");
#endif
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

    bool TinyElephant::is_empty() const
    {
        return (root.is_empty());
    }

#ifdef TE_USE_DEFAULT_BUFFERS
    TinyOperationResult TinyElephant::insert()
    {
        return insert(writer);
    }
#endif

    bool TinyElephant::remove_last_docs()
    {
        if (root.is_empty())
        {
            return false;
        }
        bool ok;
        if (root.is_endpoint)
        {
            sprintf(path_buf, "%s/%d.ted", root_path, root.child_first());
            ok = dd->remove(path_buf);
        }
        else
        {
            sprintf(path_buf, "%s/%d", root_path, root.child_first());
            ok = dd->rmtree(path_buf);
        }
        return ok && root._increase_first();
    }

    bool TinyElephant::is_inited() const
    {
        return _is_inited;
    }

    TinyElephant::operator bool() const
    {
        return is_inited();
    }

    TinyOperationResult TinyElephant::insert_pure_data_file(const unsigned char *data, size_t size, const char *extentsion)
    {
        TinyOperationResult tor;
        tor.status = false;

        //@optimization
        unsigned long long int t = dd->millis();
        _path_of_to_buf(cursor_next, extentsion);

        tor.time_loading_ms = dd->millis() - t;
        t = dd->millis();
        if (!root.commit_configs_for_inc(cursor_next))
        {
            tor.failure_reason = TinyOperationFailureReasonT::commit_error;
            return tor;
        }
        tor.time_commit_ms = dd->millis() - t;
        t = dd->millis();
        if (!dd->write(path_buf, data, size, 0))
        {
            tor.failure_reason = TinyOperationFailureReasonT::write_error;
            return tor;
        }
        tor.time_doc_io_ms = dd->millis() - t;

        tor.status = true;
        tor.failure_reason = TinyOperationFailureReasonT::success;
        tor.id = cursor_next;

        cursor_next++;

        return tor;
    }

    TinyOperationResult TinyElephant::insert(DocWriter &document)
    {
        TinyOperationResult tor;
        tor.status = false;

        sprintf(path_buf, "%llu", cursor_next);
        if (!document.append_field("_id", path_buf))
        {
            tor.failure_reason = TinyOperationFailureReasonT::writer_error;
            return tor;
        }

        //@optimization
        unsigned long long int t = dd->millis();
        _path_of_to_buf(cursor_next);
        tor.time_loading_ms = dd->millis() - t;
        t = dd->millis();
        if (!root.commit_configs_for_inc(cursor_next))
        {
            tor.failure_reason = TinyOperationFailureReasonT::commit_error;
            return tor;
        }
        tor.time_commit_ms = dd->millis() - t;
        t = dd->millis();
        if (!document.save_to_file(dd, path_buf))
        {
            tor.failure_reason = TinyOperationFailureReasonT::write_error;
            return tor;
        }
        tor.time_doc_io_ms = dd->millis() - t;

        tor.status = true;
        tor.failure_reason = TinyOperationFailureReasonT::success;
        tor.id = cursor_next;

        cursor_next++;

        return tor;
    }

#ifdef TE_USE_DEFAULT_BUFFERS
    DefaultDoc TinyElephant::fetch(tenum id)
    {
        if (id < cursor_first || id >= cursor_next)
            return DefaultDoc();
        _path_of_to_buf(id);
        return DefaultDoc(path_buf, dd);
    }
#endif

    Doc TinyElephant::fetch(tenum id, unsigned char *const buf, const size_t buf_size)
    {
        if (id < cursor_first || id >= cursor_next)
            return Doc();
        _path_of_to_buf(id);
        return Doc(path_buf, dd, buf, buf_size);
    }

    void TinyElephant::_path_of_to_buf(const tenum id, const char *extension)
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
                const char *ext;
                if (extension == nullptr)
                    ext = "ted";
                else
                    ext = extension;
                path_buf[target_ctr] = '.';
                target_ctr++;
                for (size_t j = 0; j < strlen(ext) + 1; j++)
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