#ifndef BLACKBOX_H
#define BLACKBOX_H

#include "FileManager.h"

class BlackBox
{
public:
    BlackBox() :
        fileManager(nullptr), motorCAN(nullptr), dataCAN(nullptr),
        startTimeOffset(millis()), lastSaveTime(0), saveInterval(0)
    {  }
    ~BlackBox();
    explicit operator bool() const;
    void init(
        FileManager* file_Manager,
        FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16>* motor_CAN,
        FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16>* data_CAN,
        int save_Interval
    );
    void startup();
    void shutdown();
    void CANMsgHandler(const CAN_message_t& canMsg) const;
    void readCAN() const;
    void readSerial() const;
    void checkSave();
    void log(const String& msg) const;
    bool shutdownRequested = false;
    bool verbose = true;
private:
    FileManager* fileManager;
    FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16>* motorCAN;
    FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16>* dataCAN;
    uint32_t startTimeOffset;
    uint32_t lastSaveTime;
    uint32_t saveInterval;
    bool loggingAvailable = false;
};

#endif //BLACKBOX_H
