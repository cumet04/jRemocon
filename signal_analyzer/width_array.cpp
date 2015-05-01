#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

vector<int> encodeToWidthArray(vector<bool> bit_array, int period);
vector<bool> decodeToBinarySignal(vector<int> width_array, int period);

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
        vector<int> width_array = encodeToWidthArray(bit_array, period);

        // output result
        cout << period;
        for( int width : width_array ) cout << " " << width;
    }
    else if( mode == MODE_DECODE )
    {
        int period = 0;
        vector<int> width_array;

        cin >> period;
        while( cin )
        {
            int width;
            cin >> width;
            *Log << "log: input width = " << width << endl;
            width_array.push_back(width);
        }
        width_array.pop_back();

        vector<bool> bit_array = decodeToBinarySignal(width_array, period);
        cout << to_string(period) << " ";
        for( bool bit : bit_array ) cout << (bit ? '1' : '0');
    }
    return 0;
}

vector<int> encodeToWidthArray(vector<bool> bit_array, int period)
{
    *Log << "log: encodeToWidthArray" << std::endl;

    vector<int> width_array;

    auto bit_iter = bit_array.begin(); 
    while( bit_iter < bit_array.end() )
    {
        bool bit = *bit_iter;
        int width = 0;
        while( bit == *bit_iter && bit_iter != bit_array.end()  )
        {
            width += period;
            bit_iter++;
        }
        width_array.push_back(width);
    }
    return width_array;
}

vector<bool> decodeToBinarySignal(vector<int> width_array, int period)
{
    *Log << "log: decodeToWidthArray" << std::endl;
    vector<bool> bit_array;

    bool level = true;
    for( int width : width_array )
    {
        int count = width / period;
        *Log << width << ", " << count << endl;
        for( int i=0; i<count; i++ ) bit_array.push_back(level);
        level = !level;
    }
    return bit_array;
}
