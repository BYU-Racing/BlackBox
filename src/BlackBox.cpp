#include "BlackBox.h"

void BlackBox::init(
    FileManager* file_Manager,
    FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16>* motor_CAN,
    FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16>* data_CAN,
    const int save_Interval
)
{
    fileManager = file_Manager;
    motorCAN = motor_CAN;
    dataCAN = data_CAN;
    saveInterval = save_Interval;
}

BlackBox::~BlackBox()
{
    shutdown();
}

BlackBox::operator bool() const { return loggingAvailable; }


void BlackBox::startup()
{
    fileManager->startup();
    loggingAvailable = true;
    log(String("[INFO][BlackBox][" + String(millis()) + "] Startup Completed - Logging Enabled"));
}

void BlackBox::shutdown()
{
    log(String("[INFO][BlackBox][" + String(millis()) + "] Shutdown Triggered"));
    shutdownRequested = true;
    fileManager->shutdown();
}

void BlackBox::CANMsgHandler(const CAN_message_t& canMsg) const
{
    if (verbose) Serial.println(String("[DEBUG][BlackBox][" + String(millis()) + "] Handling CAN Message"));

    if (verbose) Serial.println(String("[DEBUG][BlackBox][" + String(millis()) + "] Extracting SensorData from canMsg"));
    const SensorData sensorMsg(canMsg);

    if (verbose) Serial.println(String("[DEBUG][BlackBox][" + String(millis()) + "] Logging Sensor Data"));
    fileManager->logData(sensorMsg);
}

void BlackBox::readCAN() const
{
    if (CAN_message_t canMsg; dataCAN->read(canMsg))
    {
        if (verbose) Serial.println(String("[DEBUG][BlackBox][" + String(millis()) + "] Data CAN Message Received"));
        CANMsgHandler(canMsg);
    }
    if (CAN_message_t canMsg; motorCAN->read(canMsg))
    {
        if (verbose) Serial.println(String("[DEBUG][BlackBox][" + String(millis()) + "] Motor CAN Message Received"));
        CANMsgHandler(canMsg);
    }
}

void BlackBox::readSerial() const
{
    if (Serial.available())
    {
        if (verbose) Serial.println(String("[DEBUG][BlackBox][" + String(millis()) + "] Serial Message Received"));
        if (const String msg = Serial.readStringUntil('\n'); msg == String("{CURRLOG}"))
        {
            log(String("[INFO][BlackBox][" + String(millis()) + "] Received Current Logfile Request"));
            fileManager->logFileRequest();
        } else if (msg == String("{CURRDATA}"))
        {
            log(String("[INFO][BlackBox][" + String(millis()) + "] Received Current Datafile Request"));
            fileManager->dataFileRequest();
        } else if (msg.startsWith("{GETFILE=") && msg.endsWith("}"))
        {
            if (verbose) Serial.println(String("[DEBUG][BlackBox][" + String(millis()) + "] Extracting Filepath from request"));
            const String filePath = msg.substring(String("{GETFILE=").length(), msg.length() - 1);

            log(String("[INFO][BlackBox][" + String(millis()) + "] Received Request for File: " + filePath));
            fileManager->filePathRequest(filePath);
        } else if (msg == String("{LISTLOGS}"))
        {
            log(String("[INFO][BlackBox][" + String(millis()) + "] Received List Log Files Request"));
            fileManager->listDirectory("/logs");
        } else if (msg == String("{LISTDATAS}"))
        {
            log(String("[INFO][BlackBox][" + String(millis()) + "] Received List Data Files Request"));
            fileManager->listDirectory("/data");
        } else
        {
            log(String("[INFO][BlackBox][" + String(millis()) + "] Received Unknown Serial Input: `" + msg + "`"));
            Serial.println("Command Not Found. Available Commands:");
            Serial.println("{CURRLOG} -> Get the contents of the current active log file (.log)");
            Serial.println("{CURRDATA} -> Get the contents of the current active data file (.csv)");
            Serial.println("{LISTLOGS} -> Get all file names of the log directory");
            Serial.println("{LISTDATAS} -> Get all file names of the data directory");
            Serial.println("{GETFILE=?} -> Get the contents of a file at a Unix-like location (i.e. /log/logFile.log)");
        }
    }
}

void BlackBox::checkSave()
{
    if (millis() - lastSaveTime > saveInterval)
    {
        if (verbose) Serial.println(String("[DEBUG][BlackBox][" + String(millis()) + "] Saving Files"));
        fileManager->saveFiles();

        if (verbose) Serial.println(String("[DEBUG][BlackBox][" + String(millis()) + "] Resetting Save Interval"));
        lastSaveTime = millis();
    }
}

void BlackBox::log(const String& msg) const
{
    if (!loggingAvailable)
    {
        if (verbose) Serial.println(String("[DEBUG][BlackBox][" + String(millis()) + "] Logging Not Available for Message: " + msg));
        return;
    }
    if (verbose) Serial.println(msg);
    fileManager->logMsg(msg);
}
