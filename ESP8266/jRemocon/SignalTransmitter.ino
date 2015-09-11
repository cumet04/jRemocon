
// return code: 0 = ok
//              -1 = format error
//              -2 = signaldata includes non hex char
//              -3 = buffer over flow
int decodeSignalData(String input, unsigned int &width, byte signal[], int size) {
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

        if (i/2 >= size) return -3;
        byte add = (i%2 == 1) ? b : b << 4;
        signal[i/2] += add;
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


void emitSignal(unsigned int width, byte signal[], int size, int pin) {
    return;
    analogWriteFreq(38 * 1000);
    for (int n = 0; n < size; n++) {
        byte single_byte = signal[n];
        for(int i = 7; i >= 0; i--) {
            int value = (single_byte & (1 << i)) ? PWMRANGE/3 : LOW;
            analogWrite(pin, value);
            delayMicroseconds(width);
        }
    }
    digitalWrite(pin, LOW);
}

