#include <driver/sdcard/driver_sdcard.hpp>


bool SDCard::init() {return SD.begin(SD_CS);}
void SDCard::end() {SD.end();}
uint8_t SDCard:: getCardType() { return SD.cardType();}
uint64_t SDCard:: getCardSize() { return SD.cardSize()/ (1024 * 1024);}
uint64_t SDCard:: getTotalSpace() { return SD.totalBytes() / (1024 * 1024);}
uint64_t SDCard:: getUseSpace() {return SD.usedBytes() / (1024 * 1024);}

uint8_t SDCard:: listDir(const String &dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\n", dirname);

    File root = SD.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return sd_open_fail;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return sd_not_dir;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
    return sd_pass;
}

uint8_t SDCard:: createDir(const String& path){
    Serial.printf("Creating Dir: %s\n", path);
    if(SD.mkdir(path)){
        Serial.println("Dir created");
        return sd_pass;
    } else {
        Serial.println("mkdir failed");
        return sd_fail;
    }
}

uint8_t SDCard:: removeDir(const String &path){
    Serial.printf("Removing Dir: %s\n", path);
    if(SD.rmdir(path)){
        Serial.println("Dir removed");
        return sd_pass;
    } else {
        Serial.println("rmdir failed");
        return sd_fail;
    }
}

uint8_t SDCard:: renameFile(const String &path1, const String &path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (SD.rename(path1, path2)) {
        Serial.println("File renamed");
        return sd_pass;
    } else {
        Serial.println("Rename failed");
        return sd_fail;
    }
}

uint8_t SDCard:: deleteFile(const String &path){
    Serial.printf("Deleting file: %s\n", path);
    if(SD.remove(path)){
        Serial.println("File deleted");
        return sd_pass;
    } else {
        Serial.println("Delete failed");
        return sd_fail;
    }
}

uint8_t SDCard:: readFile(String path, uint8_t (*callbackFuncReadFile)(File*)){
    Serial.printf("Reading file: %s\n", path);

    File file = SD.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return sd_fail;
    }

    Serial.printf("Read from file: %s\n", file.name());
    callbackFuncReadFile(&file);
    file.close();
    return sd_pass;
}
uint8_t SDCard:: getFile(File *file, String path) {
    File f = SD.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return sd_fail;
    }
    file = &f;
    Serial.printf("Open from file: %s\n", f.name());
    return sd_pass;
}

uint8_t appendFileTxt(String path) {
    File f = SD.open(path);
    if(!f){
        Serial.println("Failed to open file for reading");
        return sd_fail;
    }
}

uint8_t ReadFile:: readFile8_t(File *file) {
    if(!file){
        Serial.println("Failed to open file for reading");
        return sd_fail;
    } else {
        return file->read();
    }
}

uint16_t ReadFile:: readFile16_t(File *file) {
    if(!file){
        Serial.println("Failed to open file for reading");
        return sd_fail;
    } else {
        uint16_t result;
        ((uint8_t *)&result)[0] = file->read(); // LSB
        ((uint8_t *)&result)[1] = file->read(); // MSB
        return result;
    }
}
uint32_t ReadFile:: readFile32_t(File *file) {
    if(!file){
        Serial.println("Failed to open file for reading");
        return sd_fail;
    } else {
        uint32_t result;
        ((uint8_t *)&result)[0] = file->read(); // LSB
        ((uint8_t *)&result)[1] = file->read();
        ((uint8_t *)&result)[2] = file->read();
        ((uint8_t *)&result)[3] = file->read(); // MSB
        return result;
    }
}
uint64_t ReadFile:: readFile64_t(File *file) {
    if(!file){
        Serial.println("Failed to open file for reading");
        return sd_fail;
    } else {
        uint64_t result;
        ((uint8_t *)&result)[0] = file->read(); // LSB
        ((uint8_t *)&result)[1] = file->read();
        ((uint8_t *)&result)[2] = file->read();
        ((uint8_t *)&result)[3] = file->read();
        ((uint8_t *)&result)[4] = file->read();
        ((uint8_t *)&result)[5] = file->read();
        ((uint8_t *)&result)[6] = file->read();
        ((uint8_t *)&result)[7] = file->read(); // MSB
        return result;
    }
}

size_t ReadFile:: readFileBuff(File *file, uint8_t *buff, size_t size) {
    if(!file){
        Serial.println("Failed to open file for reading");
        return sd_fail;
    } else {
        return file->read(buff, size);
    }
}