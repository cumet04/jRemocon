#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "FS.h"

#include "config.h"
#define LED_PIN 2


ESP8266WebServer *server = NULL;

void setup() {
    delay(3 * 1000);

    // initialize
    Serial.begin(115200);

    IR_OFF(LED_PIN);
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount file system");
        return;
    }

    wifi_config config;
    if (loadConfig(config) != 0) {
        Serial.println("Failed to load config file");
        return;
    }

    // connect to wifi
    WiFi.config(*config.ip, *config.gateway, *config.subnet);
    WiFi.begin(config.ssid, config.pass);
    Serial.print("connecting.");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nwifi connected.");

    // server setup
    server = new ESP8266WebServer(8080);
    server->on("/jRemocon/", handleHelp);
    server->on("/jRemocon/help", handleHelp);
    server->on("/jRemocon/send", handleSend);
    server->onNotFound(handleNotFound);
    server->begin();
    Serial.println("HTTP server started !!");
}

void loop() {
    if (server != NULL) server->handleClient();
}

void handleSend() {
    // read params
    if (server->hasArg("pulse") == false) {
        server->send(400, "text/plain", "param 'pulse' is not found.");
        return;
    }
    int pulse = server->arg("pulse").toInt();

    if (server->hasArg("signal") == false) {
        server->send(400, "text/plain", "param 'signal' is not found.");
        return;
    }
    String signal = server->arg("signal");

    // send signal
    int code = emitSignal(pulse, signal, LED_PIN);
    if (code == -1) {
        server->send(400, "text/plain", "signal has non-hex char.");
        return;
    }

    // respond result
    String result;
    result += "accepted data: {pulse=" + String(pulse) + \
                ", signal=" + signal + "}\r\n";
    result += "signal sent.";

    server->send(200, "text/plain", result);
}

void handleHelp() {
    // help
    server->send(200, "text/plain", "help");
}

void handleNotFound() {
    // not found
    server->send(404, "text/plain", "not found");
}