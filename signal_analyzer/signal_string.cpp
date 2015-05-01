
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

string encodeToSignal(vector<bool> bit_array, int period);
vector<bool> decodeToBinarySignal(string signal_str);
ostream *Log;


#define MODE_ENCODE 0
#define MODE_DECODE 1

int main(int argc, char** argv)
{
    int mode = MODE_ENCODE;
    Log = new ofstream("/dev/null", ios_base::out);
    // read init parameters ----------------------------------------------------
    for(int i=1; i<argc; i++)
    {
        if( strncmp(argv[i], "-v", 2) == 0) Log = new iostream(cout.rdbuf());
        if( strncmp(argv[i], "-e", 2) == 0) mode = MODE_ENCODE;
        if( strncmp(argv[i], "-d", 2) == 0) mode = MODE_DECODE;
    }

    if( mode == MODE_ENCODE )
    {
        // read parameter
        int period = 0;
        string bit_str;
        cin >> period  >> bit_str;

        // convert from input to vector<bool>
        vector<bool> bit_array;
        for( char c : bit_str )
        {
            bit_array.push_back(c == '1');
        }

        string signal_str = encodeToSignal(bit_array, period);
        cout << "pulse=" << period << "&signal=" << signal_str;
    }
    else if( mode == MODE_DECODE )
    {
        string buf;
        cin >> buf;
        int period;
        char signal_str[buf.size()];
        sscanf(buf.c_str(), "pulse=%d&signal=%s", &period, signal_str);

        vector<bool> bit_array = decodeToBinarySignal(signal_str);
        cout << to_string(period) << " ";
        for( bool bit : bit_array ) cout << (bit ? '1' : '0');
    }
    return 0;
}

string encodeToSignal(vector<bool> bit_array, int period)
{
    *Log << "log: SignalString::generateSignalString" << std::endl;

    string signal_data;

    auto bit_iter = bit_array.begin();
    while( bit_iter < bit_array.end() )
    {
        // convert 4 bit to 1 word
        // e.g. bits=1101 -> word=13
        char signal_word = 0;
        if( *(bit_iter+0) ) signal_word += 1 << 3;
        if( *(bit_iter+1) ) signal_word += 1 << 2;
        if( *(bit_iter+2) ) signal_word += 1 << 1;
        if( *(bit_iter+3) ) signal_word += 1 << 0;

        // convert word to hex-char
        // Note: If it use non ASCII code, this can't be guaranteed to work
        char char_base = signal_word < 10 ? '0' : ('a' - 10);
        char hexchar = char_base + signal_word;

        signal_data += hexchar;
        bit_iter += 4;
    }
    return signal_data;
}

vector<bool> decodeToBinarySignal( string signal_str )
{
    *Log << "log: SignalString::convertToBinarySignal" << std::endl;

    vector<bool> bit_array;
    for( char word : signal_str )
    {
        if( '0' <= word && word <= '9' ) word = word - '0';
        if( 'a' <= word && word <= 'f' ) word = word - 'a' + 10;
        bit_array.push_back( word & (1 << 3) );
        bit_array.push_back( word & (1 << 2) );
        bit_array.push_back( word & (1 << 1) );
        bit_array.push_back( word & (1 << 0) );
    }
    while( bit_array.back() == false ) bit_array.pop_back();

    return bit_array;
}
