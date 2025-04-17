#define CROW_MAIN

#include "crow_all.h"
#include <iostream>
#include "MySocket.h"
#include "PktDef.h"

using namespace std;
using namespace crow;
using namespace TermProject;

struct TelemetryResponse
{
    unsigned short int lastPacketCounter;
    unsigned short int currentGrade;
    unsigned short int hitCount;
    unsigned char lastCmd;
    unsigned char lastCmdValue;
    unsigned char lastCmdSpeed;
};

void sendFile(response &res, string filename, string contentType)
{
    ifstream in("../" + filename, ifstream::in);
    if (in)
    {
        ostringstream contents;
        contents << in.rdbuf();
        in.close();
        res.write(contents.str());
        res.set_header("Content-Type", contentType);
    }
    else
    {
        res.write("Not Found " + filename);
        res.code = 404;
    }
    res.end();
}

void sendHtml(response &res, string filename)
{
    sendFile(res, filename + ".html", "text/html");
}

void sendScript(response &res, string filename)
{
    sendFile(res, filename + ".js", "text/javascript");
}

void sendStyle(response &res, string filename)
{
    sendFile(res, filename + ".css", "text/css");
}

int main()
{
    crow::SimpleApp app;

    const int PORT_NUMBER = 23500;

    string roboIp = "0.0.0.0";
    int roboPort = 0;
    MySocket *roboSocket = nullptr;
    ConnectionType connectType = UDP;

    // Home Page
    CROW_ROUTE(app, "/")
    ([](const request &req, response &res)
     { sendHtml(res, "./public/index"); });

    CROW_ROUTE(app, "/connect/<string>/<int>/<string>").methods(HTTPMethod::POST)([&roboIp, &roboPort, &roboSocket, &connectType](const request &req, response &res, string ip, int port, string type)
                                                                                  {
        roboIp = ip;
        roboPort = port;
        if (type == "TCP") connectType = TCP;
        else connectType = UDP;

        if (roboSocket) delete roboSocket;
        roboSocket = new MySocket(CLIENT, roboIp, (unsigned int)roboPort, connectType, (unsigned int)0); 
        if (roboSocket) {
            if (connectType == TCP) roboSocket->ConnectTCP();

            res.code = 200;
            res.write("Bound Socket");
        } else {
            res.code = 500;
            res.write("Failed to Bind Socket");
        }
        res.end(); });

    CROW_ROUTE(app, "/telecommand/<string>")
        .methods(HTTPMethod::PUT)([&roboSocket](const request &req, response &res, string cmd)
                                  {
            PktDef* telepkt = new PktDef();

            if (cmd == "SLEEP") {
                telepkt->SetCmd(CmdType::SLEEP);
            } else {
                const char* dir = req.url_params.get("direction");
                const char* dur = req.url_params.get("duration");
                const char* spd = req.url_params.get("speed");
        
                if (!(dir && dur && spd)) {
                    res.code = 400;
                    res.write("Missing parameters");
                    res.end();
                    return;
                }
        
                char bodyData[3] = {
                    static_cast<char>(std::stoi(dir)),
                    static_cast<char>(std::stoi(dur)),
                    static_cast<char>(std::stoi(spd))
                };
        
                telepkt->SetCmd(CmdType::DRIVE);
                telepkt->SetBodyData(bodyData, 3);
            }
        
            telepkt->CalcCRC();
            char* packet = telepkt->GenPacket();
            int len = telepkt->GetLength();
            roboSocket->SendData(packet, len);
        
            string lengthSent = "Sent " + cmd + " packet that was " + std::to_string(len) + " bytes large";
            res.write(lengthSent);
        
            char data[10];
            int length = roboSocket->GetData(data);
            string lengthRecieved = "Got " + to_string(length) + " bytes back";
            res.write(lengthRecieved);
            PktDef recvPkt(data);
        
            if (!recvPkt.GetAck()) {
                res.write("Failed to receive ack");
                res.code = 400;
                delete telepkt;
                res.end();
                return;
            }

            res.write("Received ack");

        
            delete telepkt;
            res.code = 200;
            res.end(); });

    CROW_ROUTE(app, "/telementry_request")
    ([&roboSocket](const request &req, response &res)
     {
        PktDef sendPkt;
        sendPkt.SetCmd(CmdType::RESPONSE);
        sendPkt.CalcCRC();
        char* sendPacket = sendPkt.GenPacket();

        roboSocket->SendData(sendPacket, sendPkt.GetLength());

        char bufferOne[16];
        int firstBytesReceived = roboSocket->GetData(bufferOne);
        PktDef recvPktOne(bufferOne);
        if (!recvPktOne.GetAck()) {
            res.code = 400;
            res.end();
            return;
        }

        char bufferTwo[16];
        int secondBytesReceived = roboSocket->GetData(bufferTwo);

        PktDef recvPktTwo(bufferTwo);
        char* recvBody = recvPktTwo.GetBodyData();

        TelemetryResponse response;
        std::memcpy(&response, recvBody, sizeof(TelemetryResponse));
        
        res.write("lastPacketCounter: " + to_string(response.lastPacketCounter));
        res.write("\ncurrentGrade: " + to_string(response.currentGrade));
        res.write("\nhitCount: " + to_string(response.hitCount));
        res.write("\nlastCmd: " + to_string(response.lastCmd));
        res.write("\nlastCmdValue: " + to_string(response.lastCmdValue));
        res.write("\nlastCmdSpeed: " + to_string(response.lastCmdSpeed));
        
        res.code = 200;
        res.end(); });

    app.port(PORT_NUMBER)
        .multithreaded()
        .run();

    return 0;
}
