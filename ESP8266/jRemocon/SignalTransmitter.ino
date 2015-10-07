

byte toWord(char hex) {
    if (hex >= '0' && hex <= '9') return hex - '0';
    if (hex >= 'a' && hex <= 'f') return hex - 'a' + 10;
    if (hex >= 'A' && hex <= 'F') return hex - 'A' + 10;
    return -1;
}

// return code:  0 = ok
//              -1 = non-hex char is found
int emitSignal(int width, String signal, int pin) {
    analogWriteFreq(38 * 1000);

    for (int n = 0; n < signal.length(); n++) {
        byte word = toWord(signal[n]);
        if (word == -1) return -1;
        for(int i = 3; i >= 0; i--) {
            if (bitRead(word, i) == 1) IR_ON(pin);
            else                       IR_OFF(pin);
            delayMicroseconds(width);
        }
    }
    IR_OFF(pin);
    Serial.println("");
    return 0;
}

void IR_ON(int pin) {
    // analogWrite(pin, 2 * PWMRANGE / 3);
}
void IR_OFF(int pin) {
    // analogWrite(pin, PWMRANGE);
}
