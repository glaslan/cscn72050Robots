#define CROW_MAIN

#include "crow_all.h"
#include <iostream>

using namespace std;
using namespace crow;

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

    // Home Page
    CROW_ROUTE(app, "/")
    ([](const request &req, response &res)
    { sendHtml(res, "./public/index"); });

    CROW_ROUTE(app, "/connect/<string>/<int>").methods(HTTPMethod::POST)
    ([](const request &req, response &res, string ip, int port)
    {
        
    }
    );

    CROW_ROUTE(app, "/telecommand").methods(HTTPMethod::PUT)
    ([](const request &req, response &res)
    {
        
    }
    );

    CROW_ROUTE(app, "/telecommand_request")
    ([](const request &req, response &res)
    {
        
    }
    );

    app.port(PORT_NUMBER)
        .multithreaded()
        .run();

    return 0;
}
