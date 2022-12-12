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

void WaitForMessage(int node);

void SendCommand();

void RecvToCommandArray(const String &recv, String command[]);


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
    SendCommand();
    DebugSerial.println("\nEnd of Loop\n");
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


void WaitForMessage(int node) {
    int count = 0;
    String command[5];
//    String Node = node + "";
    String Num(node);
    String Node = Num;
    DebugSerial.println("Waiting for Node "+Num);
    do {
        count++;
        String ver = SNIPE.lora_recv();
        if (ver != AT_RX_TIMEOUT && ver.length() > 5) {
            RecvToCommandArray(ver, command);
            DebugSerial.println("\tReceived Commands: ");
            for (int i = 0; i < 5; i++) {
                DebugSerial.println("\t\t" + command[i]);
            }
            if (Node != command[0]) {
                if (SNIPE.lora_send(Node)) {
                    DebugSerial.println("\tRequest has been sent : " + count);
                }
                continue;
            }
            DebugSerial.println("\t\t수신 양호");
            SendDataToFirebase(node, command);
            return;
        } else {
            DebugSerial.println("\tinvalid message : " + ver);
            DebugSerial.println("\tresend request");
            if (SNIPE.lora_send(Node)) {
                DebugSerial.println("\tRequest has been sent : " + count);
                continue;
            }
        }
    } while (count < 3);
}

String prevCondition = "";
int mode = 0;

void SendCommand() {
    switch (mode) {
        case 1:
            if (SNIPE.lora_send("1")) {
                DebugSerial.println("\tsend success");
            } else {
                DebugSerial.println("\tsend fail");
                delay(500);
            }
            WaitForMessage(1);
            break;
        case 2:
            if (SNIPE.lora_send("2")) {
                DebugSerial.println("\tsend success");
            } else {
                DebugSerial.println("\tsend fail");
                delay(500);
            }
            WaitForMessage(2);
            break;
        case 0:
            delay(2000);
            bool successful;
            String condition = ReadCommandDocument(successful);
            if (successful) {
                prevCondition = condition;
                DebugSerial.println("세팅값 전송"+condition);
                SNIPE.lora_send(condition.c_str());
            }
            if (SNIPE.lora_send(prevCondition)) {
                DebugSerial.println("\tsend success");
            } else {
                DebugSerial.println("\tsend fail");
            }
            delay(500);
            break;
    }
    mode = (mode + 1) % 3;
}