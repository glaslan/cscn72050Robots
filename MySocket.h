#include <stdio.h> 
#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


#pragma once
//#pragma warning(disable : 4996) // Use this if theres Deprecated warnings that the define won't fix

namespace TermProject {

	enum SocketType {
		CLIENT,
		SERVER
	};

	enum ConnectionType {
		TCP,
		UDP
	};

	const int DEFAULT_SIZE = 128; // CHANGE LATER?

	class MySocket
	{
	private:
		char* Buffer;
		int WelcomeSocket; // TCP/IP Server
		int ConnectionSocket; // TCP and UDP

		sockaddr_in SvrAddr;
		SocketType mySocket;
		std::string IPAddr;
		int Port;
		ConnectionType connectionType;
		bool bTCPConnect;
		int MaxSize;

	public:
		MySocket(SocketType, std::string, unsigned int, ConnectionType, unsigned int);
		~MySocket();

		void ConnectTCP();
		void DisconnectTCP();

		void SendData(const char*, int);
		int GetData(char*);

		std::string GetIPAddr() const;
		void SetIPAddr(std::string);

		void SetPort(int);
		int GetPort() const;

		SocketType GetType() const;
		void SetType(SocketType);
	};

}