#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


ESP8266WebServer *server;

void setup() {
    // load config
    const char* ssid = "AirPort07934";
    const char* pass = "3101035127901";
    const int port = 8080;
    const char* prefix = "jRemocon/";
    IPAddress ip(192, 168, 104, 34); 
    IPAddress subnet(192, 168, 104, 0); 
    IPAddress gateway(192, 168, 104, 1); 

    // initialize
    Serial.begin(115200);
    delay(500);
    Serial.print("coning.");

    // connect to wifi
    WiFi.config(ip, gateway, subnet);
    WiFi.begin(ssid, pass);
    Serial.print("connecting.");
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nwifi connected.");
    Serial.print("IP address:");
    Serial.println(WiFi.localIP());

    // server setup
    // TODO: add prefix
    server = new ESP8266WebServer(port);
    server->on("/", handleHelp);
    server->on("/help", handleHelp);
    server->on("/send", handleSend);
    server->on("/inline", []() {
            server->send(200, "text/plain", "this works as well");
            });
    server->onNotFound(handleNotFound);
    server->begin();
    Serial.println("HTTP server started");
}

void loop() {
    server->handleClient();
}

void handleSend() {
    // send
    // byte val = 0;
    // htoi("f5ca", val);
    byte val = 0;
    // std::stringstream ss("f5");
    // ss >> std::hex >> val;
    Serial.println(val, DEC);
    server->send(200, "text/plain", "ok");
}

int htoi(const char* src, byte &dest) {
    char input[5];
    input[0] = '0';
    input[1] = 'x';
    input[2] = src[0];
    input[3] = src[1];
    input[4] = '\0';
    char *err = NULL;
    long result = strtol(input, &err, 16);
    Serial.println(result, DEC);
    Serial.println(err);
    if (err != NULL) return -1;
    dest = result;
    return 0;
}

void handleHelp() {
    // help
    server->send(200, "text/plain", "help");
}

void handleNotFound() {
    // not found
    server->send(404, "text/plain", "not found");
}
