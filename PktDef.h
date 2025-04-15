/*
Milestone #1
In this milestone you will create a PktDef class that defines and implements the robots application layer
protocol. Your class implementation shall be tested using MSTest and a sequence of Unit Tests defined
by you.
*/

#pragma once
#include <memory>
#include <cstring>
#include <cassert>

#define FORWARD 1
#define BACKWARD 2
#define RIGHT 3
#define LEFT 4
#define HEADERSIZE 4
#define CRCSIZE 1

enum CmdType {
    UNKNOWN,
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
    unsigned char Length;

	Header() : PktCount(0), Drive(0), Status(0), Sleep(0), Ack(0), Padding(0), Length(0) {} // Default constructor

} Head; // 4 bytes large

struct DriveBody {
    char Direction; // 1=FORWARD, 2=BACKWARD, 3=RIGHT, 4=LEFT
    unsigned char Duration; // number of seconds
    char Speed; // 0-100 (anything under 80 is probably too low)
}; // 3 bytes large

class PktDef
{
private:
    struct CmdPacket {
        Header header;
        char* data;
        char crc;

        CmdPacket() : data(nullptr), crc(0) { memset(&header, 0, sizeof(Header)); }
    };

    CmdPacket cmdPacket = {};
    size_t dataSize = 0;
    char* rawBuffer = nullptr;
    size_t rawBufferSize = 0;

    // Initilize DriveBody struct with default values
    DriveBody InitDriveBody();

    // Helper functions
    // Convert variables to binary to count number of ones in the variable
    int countBinaryOnes(unsigned short int val);
    int countBinaryOnes(unsigned char val);
    int countBinaryOnes(char val);

public:
    /*
    A default constructor that places the PktDef object in a safe state, defined as
    follows:
     All Header information set to zero
     Data pointer set to nullptr
     CRC set to zero
    */
    PktDef();
    /*
    An overloaded constructor that takes a RAW data buffer, parses the
    data and populates the Header, Body, and CRC contents of the PktDef object
    */
    PktDef(char* src);
    // A set function that sets the packets command flag based on the CmdType
    void SetCmd(CmdType cmd);
    /*
    a set function that takes a pointer to a RAW data buffer
    and the size of the buffer in bytes. This function will allocate the packets Body field and
    copies the provided data into the objects buffer
    */
    void SetBodyData(char* dataBuffer, int size);
    // a set function that sets the objects PktCount header variable
    void SetPktCount(int);
    // a query function that returns the CmdType based on the set command flag bit
    CmdType GetCmd();
    // a query function that returns True/False based on the Ack flag in the header
    bool GetAck();
    // a query function that returns the packet Length in bytes
    int GetLength();
    // a query function that returns a pointer to the objects Body field
    char* GetBodyData();
    // Converts char* data into the DriveBody struct to reference robot controls easier
    DriveBody GetDriveBody();
    // a query function that returns the PktCount value
    int GetPktCount();
    /*
    a function that takes a pointer to a RAW data buffer, the
    size of the buffer in bytes, and calculates the CRC. If the calculated CRC matches the
    CRC of the packet in the buffer the function returns TRUE, otherwise FALSE.
    */
    bool CheckCRC(char*, int);
    // a function that calculates the CRC and sets the objects packet CRC parameter.
    void CalcCRC();
    /*
    a function that allocates the private RawBuffer and transfers the
    contents from the objects member variables into a RAW data packet (RawBuffer) for
    transmission. The address of the allocated RawBuffer is returned.
    */
    char* GenPacket();
};