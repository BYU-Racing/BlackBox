#include <Arduino.h>
#include <FlexCAN_T4.h>
#include "BlackBox.h"


constexpr int CAN_BAUD_RATE = 250000;
constexpr int SERIAL_BAUD_RATE = 9600;
constexpr uint32_t SAVE_INTERVAL = 30000;
constexpr int MAX_MAILBOXES = 8;
constexpr bool USE_MAILBOXES = false;


FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> dataCAN;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> motorCAN;
BlackBox blackBox;


void canMsgCallback(const CAN_message_t& canMsg)
{
    blackBox.writeCANMsg(canMsg);
}

void enableMailboxes()
{
    dataCAN.setMaxMB(MAX_MAILBOXES);
    motorCAN.setMaxMB(MAX_MAILBOXES);
    dataCAN.enableFIFO();
    motorCAN.enableFIFO();
    dataCAN.enableFIFOInterrupt();
    motorCAN.enableFIFOInterrupt();
    dataCAN.onReceive(canMsgCallback);
    motorCAN.onReceive(canMsgCallback);
    dataCAN.distribute();
    motorCAN.distribute();
    dataCAN.mailboxStatus();
    motorCAN.mailboxStatus();
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(SERIAL_BAUD_RATE);
    dataCAN.begin();
    motorCAN.begin();
    dataCAN.setBaudRate(CAN_BAUD_RATE);
    motorCAN.setBaudRate(CAN_BAUD_RATE);
    if constexpr (USE_MAILBOXES) enableMailboxes();
    blackBox.begin(&dataCAN, &motorCAN, SAVE_INTERVAL);
}

void loop() {
    if constexpr (USE_MAILBOXES)
    {
        dataCAN.events();
        motorCAN.events();
    } else
    {
        blackBox.readCAN();
    }
    blackBox.save();
}