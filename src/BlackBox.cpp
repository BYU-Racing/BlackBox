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
            snprintf(filePath, 20, "%s/%05d.csv", fileDir, i);
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
        const uint32_t elapsedTime = millis() - startTimeOffset;
        int lineOffset = snprintf(line, 54, "%lu,%lu", canMsg.id, elapsedTime);
        for (int i = 0; i < canMsg.len; i++)
        {
            lineOffset += snprintf(line + lineOffset, 54, i < canMsg.len - 1 ? "%u" : "%u,", canMsg.buf[i]);
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
