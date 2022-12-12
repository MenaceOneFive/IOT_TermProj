#ifndef __CFIREBASE_H
#define __CFIREBASE_H

#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <Firebase.h>
#include <SoftwareSerial.h>
#include "SNIPE.h"

#define NODE 0
#define SOIL 1
#define LIGHTING 2
#define HUMIDITY 3
#define TEMPERATURE 4

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define API_KEY ""
#define FIREBASE_PROJECT_ID "iot-project-acf10"
#define USER_EMAIL "dlghdus122@gmail.com"
#define USER_PASSWORD "test123"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;

FirebaseConfig config;

extern SoftwareSerial DebugSerial;

unsigned long dataMillis = 0;
int count = 0;

// The Firestore payload upload callback function
void fcsUploadCallback(CFS_UploadStatusInfo info) {
    if (info.status == fb_esp_cfs_upload_status_init) {
        DebugSerial.printf("\nUploading data (%d)...\n", info.size);
    } else if (info.status == fb_esp_cfs_upload_status_upload) {
        DebugSerial.printf("Uploaded %d%s\n", (int) info.progress, "%");
    } else if (info.status == fb_esp_cfs_upload_status_complete) {
        DebugSerial.println("Upload completed ");
    } else if (info.status == fb_esp_cfs_upload_status_process_response) {
        DebugSerial.print("Processing the response... ");
    } else if (info.status == fb_esp_cfs_upload_status_error) {
        DebugSerial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}

void Esp8266FirebaseInit() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    DebugSerial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        DebugSerial.print(".");
        delay(300);
    }
    DebugSerial.println();
    DebugSerial.print("Connected with IP: ");
    DebugSerial.println(WiFi.localIP());
    DebugSerial.println();
    /* Assign the api key (required) */
    config.api_key = API_KEY;

    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    /* Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback;  // see addons/TokenHelper.h

#if defined(ESP8266)
    // In ESP8266 required for BearSSL rx/tx buffer for large data handle, increase Rx size as needed.
    fbdo.setBSSLBufferSize(2048 /* Rx buffer size in bytes from 512 - 16384 */,
                           2048 /* Tx buffer size in bytes from 512 - 16384 */);
#endif
    fbdo.setResponseSize(2048);
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
}

/**
 * @param node 데이터를 전송하고자 하는 노드
 * @param recv 무조건 길이가 4인 배열이여야 함
 */
void SendDataToFirebase(int node, const String recv[]) {
    if (Firebase.ready()) {
        dataMillis = millis();
        const String collectionPath = "node" + recv[0] + "/";
        FirebaseJson content;

        time_t time = Firebase.getCurrentTime();
        String documentPath = collectionPath + time;

        content.set("fields/humidity/stringValue", recv[HUMIDITY]);
        content.set("fields/soil/stringValue", recv[SOIL]);
        content.set("fields/lighting/stringValue", recv[LIGHTING]);
        content.set("fields/temperature/stringValue", recv[TEMPERATURE]);
        DebugSerial.print("\tCreate a document... ");

        if (Firebase.Firestore.createDocument(
                &fbdo, FIREBASE_PROJECT_ID,
                "" /* databaseId can be (default) or empty */,
                documentPath.c_str(),
                content.raw())) {
            size_t wrote = strlen(fbdo.payload().c_str());
            DebugSerial.println("\n\tok wrote " + wrote);
        } else
            DebugSerial.println("\n\t" + fbdo.errorReason());
    }
}

String ReadCommandDocument(bool &successful) {
    //Firestore에서 문서를 가져옴
    String documentPath = "command/command";
    Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", documentPath.c_str(), "");
    DebugSerial.println(fbdo.payload());

    //가져온 문서에서 필요한 값(온도, 습도)를 추출
    FirebaseJson json;
    json.setJsonData(fbdo.payload().c_str());
    FirebaseJsonData fan;
    FirebaseJsonData waterPump;
    json.get(fan, "fields/fan/stringValue", true);
    json.get(waterPump, "fields/waterpump/stringValue", true);

    successful = (fan.success && waterPump.success);
    if (!successful)
        DebugSerial.println(fbdo.errorReason());

    String ret = "";
    ret += "T";
    ret += fan.stringValue;
    ret += ",";
    ret += "S";
    ret += waterPump.stringValue;
    DebugSerial.println(ret);
    return ret;
}

#endif