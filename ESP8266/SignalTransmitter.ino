
void IRTransmitter_setup() {
//     pinMode(LED_PIN, OUTPUT);
// 
//     // from http://www.wsnak.com/wsnakblog/?p=4110
//     // ************ 38KHz ***************
//     // 16MHz * 1/1 = 16MHz
//     // T = 1 / 16MHz = 0.0625us
//     // 逆算 F = 1 / (0.0625 * 208 * 2) = 38.46kHz
//     pinMode(CARRIER_PIN, OUTPUT);
// 
//     // OC1A(PB1/D9) toggle
//     TCCR1A &= ~(1<<COM1A1);     // 0
//     TCCR1A |=  (1<<COM1A0);     // 1
// 
//     // WGM13-10 = 0100 CTCモード
//     TCCR1B &= ~(1<<WGM13);      // 0
//     TCCR1B |=  (1<<WGM12);      // 1
//     TCCR1A &= ~(1<<WGM11);      // 0
//     TCCR1A &= ~(1<<WGM10);      // 0
// 
//     // ClockSource CS12-CS10 = 001 16MHz / 1 T= 0.0625us
//     TCCR1B &= ~(1<<CS12);       // 0
//     TCCR1B &= ~(1<<CS11);       // 0
//     TCCR1B |=  (1<<CS10);       // 1
// 
//     OCR1A = 207;                // コンペア値
}

// return code: 0 = ok
//              -1 = format error
//              -2 = signaldata includes non hex char
//              -3 = buffer over flow
int decodeSignalData(String input, unsigned int &width, byte signal[]) {
    // read params; width and signal
    String signal_str = extractValue(input, "signal");
    if (signal_str == "") return -1;
    
    String width_raw = extractValue(input, "pulse");
    if (width_raw == "") return -1;
    width = width_raw.toInt();

    // decode signal string to byte array
    for (int i = 0; i < signal_str.length(); i++) {
        byte b = convertHexcharToByte(signal_str[i]);
        if (b == -1) return -2;

        if (i/2 >= sizeof(signal)) return -3;
        signal[i/2] = (i%2 == 1) ? b << 4 : b;
    }

    return 0;
}

String extractValue(String input, String param) {
    int from, to;
    from = input.indexOf(param + "=");
    if (from == -1) return ""; // param not found
    if (from != 0 && input[from-1] != '&') return ""; // param incomplete
    from += param.length() + 1;

    to = input.indexOf("&", from);
    to = to != -1 ? to : input.length();

    return input.substring(from, to);
}

int TEST_extractValue() {
    int number = 1;

    String input = "signal=aaff00&pulse=562";
    if (extractValue(input, "signal") == "aaff00") number++; else return number;
    if (extractValue(input, "pulse")  == "562")    number++; else return number;
    if (extractValue(input, "signa")  == "")       number++; else return number;
    if (extractValue(input, "puls")   == "")       number++; else return number;
    if (extractValue(input, "ignal")  == "")       number++; else return number;
    if (extractValue(input, "ulse")   == "")       number++; else return number;

    return 0;
}


byte convertHexcharToByte(char hex) {
    if (hex >= '0' && hex <= '9') return hex - '0';
    if (hex >= 'a' && hex <= 'f') return hex - 'a' + 10;
    if (hex >= 'A' && hex <= 'F') return hex - 'A' + 10;
    return -1;
}

int TEST_convertHexcharToByte() {
    int number = 1;
    if (convertHexcharToByte('0') == 0 ) number++; else return number;
    if (convertHexcharToByte('9') == 9 ) number++; else return number;
    if (convertHexcharToByte('a') == 10) number++; else return number;
    if (convertHexcharToByte('f') == 15) number++; else return number;
    if (convertHexcharToByte('A') == 10) number++; else return number;
    if (convertHexcharToByte('F') == 15) number++; else return number;
    if (convertHexcharToByte('g') == (byte)(-1)) number++; else return number;
    if (convertHexcharToByte('G') == (byte)(-1)) number++; else return number;
    return 0;
}


void emitSignal(unsigned int width, byte signal[], int pin) {
    for (int n = 0; n < sizeof(signal); n++) {
        byte single_byte = signal[n];
        for(int i = 7; i >= 0; i--) {
            int value = (single_byte & (1 << i)) ? HIGH : LOW;
            digitalWrite(pin, value);
            delayMicroseconds(width);
        }
    }
    digitalWrite(pin, LOW);
}

