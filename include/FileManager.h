#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <SD.h>

#include "SensorData.h"
#include "TimeKeeper.h"

class FileManager
{
public:
    FileManager() : timeKeeper(nullptr), startTimeOffset(-1) {  }
    ~FileManager();
    void init(TimeKeeper* time_Keeper);
    explicit operator bool() const;
    void startup();
    void shutdown();
    void logData(const SensorData& sensorData);
    void logMsg(const String& logMsg);
    void closeFiles();
    void saveFiles();
    void logFileRequest();
    void dataFileRequest();
    void filePathRequest(const String& filePath);
    void listDirectory(const String& directory) const;
    bool verbose = true;
private:
    bool isActive = false;
    TimeKeeper* timeKeeper;
    String logFilePath;
    File logFile;
    String dataFilePath;
    File dataFile;
    uint32_t startTimeOffset;
    String getAvailableFilePath(const String& dir, const String& name, const String& ext) const;
    void setLogFile();
    void setDataFile();
    void printFile(const String& filePath) const;
};

#endif //FILEMANAGER_H
