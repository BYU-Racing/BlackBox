#ifndef BLACKBOX_H
#define BLACKBOX_H

#include <SD.h>
#include <EEPROM.h>
#include <FlexCAN_T4.h>

constexpr int EEPROM_FILE_NUMBER_ADDRESS = 0; // Rotate every 100,000 writes to address

class BlackBox
{
public:
    ~BlackBox();
    explicit operator bool() const { return isActive; }
    void begin(
        FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16>* data_CAN,
        FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16>* motor_CAN,
        int save_Interval
    );
    void save();
    void writeCANMsg(const CAN_message_t& canMsg);
    void readCAN();
private:
    bool isActive = false;

    // CAN Lines
    FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16>* dataCAN = nullptr;
    FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16>* motorCAN = nullptr;

    // Saving interval
    uint32_t saveInterval = 0;
    uint32_t startTimeOffset = 0;
    uint32_t lastSaveTime = 0;

    // File interaction
    const char* fileDir = "/data/";
    char filePath[20] = "";
    File currFile;
    void beginSD();
    void setFilePath();
    void openFile();
};

#endif //BLACKBOX_H
