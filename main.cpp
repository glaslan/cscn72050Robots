#define CROW_MAIN

#include "crow_all.h"
#include <iostream>
#include "MySocket.h"
#include "PktDef.h"

using namespace std;
using namespace crow;
using namespace TermProject;

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

    // Home Page
    CROW_ROUTE(app, "/")
    ([](const request &req, response &res)
     { sendHtml(res, "./public/index"); });

    CROW_ROUTE(app, "/connect/<string>/<int>").methods(HTTPMethod::POST)([&roboIp, &roboPort, &roboSocket](const request &req, response &res, string ip, int port)
                                                                         {
        roboIp = ip;
        roboPort = port;

        roboSocket = new MySocket(CLIENT, ip, (unsigned int)port, UDP, (unsigned int)0); });

    CROW_ROUTE(app, "/telecommand/<string>")
        .methods(HTTPMethod::PUT)([&roboSocket](const request &req, response &res, string cmd)
                                  {
            if (cmd == "SLEEP") {
                PktDef pkt;
                pkt.SetCmd(CmdType::SLEEP);
                pkt.CalcCRC();
                char* packet = pkt.GenPacket();
        
                roboSocket->SendData(packet, sizeof(packet));
            } 
            else {
                bool isValid = req.url_params.get("direction") && 
                               req.url_params.get("duration") &&
                               req.url_params.get("speed");
        
                if (!isValid) {
                    res.code = 400;
                    res.write("Missing parameters");
                    res.end();
                    return;
                }
        
                int direction = std::stoi(req.url_params.get("direction"));
                int duration = std::stoi(req.url_params.get("duration"));
                int speed = std::stoi(req.url_params.get("speed"));
        
                char* bodyData = new char[direction, duration, speed];

                PktDef pkt;
                pkt.SetCmd(CmdType::DRIVE);
                pkt.SetBodyData(bodyData, sizeof(bodyData));
                pkt.CalcCRC();
                char* packet = pkt.GenPacket();
        
                roboSocket->SendData(packet, sizeof(packet));
            }
        
            res.code = 200;
            res.end(); });

    CROW_ROUTE(app, "/telementry_request")
    ([&roboSocket](const request &req, response &res)
     {
        PktDef sendPkt;
        sendPkt.SetCmd(CmdType::RESPONSE);
        sendPkt.CalcCRC();
        char* sendPacket = sendPkt.GenPacket();

        roboSocket->SendData(sendPacket, sizeof(sendPacket));

        char* buffer = nullptr;
        int bytesReceived = roboSocket->GetData(buffer);

        PktDef recvPkt(buffer);
        char* recvBody = recvPkt.GetBodyData();

        unsigned short int lastPacketCounter = recvBody[0];
        unsigned short int currentGrade = recvBody[4];
        unsigned short int hitCount = recvBody[8];
        unsigned short int lastCmd = recvBody[12];
        unsigned short int lastCmdValue = recvBody[13];
        unsigned short int lastCmdSpeed = recvBody[14];
    
        res.code = 200;
        res.end(); });

    app.port(PORT_NUMBER)
        .multithreaded()
        .run();

    return 0;
}
