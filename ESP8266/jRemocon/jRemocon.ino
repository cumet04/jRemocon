#include <ESP8266WiFi.h>

#define equal(a,b) (strcmp(a,b) == 0)

const int LED_PIN = 2;

IPAddress ip(192, 168, 104, 34); 
IPAddress subnet(192, 168, 104, 0); 
IPAddress gateway(192, 168, 104, 1); 
const char* Wifi_SSID = "AirPort07934";
const char* Wifi_PASS = "3101035127901";
const char* API_PREFIX = "/jRemocon";
const int Server_Portnum = 8080;

const char* HTTP_OK = "200 OK";
const char* HTTP_NOTALLOWED = "405 Method Not Allowed";
const char* HTTP_NOTFOUND = "404 Not Found";
const char* HTTP_ERROR = "500 Internal Server Error";
const char* HTTP_BADREQUEST = "400 Bad Request";

WiFiServer server(Server_Portnum);

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);
    Serial.begin(115200);
    delay(10);

    if (connectWifi(Wifi_SSID, Wifi_PASS) != WL_CONNECTED) {
        return;
    }

    server.begin();
    Serial.print(F("wait for request: "));
    Serial.println(WiFi.localIP());
}

void loop() {
    WiFiClient client = server.available();
    if (!client) return;
    Serial.println(F("connection established."));

    Serial.println(F("log: readRequest"));
    char request[0x500];
    if (readRequest(client, request, sizeof(request)) != 0) return;
    Serial.print(F("res: request = ")); Serial.println(request);

    Serial.println(F("log: extractMethod"));
    char method[0x10], param_str[0x200];
    int result = extractMethod(request, API_PREFIX, method, param_str, \
            sizeof(method), sizeof(param_str));
    if (result == -1) {
        respondString(client, HTTP_NOTALLOWED, "unexpected HTTP method");
        client.stop();
        return;
    } else if (result == -2) {
        respondString(client, HTTP_NOTFOUND, "API path not found");
        client.stop();
        return;
    } else if (result == -3) {
        respondString(client, HTTP_ERROR, "buffer over flow");
        client.stop();
        return;
    }
    Serial.print(F("res: method = ")); Serial.println(method);
    Serial.print(F("res: param_str = ")); Serial.println(param_str);

    Serial.println(F("log: processRequest"));
    char res_status[0x50], res_message[0x200];
    if (strlen(method) == 0 || equal(method, "/") || equal(method, "/help")) {
        Serial.println(F("log: processRequest_help"));
        processRequest_help(res_status, res_message);
    }
    if (equal(method, "/send")) {
        Serial.println(F("log: processRequest_send"));
        processRequest_send(param_str, res_status, res_message);
    }
    Serial.print(F("res: res_status = ")); Serial.println(res_status);
    Serial.print(F("res: res_message = ")); Serial.println(res_message);

    Serial.println(F("log: respondString"));
    respondString(client, res_status, res_message);

    client.stop();

    Serial.print(F("wait for request: "));
    Serial.println(WiFi.localIP());
    Serial.println(ESP.getFreeHeap(),DEC);
}

int extractMethod(char request[], const char* prefix, char method[], char param[],
                    int size_m, int size_p) {
    char *p = strtok(request, " ");
    if (strncmp(p, "GET", 3) != 0) return -1;

    p = strtok(NULL, " ");
    if (strncmp(p, prefix, strlen(prefix)) != 0) return -2;

    if (strchr(p, '?') == NULL) {
        if (strlen(p + strlen(prefix)) > size_m -1) return -3;
        strcpy(method, p +strlen(prefix));

        param[0] = '\0';
    } else {
        char *mp = strtok(p, "?");
        if (strlen(mp + strlen(prefix)) > size_m -1) return -3;
        strcpy(method, mp + strlen(prefix));

        char *pp = strtok(NULL, "");
        if (strlen(pp) > size_p-1) return -3;
        strcpy(param, pp);
    }
    return 0;
}

int readRequest(WiFiClient client, char request[], int size) {
    while (!client.available()) ; // wait until first byte is reached

    int i = 0;
    while (true) {
        if (!client.connected()) {
            Serial.println(F("readRequest failed: connection closed."));
            return -1;
        }
        if (!client.available()) break;

        request[i] = client.read();
        i++;
        if (i == size) {
            Serial.println(F("readRequest failed: buffer over flow."));
            return -2;
        }
    }
    request[i] = '\0';
    return 0;
}

void processRequest_help(char status[], char response[]) {
    strcpy(status, HTTP_OK);
    strcpy(response, "to be implemented; help message");
    return;
}

void processRequest_send(char input[], char status[], char response[]) {
    // parse params
    char pulse_raw[0xF] = {};
    char signal_raw[0x200] = {};
    if (parseParams(input, pulse_raw, signal_raw, 
            sizeof(pulse_raw), sizeof(signal_raw)) != 0) {
        strcpy(status, HTTP_BADREQUEST);
        strcpy(response, "parse parameters failed (1)");
        return;
    }

    // check params
    if (pulse_raw[0] == '\0' || signal_raw[0] == '\0') {
        strcpy(status, HTTP_BADREQUEST);
        strcpy(response, "parse parameters failed (2)");
        return;
    }
    int pulse = atoi(pulse_raw);
    if (pulse <= 0) {
        strcpy(status, HTTP_BADREQUEST);
        strcpy(response, "parse parameters failed (3)");
        return;
    }
    byte signal_bytes[0x100];
    int size = sizeof(signal_bytes);
    if (decodeSignalData(signal_raw, signal_bytes, size) != 0) {
        strcpy(status, HTTP_BADREQUEST);
        strcpy(response, "parse parameters failed (4)");
        return;
    }

    strcpy(status, HTTP_OK);
    strcpy(response, "send accepted");

    Serial.println("");

    emitSignal(pulse, signal_bytes, size, LED_PIN);

    return;
}

int parseParams(char input[], char pulse[], char signal[], int size_p, int size_s) {
    char *p = strtok(input, "&");
    if (p == NULL) return -1;

    for (int i=0; i<2; i++) {
        if (strncmp(p, "pulse=", strlen("pulse=")) == 0) {
            if (size_p <= strlen(p+6)) {
                Serial.println(F("buffer over flow: pulse"));
                return -2;
            }
            strcpy(pulse, p+6);
        }
        else if (strncmp(p, "signal=", strlen("signal=")) == 0) {
            if (size_s <= strlen(p+7)) {
                Serial.println(F("buffer over flow: pulse"));
                return -3;
            }
            strcpy(signal, p+7);
        }
        p = strtok(NULL, "");
    }
    return 0;
}


void respondString(WiFiClient client, const char *status, const char *response) {
    Serial.println(response);

    // return header
    client.print(F("HTTP/1.1 "));
    client.println(status);
    client.println(F("Content-Type: text/plain"));
    if (status == HTTP_NOTALLOWED) client.println(F("Allow: GET"));
    client.println(F("Connection: close"));
    client.println();

    // return response string
    client.println(response);
    client.flush();

    return;
}

int connectWifi(const char* ssid, const char* pass) {
    WiFi.config(ip, gateway, subnet);
    // try to connect
    Serial.print(F("connectWifi: SSID = "));
    Serial.println(ssid);
    WiFi.begin(ssid, pass);

    // wait
    Serial.print(F("connecting"));
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
// output result
// TODO: WiFi.beigin DON'T return WL_NO_SSID_AVAIL, WL_CONNECT_FAILED
    switch (WiFi.status()) {
        case WL_CONNECTED:
            Serial.println(F("connected."));
            return WL_CONNECTED;
            break;
        case WL_NO_SSID_AVAIL:
            Serial.println(F("specified ssid is not available."));
            return WL_NO_SSID_AVAIL;
            break;
        case WL_CONNECT_FAILED:
            Serial.println(F("connection failed."));
            return WL_CONNECT_FAILED;
            break;
    }
    Serial.println(F("unexpected error."));
    return WiFi.status();
}
