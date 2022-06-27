// In the name of Allah

#ifndef __SD_DRIVER_B_H
#define __SD_DRIVER_B_H

#include "DiskDriverBase.h"
#include "SD_MMC.h"
#include "SPI.h"
#include "Arduino.h"
#include "Esp.h"

#define ESP32SD_PATH_STR_BUF_SIZE 128
#define ESP32SD_MOUNT_POINT "/sdcard"

namespace tinyelephant
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

        virtual bool _read(const char *path, unsigned char *buf, const size_t max_buf_len){
            
        };

        virtual bool _write(const char *path, const unsigned char *buf, const size_t buf_len){

        };
    };
};
#endif