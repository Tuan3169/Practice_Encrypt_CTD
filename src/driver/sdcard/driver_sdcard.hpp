#ifndef SDCARD_H
#define SDCARD_H

#include <SD.h>
#include <FS.h>
#include <SPI.h>
#include <config.hpp>

typedef enum {
    sd_fail,
    sd_pass,
    sd_card_mount_fail,
    sd_open_fail,
    sd_not_dir
} sdcard_log;

class SDCard {
private:

public:
    SDCard() {
        // SPI.begin(SD_SCK,SD_MISO,SD_MOSI);
        
    }
    ~SDCard() {
        SD.end();
        // SPI.end();
    }

    bool init();
    void end();
    uint8_t getCardType();
    uint64_t getCardSize();
    uint64_t getTotalSpace();
    uint64_t getUseSpace();

    uint8_t listDir(const String &dirname, uint8_t levels = 0);
    uint8_t createDir(const String& path);
    uint8_t removeDir(const String &path);
    uint8_t renameFile(const String &path1, const String &path2);
    uint8_t deleteFile(const String &path);
    uint8_t readFile(String path, uint8_t (*callbackFuncReadFile)(File*));
    uint8_t getFile(File *file, String path);
    uint8_t appendFileTxt(const String &path, String data);
};

class ReadFile {
public:
    static uint8_t readFile8_t(File *file);
    static uint16_t readFile16_t(File *file);
    static uint32_t readFile32_t(File *file);
    static uint64_t readFile64_t(File *file);
    static size_t readFileBuff(File *file, uint8_t *buff, size_t size);
};

#endif