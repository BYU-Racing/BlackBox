#include "BlackBox.h"

void BlackBox::begin(
    FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16>* data_CAN,
    FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16>* motor_CAN,
    const int save_Interval
)
{
    dataCAN = data_CAN;
    motorCAN = motor_CAN;
    saveInterval = save_Interval;
    beginSD();
    openFile();
    startTimeOffset = millis();
    lastSaveTime = millis();
    isActive = true;
}

BlackBox::~BlackBox()
{
    if (currFile) currFile.close(); // close open resources
}

void BlackBox::beginSD()
{
    if (!SD.begin(BUILTIN_SDCARD))
    {
        isActive = false;
        return;
    }
    if (!SD.exists(fileDir)) SD.mkdir(fileDir);
    setFilePath();
}


void BlackBox::setFilePath()
{
    if (!currFile) // Only set the file path if a file is not already specified
    {
        for (int i = 1; i <= 99999; i++)
        {
            snprintf(filePath, 20, "%s/%05d.csv", fileDir, i);
            if (!SD.exists(filePath)) return; // filename is available - return to keep filePath
        }
        isActive = false; // no more available filenames - file not set
    }
}

void BlackBox::openFile()
{
    if (filePath[0] != '\0') // filePath must have a value
    {
        currFile = SD.open(filePath, FILE_WRITE_BEGIN);
        if (!currFile)
        {
            isActive = false;
            return;
        } else
        {
            currFile.println("ID,Time,Data"); // Write csv header
            return;
        }
    }
    isActive = false; // filePath was unavailable - file not opened
}

void BlackBox::save()
{
    if (isActive) // File cannot be flushed - not set
    {
        if (millis() - lastSaveTime > saveInterval) // Interval-based guard
        {
            currFile.flush(); // Force clear remaining write buffer
            lastSaveTime = millis();
        }
    }
}

void BlackBox::writeCANMsg(const CAN_message_t& canMsg)
{
    if (isActive) // File cannot be written to - not set
    {
        currFile.print(canMsg.id);
        currFile.print(",");
        currFile.print(millis() - startTimeOffset);
        currFile.print(",");
        for (int i = 0; i < canMsg.len; i++)
        {
            currFile.print(canMsg.buf[i]);
            if (i < canMsg.len - 1) currFile.print(",");
        }
        currFile.println();
    }
}

void BlackBox::readCAN()
{
    if (CAN_message_t canMsg; dataCAN->read(canMsg))
    {
        writeCANMsg(canMsg);
    }
    if (CAN_message_t canMsg; motorCAN->read(canMsg))
    {
        writeCANMsg(canMsg);
    }
}
