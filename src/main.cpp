#include <Arduino.h>
#include "CFirebase.h"
#include <cstring>

#define PING  1
#define PONG  2
#define CODE  PONG    /* Please define PING or PONG */
#define TXpin 0
#define RXpin 2
#define ATSerial Serial

const char *AT_RX_TIMEOUT = "AT_RX_TIMEOUT";
//16byte hex key
String lora_app_key = "44 22 33 44 55 66 77 88 99 aa bb cc dd ee ff 00";

SoftwareSerial DebugSerial(RXpin, TXpin);

SNIPE SNIPE(ATSerial);

void RecvToCommandArray(const String &recv, String command[]);

bool IsCommandValid(const String *command);

void setup() {
    ATSerial.begin(115200);
    // put your setup code here, to run once:
    while (ATSerial.read() >= 0) {}
    while (!ATSerial);
    DebugSerial.begin(115200);

    Esp8266FirebaseInit();
    /* SNIPE LoRa Initialization */
    if (!SNIPE.lora_init()) {
        DebugSerial.println("LoRa Initialization Fail!");
    }

    /* SNIPE LoRa Set Appkey */
    if (!SNIPE.lora_setAppKey(lora_app_key)) {
        DebugSerial.println("LoRa app key value has not been changed");
    }

    /* SNIPE LoRa Set Frequency */
    if (!SNIPE.lora_setFreq(LORA_CH_2)) {
        DebugSerial.println("LoRa Frequency value has not been changed");
    }

    /* SNIPE LoRa Set Spreading Factor */
    if (!SNIPE.lora_setSf(LORA_SF_7)) {
        DebugSerial.println("LoRa Sf value has not been changed");
    }

    /* SNIPE LoRa Set Rx Timeout
     * If you select LORA_SF_12,
     * RX Timout use a value greater than 5000
    */
    if (!SNIPE.lora_setRxtout(5000)) {
        DebugSerial.println("LoRa Rx Timout value has not been changed");
    }
    DebugSerial.println("SmartFarm LoRa Client");
}

void loop() {
    DebugSerial.println("\n\n\nStart of Loop\n");
    String ver = SNIPE.lora_recv();
    delay(300);
    String condition = ReadCommandDocument();

    String recv = "1,S1,L1,H1,T1";
    String command[5];

    if (ver != AT_RX_TIMEOUT && ver.length() > 5) {
        RecvToCommandArray(ver, command);
        DebugSerial.println("\tReceived Commands: ");
        for (int i = 0; i < 5; i++) {
            DebugSerial.println("\t\t"+command[i]);
        }
        DebugSerial.println("\tcommand is :" + condition);
        DebugSerial.println("\t\trecv success");
        if (ver.length() > 0 && (IsCommandValid(command)))
            SendDataToFirebase(1, command);
        if (SNIPE.lora_send(condition)) {
            DebugSerial.println("\tsend success");
        } else {
            DebugSerial.println("\tsend fail");
            delay(500);
        }
    } else {
        DebugSerial.println("\tReceive Fail : "+ver);
        DebugSerial.println("\tcommand is :" + condition);
    }
    DebugSerial.println("\nEnd of Loop\n");
}

bool IsCommandValid(const String *command) {
    return strcmp(AT_RX_TIMEOUT, command[0].c_str()) != 0;
}

void RecvToCommandArray(const String &recv, String command[]) {
    char *str = (char *) malloc(strlen(recv.c_str()) + 1);
    strcpy(str, recv.c_str());
    char *ptr = strtok(str, ",");
    for (int i = 0; i < 5 && ptr != nullptr; i++) {
        command[i] = ptr;
        ptr = strtok(nullptr, ",");
    }
    free(str);
}

