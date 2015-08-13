#define LED_PIN 13
#define CARRIER_PIN 9
void setup() {
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

    // **********************************

    Serial.begin(9600);
}
void loop() {

    char input[] = "pulse=562&signal=ffff00a2aa88a2288a88888a22aa8a";// TEST: light off

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
    delay(1000);
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
