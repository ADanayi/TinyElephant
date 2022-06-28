// In the name of Allah

#ifndef __SD_DRIVER_B_H
#define __SD_DRIVER_B_H

#include "DiskDriverBase.h"
#include "SD_MMC.h"
#include "SPI.h"
#include "Arduino.h"
#include "Esp.h"
#include "FS.h"

#define ESP32SD_PATH_STR_BUF_SIZE 128
#define ESP32SD_MOUNT_POINT "/sdcard"

namespace elephant
{
    class ESP32DiskDriver : public DiskDriverBase
    {
    public:
        explicit ESP32DiskDriver() : DiskDriverBase(this->path_str_buf, (size_t)ESP32SD_PATH_STR_BUF_SIZE, ""){};
        explicit ESP32DiskDriver(ESP32DiskDriver &) = delete;

    private:
        char path_str_buf[ESP32SD_PATH_STR_BUF_SIZE];

    protected:
        virtual bool _init() override
        {
            Serial.println("ESP32SD: initializing...");
            Serial.flush();
            delay(10);
            // Initialize the SD card
            if (!SD_MMC.begin(ESP32SD_MOUNT_POINT, true))
            {
                Serial.println("Failed to mount SD card");
                return false;
            }

            // Check for an SD card
            uint8_t cardType = SD_MMC.cardType();
            if (cardType == sdcard_type_t::CARD_NONE)
            {
                Serial.println("No SD card attached");
                return false;
            }
            else
            {
                Serial.printf("SD Card type: %d\n", (int)cardType);
            }
            return true;
        };

        virtual unsigned long int _disk_capacity_KB() override
        {
            return SD_MMC.totalBytes() / 1024;
        }

        virtual unsigned long int _disk_used_KB() override
        {
            return (SD_MMC.usedBytes() / 1024);
        }

        virtual bool _exists(const char *path) override
        {
            return SD_MMC.exists(path);
        }

        virtual bool _mkdir(const char *path) override
        {
            if (!SD_MMC.mkdir(path))
            {
                Serial.printf("ESP32SD Warning: Could not mkdir %s\n", path);
                return false;
            }
            return true;
        }

        virtual bool _rmdir(const char *path) override
        {
            if (!SD_MMC.rmdir(path))
            {
                Serial.printf("ESP32SD Warning: Could not rmdir %s\n", path);
                return false;
            }
            return true;
        }

        virtual bool _remove(const char *path) override
        {
            if (!SD_MMC.remove(path))
            {
                Serial.printf("ESP32SD Warning: Could not remove %s\n", path);
                return false;
            }
            return true;
        }

        virtual bool _rename(const char *old_path, const char *new_path) override
        {
            if (!SD_MMC.rename(old_path, new_path))
            {
                Serial.printf("ESP32SD Warning: Could not rename %s to %s\n", new_path, old_path);
                return false;
            }
            return true;
        }

        virtual bool _read(const char *path, unsigned char *buf, size_t &len, const size_t max_buf_len) override
        {
            File f = SD_MMC.open(path, FILE_READ);
            if (!f) {
                Serial.printf("ESP32SD Warning: @read: could not open file %s\n", path);
            }
            size_t i = 0;
            len = f.size();
            if (len > max_buf_len)
            {
                Serial.printf("ESP32SD Warning: @read: file size: %lu, buf size: %lu, file: %s\n", len, max_buf_len, path);
                f.close();
                return false;
            }
            f.read(buf, len);
            f.close();
            return true;
        };

        virtual bool _write(const char *path, const unsigned char *buf, const size_t buf_len) override
        {
            File f = SD_MMC.open(path, FILE_WRITE);
            size_t l = f.write(buf, buf_len);
            f.close();
            return l == buf_len;
        };

        virtual bool _is_dir(const char *path) override
        {
            File f = SD_MMC.open(path, FILE_READ);
            bool is_dir = f.isDirectory();
            f.close();
            return is_dir;
        };

        virtual bool _rmtree(const char *path) override
        {
            char parent_name[128];
            strcpy(parent_name, path);
            // Serial.printf("rmtree %s\n", parent_name);
            File parent = SD_MMC.open(parent_name, FILE_READ);
            while (1)
            {
                File child = parent.openNextFile(FILE_READ);
                if (!child)
                    break;
                char fname[128];
                strcpy(fname, child.name());
                // bool f_is_dir = false;//child.isDirectory();
                bool f_is_dir = child.isDirectory();
                child.close();
                // Serial.printf("\tinner %s, is_dir=%d\n", fname, f_is_dir);
                if (!f_is_dir)
                {
                    if (!this->_remove(fname))
                    {
                        return false;
                    }
                }
                else
                {
                    if (!_rmtree(fname))
                        return false;
                }
            };
            // Serial.printf("rmdir %s\n", parent_name);
            return _rmdir(parent_name);
        };
    };
};
#endif