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

        roboSocket = new MySocket(CLIENT, roboIp, (unsigned int)roboPort, UDP, (unsigned int)0); 
        if (roboSocket) {
            res.code = 200;
            res.write("Bound Socket");
        } else {
            res.code = 500;
            res.write("Failed to Bind Socket");
        }
        res.end();
    });

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
            telepkt->SetLength(8);
            int length = roboSocket->GetData(data);
            cout << "Got " << length << " bytes: " << string(data, length) << endl;
        
            delete telepkt;
            res.end();
            });

    CROW_ROUTE(app, "/telementry_request")
    ([&roboSocket](const request &req, response &res)
    {
        PktDef sendPkt;
        sendPkt.SetCmd(CmdType::RESPONSE);
        sendPkt.CalcCRC();
        char* sendPacket = sendPkt.GenPacket();

        roboSocket->SendData(sendPacket, sendPkt.GetLength());

        char buffer[16];
        int bytesReceived = roboSocket->GetData(buffer);

        PktDef recvPkt(buffer);
        char* recvBody = recvPkt.GetBodyData();

        unsigned short int lastPacketCounter = recvBody[0];
        unsigned short int currentGrade = recvBody[4];
        unsigned short int hitCount = recvBody[8];
        unsigned char lastCmd = recvBody[12];
        unsigned char lastCmdValue = recvBody[13];
        unsigned char lastCmdSpeed = recvBody[14];
    
        res.code = 200;
        res.end(); });

    app.port(PORT_NUMBER)
        .multithreaded()
        .run();

    return 0;
}
