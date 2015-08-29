#include <ESP8266WiFi.h>

// #define DO_TEST

const int LED_PIN = 0;

const char* Wifi_SSID = "AirPort07935";
const char* Wifi_PASS = "3101035127901";
const int Server_Portnum = 8080;

const String HTTP_OK = "200 OK";
const String HTTP_NOTFOUND = "404 Not Found";
const String HTTP_ERROR = "500 Internal Server Error";
const String HTTP_BADREQUEST = "400 Bad Request";

WiFiServer server(Server_Portnum);

void setup() {
    Serial.begin(115200);
    delay(10);
#ifdef DO_TEST
    runTest();
    return;
#endif

    if (connectWifi(Wifi_SSID, Wifi_PASS) != WL_CONNECTED) {
        return;
    }
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void loop() {
#ifdef DO_TEST
    return;
#endif
    // Serial.println("test");
    return;

    WiFiClient client = server.available();
    if (!client) return;
    Serial.println("connection established.");

    String method, param;
    int result = readRequest(client, method, param);
    if (result != 0) {
        Serial.println("readRequest failed.");
        return;
    }

    String status = "";
    String response = "";
    if (method == "/" || method == "/help") {
        processRequest_help(status, response);
    } else if (method == "/send") {
        processRequest_send(status, response, param);
    } else {
        status = HTTP_NOTFOUND;
        response = "method not found";
    }
    respondString(client, status, response);

    Serial.print("wait for request: ");
    Serial.println(WiFi.localIP());
}


int readRequest(WiFiClient client, String &method, String &param) {
    method = "";
    param = "";
    return 0;
}

void processRequest_help(String &status, String &response) {
    status = HTTP_OK;
    response = "";
    return;
}

void processRequest_send(String &status, String &response, String param) {
    unsigned int width;
    byte signal[0xFF];
    switch (decodeSignalData(param.c_str(), width, signal)) {
        case 0:
            break;
        case -1:
            status = HTTP_BADREQUEST;
            response = "invalid parameters";
            return;
        case -2:
            status = HTTP_BADREQUEST;
            response = "signal has nox-hex char";
            return;
        default:
            status = HTTP_ERROR;
            response = "decodeSignalData is falied";
            return;
    }

    emitSignal(width, signal, LED_PIN);

    return;
}


void respondString(WiFiClient client, String status, String response) {
    // return header
    // return response string
    // put end char
    return;
}

int connectWifi(const char* ssid, const char* pass) {
    // try to connect
    Serial.print("connectWifi: SSID = ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);

    // wait
    Serial.print("connecting");
    while (WiFi.status() == WL_IDLE_STATUS) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
// output result
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
