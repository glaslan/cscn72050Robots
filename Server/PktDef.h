/*
Milestone #1
In this milestone you will create a PktDef class that defines and implements the robots application layer
protocol. Your class implementation shall be tested using MSTest and a sequence of Unit Tests defined
by you.
*/

#pragma once
#include <memory>

#define FORWARD 1
#define BACKWARD 2
#define RIGHT 3
#define LEFT 4
#define HEADERSIZE 4


enum CmdType {
    DRIVE,
    SLEEP,
    RESPONSE
};

struct Header
{
    unsigned short int PktCount;
    char Drive : 1;
    char Status : 1;
    char Sleep : 1;
    char Ack : 1;
    char Padding : 4;
    unsigned short int Length : 8;
} Head;

struct DriveBody {
    char Direction; // 1=FORWARD, 2=BACKWARD, 3=RIGHT, 4=LEFT
    unsigned char Duration; // number of seconds
    char Speed; // 0-1 (anything under 0.8 is probably too low)
};

class PktDef
{
private:
    struct CmdPacket {
        Header header;
        char* Data;
        char CRC;
    }CmdPacket;

public:
    char* RawBuffer;


    PktDef() : RawBuffer(nullptr) {memset(&CmdPacket, 0, sizeof(CmdPacket)); CmdPacket.header.PktCount = 0; CmdPacket.header.Length = sizeof(DriveBody);};

    PktDef(char* src)
    {
        RawBuffer = src;

        memcpy(&CmdPacket.header, RawBuffer, sizeof(Header));
        RawBuffer += sizeof(Header);

        if ((int)CmdPacket.header.Length > 0) {
            CmdPacket.Data = new char[CmdPacket.header.Length];
            memcpy(CmdPacket.Data, RawBuffer, (int)CmdPacket.header.Length);
            RawBuffer += (int)CmdPacket.header.Length;
        }
        else {
            CmdPacket.Data = nullptr;
        }

        memcpy(&CmdPacket.CRC, RawBuffer, sizeof(char));
        RawBuffer += sizeof(char); // This should take us to the end of the buffer
    };
};