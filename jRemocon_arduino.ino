#include "ESP8266.h"
#include <SoftwareSerial.h>

#define LED_PIN 13
#define CARRIER_PIN 9
#define ESP_RX_PIN 11
#define ESP_TX_PIN 10

#define SSID "AirPort07934"
#define PASSWORD "3101035127901"


SoftwareSerial mySerial(ESP_RX_PIN, ESP_TX_PIN);
ESP8266 wifi(mySerial);

void setup(void)
{
    Serial.begin(9600);
    IRTransmitter_setup();
    // this port is ignored. it needs search embedded code.
    Server_setup(8090);
}

void Server_setup(uint32_t port)
{
    Serial.print("setup begin\r\n");

    Serial.print("FW Version:");
    Serial.println(wifi.getVersion().c_str());

    if (wifi.setOprToStationSoftAP()) {
        Serial.print("to station + softap ok\r\n");
    } else {
        Serial.print("to station + softap err\r\n");
    }

    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");
        Serial.print("IP: ");
        Serial.println(wifi.getLocalIP().c_str());    
    } else {
        Serial.print("Join AP failure\r\n");
    }

    if (wifi.enableMUX()) {
        Serial.print("multiple ok\r\n");
    } else {
        Serial.print("multiple err\r\n");
    }

    if (wifi.startTCPServer(port)) {
        Serial.print("start tcp server ok\r\n");
    } else {
        Serial.print("start tcp server err\r\n");
    }

    if (wifi.setTCPServerTimeout(10)) { 
        Serial.print("set tcp server timout 10 seconds\r\n");
    } else {
        Serial.print("set tcp server timout err\r\n");
    }

    Serial.print("setup end\r\n");
}

void processLine(uint8_t *line, uint8_t len)
{
    char buf[0xFF];

    int res = sscanf((char*)line, "GET /jRemocon/send\?%s HTTP/1.1", buf);
    if( res != 1 ) return;

    Serial.print("send: ");
    Serial.println(buf);
    transmitSignalData(buf);
}

// TODO: check mux_id
void loop(void)
{
    uint8_t buffer[0xFF] = {0};
    uint8_t mux_id;
    uint32_t len = wifi.recv(&mux_id, buffer, sizeof(buffer)-1, 100);
    if( len <= 0 ) return;

    Serial.print("Status:[");
    Serial.print(wifi.getIPStatus().c_str());
    Serial.println("]");

    Serial.print("Received from :");
    Serial.println(mux_id);

    Serial.print("Received length :");
    Serial.println(len);
    Serial.println("");

    do
    {
        uint8_t *p = buffer;
        for( ; (p - buffer) < len; p++)
        {
            if( *p == '\r' && *(p+1) == '\n' )
            {
                processLine(buffer, p - buffer);
                len -= p+2 - buffer;
                if( len > 0 ) memmove(buffer, p+2, len);
                break;
            }
        }
    }while( len > 0 );

    Serial.println("");

    if (wifi.releaseTCP(mux_id)) {
        Serial.print("release tcp ");
        Serial.print(mux_id);
        Serial.println(" ok");
    } else {
        Serial.print("release tcp");
        Serial.print(mux_id);
        Serial.println(" err");
    }

    Serial.print("Status:[");
    Serial.print(wifi.getIPStatus().c_str());
    Serial.println("]");
}



void IRTransmitter_setup() {
    pinMode(LED_PIN, OUTPUT);

    // from http://www.wsnak.com/wsnakblog/?p=4110
    // ************ 38KHz ***************
    // 16MHz * 1/1 = 16MHz
    // T = 1 / 16MHz = 0.0625us
    // 逆算 F = 1 / (0.0625 * 208 * 2) = 38.46kHz
    pinMode(CARRIER_PIN, OUTPUT);

    // OC1A(PB1/D9) toggle
    TCCR1A &= ~(1<<COM1A1);     // 0
    TCCR1A |=  (1<<COM1A0);     // 1

    // WGM13-10 = 0100 CTCモード
    TCCR1B &= ~(1<<WGM13);      // 0
    TCCR1B |=  (1<<WGM12);      // 1
    TCCR1A &= ~(1<<WGM11);      // 0
    TCCR1A &= ~(1<<WGM10);      // 0

    // ClockSource CS12-CS10 = 001 16MHz / 1 T= 0.0625us
    TCCR1B &= ~(1<<CS12);       // 0
    TCCR1B &= ~(1<<CS11);       // 0
    TCCR1B |=  (1<<CS10);       // 1

    OCR1A = 207;                // コンペア値
}

void transmitSignalData(char input[])
{
    unsigned int pulse_width;
    byte signal_bytes[0xFF];
    int count;
    int result = decodeSignalData(input, pulse_width, signal_bytes, count);
    if( result != 0 )
    {
        Serial.print("decode failed: ");
        Serial.println(result, DEC);
    }
    else
    {
        emitSignal(pulse_width, signal_bytes, count);
    }
}

// return code: 0 = ok
//              -1 = format error
//              -2 = signaldata includes non hex char
int decodeSignalData(char input[], unsigned int &width, byte signal[], int &count)
{
    // read width
    if( sscanf(input, "pulse=%u&signal=%*s", &width) != 1 && \
        sscanf(input, "signal=%*s&pulse=%u", &width) != 1 ) return -1;

    // decode signal string to byte array
    char *p = strstr(input, "signal=") + 7;
    boolean hword = true;
    byte b;
    count = 0;
    while( *p != 0 && *p != '&' )
    {
        if( convertHexcharToByte(*p, b) == false ) return -2;
        if( hword )
        {
            signal[count] = b << 4;
            hword = false;
        }
        else
        {
            signal[count] += b;
            hword = true;
            count++;
        }
        p++;
    }

    return 0;
}

boolean convertHexcharToByte(char hex, byte &out)
{
    if( hex >= '0' && hex <= '9' )
    {
        out = hex - '0';
        return true;
    }
    if( hex >= 'a' && hex <= 'f' )
    {
        out = hex - 'a' + 10;
        return true;
    }
    if( hex >= 'A' && hex <= 'F' )
    {
        out = hex - 'A' + 10;
        return true;
    }

    return false;
}

void emitSignal(unsigned int width, byte signal[], int count)
{
    for( int n = 0; n < count; n++ )
    {
        byte single_byte = signal[n];
        for( int i = 7; i >= 0; i-- )
        {
            int value = (single_byte & (1 << i)) ? HIGH : LOW;
            digitalWrite(LED_PIN, value);
            delayMicroseconds(width);
        }
    }
    digitalWrite(LED_PIN, LOW);
}
