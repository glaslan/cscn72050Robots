#include "PktDef.h"
#include <iostream>

PktDef::PktDef()
{
    // Initilize base values
    memset(&cmdPacket.header, 0, HEADERSIZE);
    cmdPacket.header.Length = HEADERSIZE + CRCSIZE; // Setting it to the default size
    cmdPacket.data = nullptr;
    cmdPacket.crc = 0;
    rawBuffer = nullptr;
}

PktDef::PktDef(char *src) : PktDef()
{
    assert(src != nullptr);

    // Copy header
    char *tmp = src;
    memcpy(&cmdPacket.header, tmp, HEADERSIZE);
    tmp += HEADERSIZE;

    assert((int)cmdPacket.header.Length >= HEADERSIZE + CRCSIZE);

    // Copy body
    if ((int)cmdPacket.header.Length > (HEADERSIZE + CRCSIZE))
    {
        int size = ((int)cmdPacket.header.Length - HEADERSIZE) - CRCSIZE;
        cmdPacket.data = new char[size];
        memcpy(cmdPacket.data, tmp, size);
        tmp += size;
    }
    else
    {
        cmdPacket.data = nullptr;
    }

    // Copy CRC
    memcpy(&cmdPacket.crc, tmp, CRCSIZE);
}

PktDef::~PktDef()
{
    if (cmdPacket.data)
        delete[] cmdPacket.data;
    if (rawBuffer)
        delete[] rawBuffer;
}

void PktDef::SetCmd(CmdType cmd)
{
    // Start by setting Header values to 0
    cmdPacket.header.Drive = 0;
    cmdPacket.header.Status = 0;
    cmdPacket.header.Sleep = 0;
    cmdPacket.header.Ack = 0;

    // Depending on command, set header
    switch (cmd)
    {
    case CmdType::DRIVE:
        cmdPacket.header.Drive = 1;
        cmdPacket.header.Ack = 1;
        break;
    case CmdType::SLEEP:
        cmdPacket.header.Sleep = 1;
        cmdPacket.header.Ack = 1;
        break;
    case CmdType::RESPONSE:
        cmdPacket.header.Status = 1;
        cmdPacket.header.Ack = 1;
        break;
    default:
        break;
    }
}

void PktDef::SetBodyData(char *dataBuffer, int size)
{
    assert(dataBuffer != nullptr);
    assert(size > 0);

    if (cmdPacket.data)
        delete[] cmdPacket.data;
    cmdPacket.data = new char[size];
    memcpy(cmdPacket.data, dataBuffer, size);
    cmdPacket.header.Length = HEADERSIZE + size + CRCSIZE;
}

void PktDef::SetPktCount(int count)
{
    cmdPacket.header.PktCount = count;
}

CmdType PktDef::GetCmd()
{
    // Returns CmdType depending on Header information
    int flagSet = 0;
    if (cmdPacket.header.Drive)
        flagSet += 1;
    else if (cmdPacket.header.Sleep)
        flagSet += 2;
    else if (cmdPacket.header.Status)
        flagSet += 4;

    switch (flagSet)
    {
    case 1:
        return CmdType::DRIVE;
    case 2:
        return CmdType::SLEEP;
    case 4:
        return CmdType::RESPONSE;
    default:
        std::cout << "GetCmd(): CmdPacket cmd couldn't be identified." << std::endl;
        return CmdType::UNKNOWN;
    }
}

bool PktDef::GetAck()
{
    return (bool)cmdPacket.header.Ack;
}

int PktDef::GetLength()
{
    return (int)cmdPacket.header.Length;
}

char *PktDef::GetBodyData()
{
    if (!cmdPacket.data)
        return nullptr;
    return cmdPacket.data;
}

void PktDef::SetLength(int length)
{
    cmdPacket.header.Length = (unsigned char)length;
}

DriveBody PktDef::GetDriveBody()
{
    // Initilize with base data incase cmdPacket.data is null
    DriveBody driveBody = InitDriveBody();

    if (cmdPacket.data && (cmdPacket.header.Length - HEADERSIZE - CRCSIZE) == 3)
    {
        driveBody.Direction = cmdPacket.data[0];
        driveBody.Duration = (unsigned char)cmdPacket.data[1];
        driveBody.Speed = cmdPacket.data[2];
    }

    return driveBody;
}

bool PktDef::CheckCRC(char *buffer, int size)
{

    assert(buffer != nullptr);

    if (size != cmdPacket.header.Length)
        return false; // Buffer size doesn't match up

    // Get count of all ones in buffer
    int count = 0;
    for (int i = 0; i < size - 1; i++)
    {
        count += countBinaryOnes(buffer[i]);
    }

    // Get expected CRC from end of buffer
    char bufferCRC = buffer[size - 1];

    return count == (int)bufferCRC;
}

void PktDef::CalcCRC()
{
    int count = 0; // The amount of 1's found in the packet

    // Header
    unsigned short pktCount = cmdPacket.header.PktCount;
    count += countBinaryOnes((unsigned char)(pktCount & 0xFF));
    count += countBinaryOnes((unsigned char)((pktCount >> 8) & 0xFF));

    // Stick the flags into one byte to be counted
    unsigned char flags = 0;
    flags |= (cmdPacket.header.Drive & 0x01) << 7;
    flags |= (cmdPacket.header.Status & 0x01) << 6;
    flags |= (cmdPacket.header.Sleep & 0x01) << 5;
    flags |= (cmdPacket.header.Ack & 0x01) << 4;

    count += countBinaryOnes(flags);
    count += countBinaryOnes(cmdPacket.header.Length);

    // Body
    if (cmdPacket.data)
    {
        int bodySize = cmdPacket.header.Length - HEADERSIZE - CRCSIZE;
        for (int i = 0; i < bodySize; i++)
        {
            count += countBinaryOnes(cmdPacket.data[i]);
        }
    }

    // CRC
    cmdPacket.crc = (unsigned char)count;
}

char *PktDef::GenPacket()
{
    size_t totalSize = cmdPacket.header.Length;

    char *buffer = new char[totalSize];

    // Copy Header information
    memcpy(buffer, &cmdPacket.header, HEADERSIZE);

    // If data is valid, copy body data
    if (cmdPacket.data)
    {
        memcpy(buffer + HEADERSIZE, cmdPacket.data, cmdPacket.header.Length - HEADERSIZE - CRCSIZE);
    }

    // Copy CRC to end of packet
    memcpy(buffer + (totalSize - CRCSIZE), &cmdPacket.crc, CRCSIZE);
    rawBuffer = buffer;
    return buffer;
}

int PktDef::GetPktCount()
{
    return cmdPacket.header.PktCount;
}

DriveBody PktDef::InitDriveBody()
{
    DriveBody db;

    db.Direction = 0;
    db.Duration = 0;
    db.Speed = 0;

    return db;
}

int PktDef::countBinaryOnes(unsigned short int val)
{
    int count = 0;

    // Traverse unsigned short int (16 bits) and count amount of 1's
    for (int i = 0; i < 16; i++)
    {
        if (val % 2 == 1)
        {
            count++;
        }
        val /= 2;
    }

    return count;
}

int PktDef::countBinaryOnes(unsigned char val)
{
    int count = 0;

    // Traverse unsigned char (8 bits) and count amount of 1's
    for (int i = 0; i < 8; i++)
    {
        if (val % 2 == 1)
        {
            count++;
        }
        val /= 2;
    }

    return count;
}

int PktDef::countBinaryOnes(char val)
{
    // Turned signed char to unsigned to avoid negative values
    unsigned char unsignedVal = static_cast<unsigned char>(val);

    return countBinaryOnes(unsignedVal);
}