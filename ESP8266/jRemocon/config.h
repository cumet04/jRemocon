// ArduinoJson.hやFS.hはここでincludeすべきだが，
// ArduinoJson.hがこのファイルからincludeできなかった（No such file or directory）
// ため，メインのjRemocon.inoでまとめてincludeするようにしている

typedef struct {
    char ssid[0xFF];
    char pass[0xFF];
    IPAddress *ip;
    IPAddress *subnet;
    IPAddress *gateway;
} wifi_config;


IPAddress* toIPAddress(const char *str) {
    char buf[0xFF];
    char *tp, *err = NULL;
    int num[4];
    strcpy(buf, str);

    tp = strtok(buf, ".");
    for(int i=0; i<4; i++) {
        if (tp == NULL) return NULL;
        num[i] = strtol(tp, &err, 10);
        if (*err != '\0') return NULL;
        tp = strtok(NULL, ".");
    }

    return new IPAddress(num[0], num[1], num[2], num[3]);
}

// -1: file error
// -2: json error
// -3: param load error
int loadConfig(wifi_config &config) {
    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile) {
        Serial.println("Failed to open config file");
        return -1;
    }

    size_t size = configFile.size();
    if (size > 1024) {
        Serial.println("Config file size is too large");
        return -1;
    }

    std::unique_ptr<char[]> buf(new char[size]);
    configFile.readBytes(buf.get(), size);

    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& json = jsonBuffer.parseObject(buf.get());

    if (!json.success()) {
        Serial.println("Failed to parse config file");
        return -2;
    }

    if (json.containsKey("ssid") == false) {
        Serial.println("Failed to load parameter: ssid");
        return -3;
    }
    if (json.containsKey("pass") == false) {
        Serial.println("Failed to load parameter: pass");
        return -3;
    }
    if (json.containsKey("ip") == false) {
        Serial.println("Failed to load parameter: ip");
        return -3;
    }
    if (json.containsKey("subnet") == false) {
        Serial.println("Failed to load parameter: subnet");
        return -3;
    }
    if (json.containsKey("gateway") == false) {
        Serial.println("Failed to load parameter: gateway");
        return -3;
    }

    strcpy(config.ssid, json["ssid"]);
    strcpy(config.pass, json["pass"]);
    config.ip = toIPAddress(json["ip"]);
    config.subnet = toIPAddress(json["subnet"]);
    config.gateway = toIPAddress(json["gateway"]);
	return 0;
}


int saveConfig(const char *ssid, const char *pass, const char *ip,
                const char *subnet, const char *gateway) {
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["ssid"] = ssid;
    json["pass"] = pass;
    json["ip"] = ip;
    json["subnet"] = subnet;
    json["gateway"] = gateway;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return false;
    }
    json.printTo(configFile);
}