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
    const uint32_t now = millis();
    startTimeOffset = now;
    lastSaveTime = now;
    isActive = true;
}

BlackBox::~BlackBox()
{
    if (currFile) currFile.close();
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
    if (!currFile)
    {
        // Use EEPROM to store last used number for faster "available path" finding
        int lastFileNumber = EEPROM.read(EEPROM_FILE_NUMBER_ADDRESS);
        if (lastFileNumber < 1 || lastFileNumber > 99999) lastFileNumber = 1;
        for (int i = lastFileNumber; i <= 99999; i++)
        {
            strcpy(filePath, fileDir); // replaces existing buffer values
            strcat(filePath, "/");
            char fileNumStr[6];
            itoa(i, fileNumStr, 10);
            strcat(filePath, fileNumStr);
            strcat(filePath, ".csv");
            if (!SD.exists(filePath)) {
                EEPROM.write(EEPROM_FILE_NUMBER_ADDRESS, i); // Save number to EEPROM
                return;
            }
        }
        isActive = false;
    }
}

void BlackBox::openFile()
{
    if (filePath[0] != '\0')
    {
        currFile = SD.open(filePath, FILE_WRITE_BEGIN);
        if (currFile)
        {
            currFile.println("ID,Time,Data");
            return;
        }
    }
    isActive = false;
}

void BlackBox::save()
{
    if (isActive) // File cannot be flushed - not set
    {
        // MAYBE: Determine whether to replace interval-based guard with alternative?
        // Buffer based guard
        // Brownout detection
        // Shutdown command
        if (const uint32_t now = millis(); now - lastSaveTime >= saveInterval)
        {
            currFile.flush();
            lastSaveTime = now;
        }
    }
}

void BlackBox::writeCANMsg(const CAN_message_t& canMsg)
{
    if (isActive)
    {
        //   20 total bytes for id and millis offset +
        // + 24 total bytes for all possibledata values
        // + 9 total bytes for all possible commas
        // + 1 byte for \0
        // = 54 total necessary buffer size
        char line[54];
        char idStr[11];
        ultoa(canMsg.id, idStr, 10);
        char timeStr[11];
        const uint32_t elapsedTime = millis() - startTimeOffset;
        ultoa(elapsedTime, timeStr, 10);
        strcpy(line, idStr);
        strcat(line, ",");
        strcat(line, timeStr);
        strcat(line, ",");
        for (int i = 0; i < canMsg.len; i++)
        {
            char bufStr[4];
            itoa(canMsg.buf[i], bufStr, 10);
            strcat(line, bufStr);
            if (i < canMsg.len - 1) strcat(line, ",");
        }
        currFile.println(line);
    }
}

void BlackBox::readCAN()
{
    if (isActive)
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
}
