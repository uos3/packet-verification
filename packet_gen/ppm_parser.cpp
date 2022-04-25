#include <iostream>
#include <fstream>
#include <cstdint>
#include <filesystem>
using namespace std;

int main(){

    filesystem::create_directory("export");

    // input file - rocket as reference image
    ifstream rf("rocket.ppm", ios::binary);
    if(!rf) {
        cout << "Cannot open file!" << endl;
        return 1;
    }

    // auth layer as packet
    //
    // [
    //   16 bytes: auth hash
    // ],
    // [
    //   2 bytes: sequence number,
    //   1 nibble: transport information
    // ],
    // [
    //   4 bytes: length,
    //   1 nibble: command information,
    //   <=232 bytes: data
    // ]
    //
    uint8_t auth_layer[255] = {0};

    rf.ignore(15,EOF); // ignore ppm header

    uint16_t packet_count = 0;
    uint32_t content_length = 0;
    char filename_buffer[32];

    while (!rf.eof())
    {   
        content_length = 0;
        for (int i = 0; i < 232; i++) // read until EOF
        {
            rf.read((char *) &auth_layer[23+i], sizeof(uint8_t));
            if(rf.eof()) break;
            else content_length++;
        }

        // generate packet binary files
        snprintf(filename_buffer, sizeof(filename_buffer), ".\\export\\packet_%d.bin", ++packet_count);
        ofstream wf(filename_buffer, ios::binary);
        if(!wf) {
            cout << "Cannot open file!" << endl;
            return 1;
        }

        // setting sequence number
        auth_layer[16] = (packet_count >> 8) & (0xff);
        auth_layer[17] = packet_count & (0xff);

        // setting length
        auth_layer[18] = (content_length >> 28) & (0x0f);
        auth_layer[19] = (content_length >> 20) & (0xff);
        auth_layer[20] = (content_length >> 12) & (0xff);
        auth_layer[21] = (content_length >> 4) & (0xff);
        auth_layer[22] = (content_length << 4) & (0xf0);

        // dump array in the file
        wf.write((char *) &auth_layer, (255 - (232 - content_length))*sizeof(uint8_t));
        wf.close();
    }
    
    cout << "Success.";
    
    rf.close();

    return 0;
}