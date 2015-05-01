#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

vector<int> readToWidthArray();
int calcPulsePeriod(vector<int> width_array);
vector<bool> convertToBinarySignal(vector<int> width_array, int period);
string readline_to();

int timeout_us = 100*1000;
ostream *Log;


int main(int argc, char** argv)
{
    Log = new ofstream("/dev/null", ios_base::out);
    int period = 0;
    // read init parameters ----------------------------------------------------
    for(int i=1; i<argc; i++)
    {
        if( strncmp(argv[i], "-v", 2) == 0) Log = new iostream(cout.rdbuf());
        if( period == 0 )
        {
            int p;
            if( sscanf(argv[i], "--pulse=%d", &p) == 1 ) period = p;
        }
    }

    vector<int> width_array = readToWidthArray();
    if( period == 0 ) period = calcPulsePeriod(width_array);
    vector<bool> bit_array = 
        convertToBinarySignal(width_array, period);

    cout << to_string(period) << " ";
    for( bool bit : bit_array ) cout << (bit ? '1' : '0');
    cout << endl;

    return 0;
}


vector<int> readToWidthArray()
{
    *Log << "log: read pulse width." << endl;

    vector<int> width_array;

    readline_to();
    while(1)
    {
        // if timeout, finish reading
        string line = readline_to();
        if( line == "" ) break;

        int width = 0;
        int n = EOF;
        if( line[0] == 'p' ) n = sscanf(line.c_str(), "pulse %d", &width);
        if( line[0] == 's' ) n = sscanf(line.c_str(), "space %d", &width);

        if( n == EOF )
        {
            cerr << "ERROR: reading line is failed." << endl;
            cerr << "-- input line : " << line << endl;
            return width_array;
        }
        width_array.push_back(width);
    }
    return width_array;
}

int calcPulsePeriod(vector<int> width_array)
{
    *Log << "log: calcurate pulse period." << endl;

    const int triangle_size = 10;
    int width_max = *max_element(width_array.begin(), width_array.end());
    vector<int> freq(width_max + triangle_size);

    for( int width : width_array )
    {
        freq[width] += triangle_size;
        for( int i=1; i<triangle_size; i++ )
        {
            freq[width + i] += triangle_size - i;
            freq[width - i] += triangle_size - i;
        }
    }

    vector<int>::iterator max_iter = max_element(freq.begin(), freq.end() );
    int period = distance(freq.begin(), max_iter);

    return period;
}

vector<bool> convertToBinarySignal(vector<int> width_array, int period)
{
    *Log << "log: generate bit array from pulse width." << std::endl;

    vector<bool> bit_array;
    bool level = true;
    for( int width : width_array )
    {
        // パルス幅が周期何個分かを計算
        int count = (int)round( (double)width / period );

        // push bit pulse_count times
        for( int i=0; i < count; i++ ) bit_array.push_back(level);
        level = ! level; // bit reverse

        *Log << "raw width = " << width << ", pulse count = " << count << endl;
    }
    return bit_array;
}

string readline_to()
{
    char line[0xFF];

    fd_set readfds;
    struct timeval timeout;
    if( timeout.tv_usec != timeout_us )
    {
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        timeout.tv_sec = 0;
        timeout.tv_usec = timeout_us;
    }

    if( select(1, &readfds, NULL, NULL, &timeout) == 0 ) return "";
    if( fgets(line, sizeof(line), stdin) == NULL ) return "";
 
    string result;
    result += line;
    return result;
}

