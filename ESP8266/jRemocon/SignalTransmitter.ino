
// return code: 0 = ok
//              -1 = format error
//              -2 = signaldata includes non hex char
//              -3 = buffer over flow
int decodeSignalData(const char *input, byte signal[], int &size) {
    for (int i = 0; i < size; i++) {
        byte high = convertHexcharToByte(input[2*i]);
        if (high == -1) return -2;
        byte low;
        if (2*i+1 < strlen(input)) {
            low = convertHexcharToByte(input[2*i+1]);
            if (low == -1) return -2;
        } else low = 0;

        signal[i] = (high << 4) + low;

        if (2*i+2 >= strlen(input)) {
            size = i+1;
            return 0;
        }
    }
    return -1;
}

byte convertHexcharToByte(char hex) {
    if (hex >= '0' && hex <= '9') return hex - '0';
    if (hex >= 'a' && hex <= 'f') return hex - 'a' + 10;
    if (hex >= 'A' && hex <= 'F') return hex - 'A' + 10;
    return -1;
}

void emitSignal(unsigned int width, byte signal[], int size, int pin) {
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

