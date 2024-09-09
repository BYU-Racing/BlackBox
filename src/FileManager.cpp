#include "FileManager.h"

void FileManager::init(TimeKeeper* time_Keeper)
{
    timeKeeper = time_Keeper;
    startTimeOffset = millis();
}

FileManager::~FileManager()
{
    shutdown();
}

FileManager::operator bool() const { return isActive; }

void FileManager::startup()
{
    if (!SD.begin(BUILTIN_SDCARD))
    {
        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] SD Card Failed Initialization"));
        isActive = false;
        return;
    }
    if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] SD Card Initialized"));

    isActive = true;

    if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Initializing Log File"));
    setLogFile();

    logMsg(String("[INFO][FileManager][" + String(millis()) + "] Log File Initialized: " + logFilePath));

    if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Initializing Data File"));
    setDataFile();

    logMsg(String("[INFO][FileManager][" + String(millis()) + "] Data File Initialized: " + dataFilePath));
}

void FileManager::shutdown()
{
    logMsg(String("[INFO][FileManager][" + String(millis()) + "] Shutdown Sequence Triggered"));
    isActive = false;

    closeFiles();
    if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Closed All Active File Resources"));
}

void FileManager::closeFiles()
{
    if (logFile)
    {
        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Closing Log File"));
        logFile.close();
    }
    if (dataFile)
    {
        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Closing Data File"));
        dataFile.close();
    }
}

void FileManager::saveFiles()
{
    if (isActive)
    {
        if (logFile)
        {
            if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Saving Log File"));
            logFile.close();
            logFile = SD.open(logFilePath.c_str(), FILE_WRITE);
        }
        if (dataFile)
        {
            if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Saving Data File"));
            dataFile.close();
            dataFile = SD.open(dataFilePath.c_str(), FILE_WRITE);
        }
    } else if (verbose) Serial.println(String("[WARNING][FileManager][" + String(millis()) + "] FileManager Not Active"));
}

String FileManager::getAvailableFilePath(
    const String& dir, const String& name, const String& ext
) const
{
    if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Getting File Path - Pattern: " + dir + name + ext));
    if (!SD.exists(dir.c_str()))
    {
        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Making Directory: " + dir));
        SD.mkdir(dir.c_str());
    }
    String tempPath = dir + name + ext;
    for (int i = 1; SD.exists(tempPath.c_str()); i++)
    {
        tempPath = dir + name + "_" + i + ext;
    }
    if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] File Path: " + tempPath));
    return tempPath;
}

void FileManager::setLogFile()
{
    const String name = timeKeeper->isActive() ? timeKeeper->getDateTime() : "logFile";

    if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Getting Best Log File Path"));
    logFilePath = getAvailableFilePath("/logs/", name, ".log");

    if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Opening Log File"));
    logFile = SD.open(logFilePath.c_str(), FILE_WRITE);
    if (!logFile)
    {
        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Log File Setting Failed"));
        isActive = false;
    }
}

void FileManager::setDataFile()
{
    const String name = timeKeeper->isActive() ? timeKeeper->getDateTime() : "dataFile";

    if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Getting Best Data File Path"));
    dataFilePath = getAvailableFilePath("/data/", name, ".csv");

    if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Opening Data File"));
    dataFile = SD.open(dataFilePath.c_str(), FILE_WRITE);
    if (!dataFile)
    {
        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Data File Setting Failed"));
        isActive = false;
    } else
    {
        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Writing Data File Headers"));
        dataFile.println("ID,Time,Data");
    }
}

void FileManager::logData(const SensorData& sensorData)
{
    if (isActive)
    {
        if (!dataFile)
        {
            if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Resetting Data File"));
            setDataFile();
        }

        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Writing Data Header Values"));
        dataFile.print(sensorData.getId());
        dataFile.print(",");

        dataFile.print(millis() - startTimeOffset);
        dataFile.print(",");

        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Extracting and Printing sensorValues"));
        const int* sensorValues = sensorData.getData();

        for (int i = 0; i < sensorData.length(); i++)
        {
            dataFile.print(sensorValues[i]);

            if (i < sensorData.length() - 1)
            {
                dataFile.print(",");
            }
        }

        dataFile.println();
    } else if (verbose) Serial.println(String("[WARNING][FileManager][" + String(millis()) + "] FileManager Not Active"));
}

void FileManager::logMsg(const String& logMsg)
{
    if (isActive)
    {
        if (!logFile)
        {
            if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Resetting Log File"));
            setLogFile();
        }
        logFile.println(logMsg.c_str());
    } else if (verbose) Serial.println(String("[WARNING][FileManager][" + String(millis()) + "] FileManager Not Active"));
}

void FileManager::printFile(const String& filePath) const
{
    if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Printing File: " + filePath));
    File file = SD.open(filePath.c_str(), FILE_READ);

    Serial.println("{START_FILE}");
    while (file.available())
    {
        String line = file.readStringUntil('\n');
        Serial.println(line);
    }
    file.close();
    Serial.println("{END_FILE}");
}

void FileManager::listDirectory(const String& directory) const
{
    if (SD.exists(directory.c_str()))
    {
        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Opening Directory: " + directory));
        File dir = SD.open(directory.c_str());
        Serial.println("{START_LIST}");
        while (true)
        {
            if (File entry = dir.openNextFile(); !entry)
            {
                break;
            } else
            {
                Serial.println(entry.name());
            }
        }
        Serial.println("{END_LIST}");
        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Closing Directory: " + directory));
        dir.close();
    }
}

void FileManager::logFileRequest()
{
    if (isActive)
    {
        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Closing logFile"));
        logFile.close();

        printFile(logFilePath);

        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Reopening logFile"));
        logFile = SD.open(logFilePath.c_str(), FILE_WRITE);
    } else if (verbose) Serial.println(String("[WARNING][FileManager][" + String(millis()) + "] FileManager Not Active"));
}

void FileManager::dataFileRequest()
{
    if (isActive)
    {
        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Closing dataFile"));
        dataFile.close();

        printFile(dataFilePath);

        if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Reopening dataFile"));
        dataFile = SD.open(dataFilePath.c_str(), FILE_WRITE);
    } else if (verbose) Serial.println(String("[WARNING][FileManager][" + String(millis()) + "] FileManager Not Active"));
}

void FileManager::filePathRequest(const String& filePath)
{
    if (isActive)
    {
        if (filePath == logFilePath)
        {
            if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Requested logFile"));
            logFileRequest();
        } else if (filePath == dataFilePath)
        {
            if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Requested dataFile"));
            dataFileRequest();
        } else if (SD.exists(filePath.c_str()))
        {
            if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Requested Existing File"));
            printFile(filePath);
        } else
        {
            if (verbose) Serial.println(String("[DEBUG][FileManager][" + String(millis()) + "] Requested Non-existing File"));
            Serial.println("{FILE_NOT_FOUND}");
        }
    } else if (verbose) Serial.println(String("[WARNING][FileManager][" + String(millis()) + "] FileManager Not Active"));
}
