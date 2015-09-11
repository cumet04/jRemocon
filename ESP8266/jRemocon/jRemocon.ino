#include <ESP8266WiFi.h>

#define equal(a,b) (strcmp(a,b) == 0)

const int LED_PIN = 2;

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
    digitalWrite(LED_PIN, LOW);
    Serial.begin(115200);
    delay(10);

    if (connectWifi(Wifi_SSID, Wifi_PASS) != WL_CONNECTED) {
        return;
    }

    server.begin();
    Serial.print("wait for request: ");
    Serial.println(WiFi.localIP());
}

void loop() {

    WiFiClient client = server.available();
    if (!client) return;
    Serial.println("connection established.");

    Serial.println("log: readRequest");
    char request[0x500];
    if (readRequest(client, request, sizeof(request)) != 0) return;
    Serial.print("res: request = "); Serial.println(request);

    Serial.println("log: extractMethod");
    // char method[0x10], param_str[0x1000];
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
    Serial.print("res: method = "); Serial.println(method);
    Serial.print("res: param_str = "); Serial.println(param_str);

    Serial.println("log: processRequest");
    char res_status[0x100], res_message[0x100];
    if (equal(method, "") || equal(method, "/") || equal(method, "/help")) {
        Serial.println("log: processRequest_help");
        processRequest_help(res_status, res_message);
    }
    if (equal(method, "/send")) {
        Serial.println("log: processRequest_send");
        processRequest_send(param_str, res_status, res_message);
    }
    Serial.print("res: res_status = "); Serial.println(res_status);
    Serial.print("res: res_message = "); Serial.println(res_message);

    Serial.println("log: respondString");
    respondString(client, res_status, res_message);

    client.stop();

    Serial.print("wait for request: ");
    Serial.println(WiFi.localIP());
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
        strcpy(method, mp);

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
            Serial.println("readRequest failed: connection closed.");
            return -1;
        }
        if (!client.available()) break;

        request[i] = client.read();
        i++;
        if (i == size) {
            Serial.println("readRequest failed: buffer over flow.");
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
    // String pulse_raw = extractValue(input, "pulse");
    // if (pulse_raw == "") {
    //     status = HTTP_BADREQUEST;
    //     response = "parse pulse failed";
    //     return;
    // }
    // unsigned int pulse = pulse_raw.toInt();

    // String signal_str = extractValue(input, "signal");
    // if (signal_str == "") {
    //     status = HTTP_BADREQUEST;
    //     response = "parse pulse failed";
    //     return;
    // }

    // byte signal[0xFF] = {};
    // int size = sizeof(signal);
    // // switch (decodeSignalData(signal_str, signal_bytes, size)) {
    // //     case -1:
    // //         status = HTTP_BADREQUEST;
    // //         response = "signal has nox-hex char";
    // //         return;
    // //     case -1:
    // //         status = HTTP_ERROR;
    // //         response = "decodeSignalData is falied";
    // //         return;
    // // }
    // status = HTTP_OK;
    // response = "send accepted: " + input;

    // unsigned int time0 = micros();
    // emitSignal(pulse, signal, size, LED_PIN);
    // unsigned int time1 = micros();
    // Serial.println(time1-time0);

    return;
}


void respondString(WiFiClient client, const char *status, const char *response) {
    Serial.println(response);

    // return header
    client.print("HTTP/1.1 ");
    client.println(status);
    client.println("Content-Type: text/plain");
    if (status == HTTP_NOTALLOWED) client.println("Allow: GET");
    client.println("Connection: close");
    client.println();

    // return response string
    client.println(response);
    client.flush();

    return;
}

int connectWifi(const char* ssid, const char* pass) {
    // try to connect
    Serial.print("connectWifi: SSID = ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);

    // wait
    Serial.print("connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
// output result
// TODO: WiFi.beigin DON'T return WL_NO_SSID_AVAIL, WL_CONNECT_FAILED
    switch (WiFi.status()) {
        case WL_CONNECTED:
            Serial.println("connected.");
            return WL_CONNECTED;
            break;
        case WL_NO_SSID_AVAIL:
            Serial.println("specified ssid is not available.");
            return WL_NO_SSID_AVAIL;
            break;
        case WL_CONNECT_FAILED:
            Serial.println("connection failed.");
            return WL_CONNECT_FAILED;
            break;
    }
    Serial.println("unexpected error.");
    return WiFi.status();
}

void runTest() {
    int result;

    Serial.print("TEST_convertHexcharToByte...");
    result = TEST_convertHexcharToByte();
    if (result != 0) {
        Serial.print("failed: ");
        Serial.println(result);
        return;
    }
    Serial.println("passed.");

    Serial.print("TEST_extractValue...");
    result = TEST_extractValue();
    if (result != 0) {
        Serial.print("failed: ");
        Serial.println(result);
        return;
    }
    Serial.println("passed.");

    Serial.println("All tests are passed.");
}
