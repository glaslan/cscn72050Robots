/*
Milestone #1
In this milestone you will create a PktDef class that defines and implements the robots application layer
protocol. Your class implementation shall be tested using MSTest and a sequence of Unit Tests defined
by you.
*/
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
    unsigned char Length;

    Header() : PktCount(0), Drive(0), Status(0), Sleep(0), Ack(0), Padding(0), Length(0) {}
};

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

        CmdPacket() : header(), Data(nullptr), CRC(0) {}
    };

public:
    char* RawBuffer;


    PktDef() {
        CmdPacket* pkt_ptr = new CmdPacket();
    }

};