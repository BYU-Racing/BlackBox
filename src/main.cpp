// INCLUDES


// Arduino Builtin Libraries
#include <Arduino.h>

// External Libraries
#include <FlexCAN_T4.h>

// Custom classes
#include "Timekeeper.h"
#include "FileManager.h"
#include "BlackBox.h"


// CONSTANTS


// Baud Rates
/** Value for the CAN Bus Baud Rate */
constexpr int CAN_BAUD_RATE = 250000;
/** Value for the Serial Port Baud Rate */
constexpr int SERIAL_BAUD_RATE = 9600;
// Timekeeping
/** Flag for whether or not the RTC chip is enabled and usable as a time sync provider */
constexpr bool USE_RTC_TIME = false;
/** Flag for whether or not the GPS chip is enabled and usable as a time sync provider */
constexpr bool USE_GPS_TIME = false;
// Intervals
/**
 * Sync rate (ms) to use for synchronizing the timekeeper to a time provider.
 *
 * Recommended value for RTC chip:  1,800,000   (30 minutes)
 * <br>
 * Recommended value for GPS chip:  300,000     (5 minutes)
 */
constexpr int USE_SYNC_INTERVAL = 300000;
/**
 * Save rate (ms) to use for saving SD card files.
 *
 * Recommended value: 5,000     (5 seconds)
 */
constexpr uint32_t SAVE_INTERVAL = 30000;
// CAN Bus
/** Flag for whether or not to use the new CAN handling style (multi-threaded callbacks) */
constexpr bool USE_CAN_MAILBOXES = false;
/** Number of maximum mailboxes for a single CAN line */
constexpr int CAN_MAILBOX_SIZE = 8;
// Logging
/** Flag for whether or not to print all MAIN debug messages */
constexpr bool MAIN_VERBOSE = true;
/** Flag for whether or not to print all FileManager debug messages */
constexpr bool FILEMANAGER_VERBOSE = true;
/** Flag for whether or not to print all BlackBox debug messages */
constexpr bool BLACKBOX_VERBOSE = true;
/** Flag for whether or not to use debug flashes (on delay) */
constexpr bool DO_FLASHY = MAIN_VERBOSE || FILEMANAGER_VERBOSE || BLACKBOX_VERBOSE;


// GLOBALS


TimeKeeper timeKeeper;
FileManager fileManager;
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> dataCAN;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> motorCAN;
BlackBox blackBox;


// FUNCTIONS


/** @returns The current configuration settings in JSON-like format */
String blackBoxConfig()
{
    const String msgStart = String("[INFO][MAIN][" + String(millis()) + "] Current Config: {");
    const String msgCAN_BAUD_RATE = "CAN_BAUD_RATE:" + String(CAN_BAUD_RATE);
    const String msgSERIAL_BAUD_RATE = ",SERIAL_BAUD_RATE:" + String(SERIAL_BAUD_RATE);
    const String msgUSE_RTC_TIME = ",USE_RTC_TIME:" + String(USE_RTC_TIME);
    const String msgUSE_GPS_TIME = ",USE_GPS_TIME:" + String(USE_GPS_TIME);
    const String msgUSE_SYNC_INTERVAL = ",USE_SYNC_INTERVAL:" + String(USE_SYNC_INTERVAL);
    const String msgSAVE_INTERVAL = ",SAVE_INTERVAL:" + String(SAVE_INTERVAL);
    const String msgUSE_CAN_MAILBOXES = ",USE_CAN_MAILBOXES:" + String(USE_CAN_MAILBOXES);
    const String msgCAN_MAILBOX_SIZE = ",CAN_MAILBOX_SIZE:" + String(CAN_MAILBOX_SIZE);
    const String msgMAIN_VERBOSE = ",MAIN_VERBOSE:" + String(MAIN_VERBOSE);
    const String msgFILEMANAGER_VERBOSE = ",FILEMANAGER_VERBOSE:" + String(FILEMANAGER_VERBOSE);
    const String msgBLACKBOX_VERBOSE = ",BLACKBOX_VERBOSE:" + String(BLACKBOX_VERBOSE);
    const String msgEnd = "}";
    const String msg = String(
        msgStart + msgCAN_BAUD_RATE + msgSERIAL_BAUD_RATE + msgUSE_RTC_TIME + msgUSE_GPS_TIME
        + msgUSE_SYNC_INTERVAL + msgSAVE_INTERVAL + msgUSE_CAN_MAILBOXES + msgMAIN_VERBOSE + msgFILEMANAGER_VERBOSE
        + msgBLACKBOX_VERBOSE + msgEnd
    );
    return msg;
}


/**
 * @brief Callback handler to be given to a FlexCAN_T4 bus for calling on incoming CAN_message_t messages.
 *  This handler *must* be defined in main.cpp to avoid receiving a `this` pointer from a class.
 */
void canMsgCallback(const CAN_message_t& canMsg)
{
    blackBox.log("[INFO][MAIN][" + String(millis()) + "] Mailbox Received CAN Message");

    if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Forwarding CAN_message_t to blackBox Handler");
    blackBox.CANMsgHandler(canMsg);
}


/** @brief Flashes the Teensy 4.1 builtin LED. *MUST* be initialized before using */
void flash(const int msec)
{
    if constexpr (DO_FLASHY)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(msec);
        digitalWrite(LED_BUILTIN, LOW);
        delay(msec);
    }
}


/** @brief Sets global variable members with values defined in main.cpp (i.e. blackBox set with fileManager pointer) */
void init()
{
    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Initializing timekeeper");
    timeKeeper.init(USE_RTC_TIME, USE_GPS_TIME, USE_SYNC_INTERVAL);

    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Initializing fileManager");
    fileManager.init(&timeKeeper);

    fileManager.verbose = FILEMANAGER_VERBOSE;

    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Initializing blackBox");
    blackBox.init(&fileManager, &motorCAN, &dataCAN, SAVE_INTERVAL);

    blackBox.verbose = BLACKBOX_VERBOSE;

    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println(String("[DEBUG][MAIN][" + String(millis()) + "] Starting blackBox"));
    blackBox.startup();

    if (CrashReport)
    {
        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println(String("[ERROR][MAIN][" + String(millis()) + "] Crash Detected:\n" + String(CrashReport)));
        blackBox.log(String("[ERROR][MAIN][" + String(millis()) + "] Crash Detected:\n" + String(CrashReport)));
        flash(1000);
        flash(1000);
        flash(1000);
        flash(1000);
        flash(1000);
    }

    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println(blackBoxConfig());
    blackBox.log(blackBoxConfig());
}


/** @brief Teensy 4.1 resources and pins (i.e. Serial) *must* be set and began here before use elsewhere. */
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(SERIAL_BAUD_RATE);

    flash(100);
    if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Serial Ready");

    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Initializing Objects");
    init();

    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Starting motorCAN");
    blackBox.log("[INFO][MAIN][" + String(millis()) + "] Starting motorCAN");
    motorCAN.begin();

    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Setting motorCAN BaudRate: " + CAN_BAUD_RATE);
    blackBox.log("[INFO][MAIN][" + String(millis()) + "] Setting motorCAN BaudRate: " + CAN_BAUD_RATE);
    motorCAN.setBaudRate(CAN_BAUD_RATE);

    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Starting dataCAN");
    blackBox.log("[INFO][MAIN][" + String(millis()) + "] Starting dataCAN");
    dataCAN.begin();

    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Setting dataCAN BaudRate: " + CAN_BAUD_RATE);
    blackBox.log("[INFO][MAIN][" + String(millis()) + "] Setting dataCAN BaudRate: " + CAN_BAUD_RATE);
    dataCAN.setBaudRate(CAN_BAUD_RATE);

    if (USE_CAN_MAILBOXES)
    {
        // motorCAN Setup
        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Setting motorCAN Maximum Mailboxes: " + CAN_MAILBOX_SIZE);
        blackBox.log("[INFO][MAIN][" + String(millis()) + "] Setting motorCAN Maximum Mailboxes: " + CAN_MAILBOX_SIZE);
        motorCAN.setMaxMB(CAN_MAILBOX_SIZE);

        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Enabling motorCAN FIFO Mailboxes");
        blackBox.log("[INFO][MAIN][" + String(millis()) + "] Enabling motorCAN FIFO Mailboxes");
        motorCAN.enableFIFO();

        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Enable motorCAN FIFO Mailbox Interrupts");
        blackBox.log("[INFO][MAIN][" + String(millis()) + "] Enable motorCAN FIFO Mailbox Interrupts");
        motorCAN.enableFIFOInterrupt();

        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Setting motorCAN Mailbox Handler");
        blackBox.log("[INFO][MAIN][" + String(millis()) + "] Setting motorCAN Mailbox Handler");
        motorCAN.onReceive(canMsgCallback);

        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Enabling motorCAN Mailbox Distribution");
        blackBox.log("[INFO][MAIN][" + String(millis()) + "] Enabling motorCAN Mailbox Distribution");
        motorCAN.distribute();

        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Printing motorCAN Mailbox Status");
        motorCAN.mailboxStatus();

        // dataCAN Setup
        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Setting dataCAN Maximum Mailboxes: " + CAN_MAILBOX_SIZE);
        blackBox.log("[INFO][MAIN][" + String(millis()) + "] Setting dataCAN Maximum Mailboxes: " + CAN_MAILBOX_SIZE);
        dataCAN.setMaxMB(CAN_MAILBOX_SIZE);

        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Enabling dataCAN FIFO Mailboxes");
        blackBox.log("[INFO][MAIN][" + String(millis()) + "] Enabling dataCAN FIFO Mailboxes");
        dataCAN.enableFIFO();

        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Enable dataCAN FIFO Mailbox Interrupts");
        blackBox.log("[INFO][MAIN][" + String(millis()) + "] Enable dataCAN FIFO Mailbox Interrupts");
        dataCAN.enableFIFOInterrupt();

        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Setting dataCAN Mailbox Handler");
        blackBox.log("[INFO][MAIN][" + String(millis()) + "] Setting dataCAN Mailbox Handler");
        dataCAN.onReceive(canMsgCallback);

        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Enabling dataCAN Mailbox Distribution");
        blackBox.log("[INFO][MAIN][" + String(millis()) + "] Enabling dataCAN Mailbox Distribution");
        dataCAN.distribute();

        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Printing dataCAN Mailbox Status");
        dataCAN.mailboxStatus();
    }

    if constexpr (MAIN_VERBOSE) Serial.println("[DEBUG][MAIN][" + String(millis()) + "] Setup Complete");
    flash(2500);
}


/** @brief loop to be called by the Teensy 4.1 - all critical processing logic is ran through here. */
void loop() {
    flash(100);
    if constexpr (MAIN_VERBOSE) Serial.println(String("[DEBUG][MAIN][" + String(millis()) + "] Starting Loop"));

    if constexpr (USE_CAN_MAILBOXES)
    {
        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println(String("[DEBUG][MAIN][" + String(millis()) + "] motorCAN Polling Async Event Loops"));
        motorCAN.events();

        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println(String("[DEBUG][MAIN][" + String(millis()) + "] dataCAN Polling Async Event Loops"));
        dataCAN.events();
    } else
    {
        flash(50);
        if constexpr (MAIN_VERBOSE) Serial.println(String("[DEBUG][MAIN][" + String(millis()) + "] blackBox Polling CAN Lines"));
        blackBox.readCAN();
    }

    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println(String("[DEBUG][MAIN][" + String(millis()) + "] blackBox Polling Serial Port"));
    blackBox.readSerial();

    flash(50);
    if constexpr (MAIN_VERBOSE) Serial.println(String("[DEBUG][MAIN][" + String(millis()) + "] blackBox Checking Save Status"));
    blackBox.checkSave();
}