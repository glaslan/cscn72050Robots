#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "MySocket.h"

using namespace TermProject;

MySocket::MySocket(SocketType socketType, std::string ipAddress, unsigned int port, ConnectionType connectType, unsigned int size = 0) {
	mySocket = socketType;
	IPAddr = ipAddress;
	Port = port;
	connectionType = connectType;

	if (size > 0) MaxSize = size; // Default if size 0 is given
	else MaxSize = DEFAULT_SIZE;

	bTCPConnect = false;
	ConnectionSocket = INVALID_SOCKET;
	WelcomeSocket = INVALID_SOCKET;

	Buffer = new char[MaxSize];

	// Windows Socket init
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "ERROR: Failed to initialize Winsock" << std::endl;
		return;
	}

	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(Port);
	SvrAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());

	if (connectionType == TCP && socketType == SERVER) { // Server TCP
		WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
			std::cout << "ERROR: Failed to bind TCP server socket" << std::endl;
			return;
		}

		listen(WelcomeSocket, 1);
	}
	else if (connectionType == TCP) { // Client TCP
		ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else { // UDP Connection
		ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (socketType == SERVER) {
			if (bind(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
				std::cout << "ERROR: Failed to bind UDP server socket" << std::endl;
				return;
			}
		}
	}

}

TermProject::MySocket::~MySocket()
{
	delete[] Buffer;
	closesocket(WelcomeSocket);
	closesocket(ConnectionSocket);
	WSACleanup();
}

void TermProject::MySocket::ConnectTCP()
{
	if (connectionType == UDP) {
		std::cerr << "ERROR: Cannot establish a UDP connection through TCP" << std::endl;
		return;
	}

	if (GetType() == SERVER) { // SERVER uses TCP
		ConnectionSocket = accept(WelcomeSocket, NULL, NULL);
		bTCPConnect = ConnectionSocket != SOCKET_ERROR;
	}
	else {
		if (connect(ConnectionSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR) {
			std::cerr << "ERROR: Connection attempted failed" << std::endl;
			return;
		}
		else {
			bTCPConnect = true;
		}
	}
}

void TermProject::MySocket::DisconnectTCP()
{
	if (connectionType == UDP || !bTCPConnect) {
		std::cerr << "ERROR: Cannot disconnect a UDP connection through TCP" << std::endl;
		return;
	}

	closesocket(ConnectionSocket);
	bTCPConnect = false;
}

void TermProject::MySocket::SendData(const char* data, int size)
{
	if (connectionType == TCP && bTCPConnect) send(ConnectionSocket, data, size, 0);
	else if (connectionType == UDP) sendto(ConnectionSocket, data, size, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
}

int TermProject::MySocket::GetData(char* buffer)
{
	int bytesWritten = 0;

	if (connectionType == TCP && bTCPConnect) {
		bytesWritten = recv(ConnectionSocket, Buffer, MaxSize, 0);
	}
	else if (connectionType == UDP) {
		int addr_len = sizeof(SvrAddr);
		bytesWritten = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &addr_len);
	}

	if (bytesWritten > 0) {
		memcpy(buffer, Buffer, bytesWritten);
	}

	return bytesWritten;
}

std::string TermProject::MySocket::GetIPAddr() const
{
	return IPAddr;
}

void TermProject::MySocket::SetIPAddr(std::string ipAddress)
{
	if (bTCPConnect) {
		std::cout << "Cannot change IP Address while there is a connection established" << std::endl;
		return;
	}

	IPAddr = ipAddress;
}

void TermProject::MySocket::SetPort(int port)
{
	if (bTCPConnect) {
		std::cout << "Cannot change Port while there is a connection established" << std::endl;
		return;
	}

	Port = port;
}

int TermProject::MySocket::GetPort() const
{
	return Port;
}

SocketType TermProject::MySocket::GetType() const
{
	return mySocket;
}

void TermProject::MySocket::SetType(SocketType socketType)
{
	if (bTCPConnect) { 
		std::cout << "Cannot change Socket Type while there is a connection established" << std::endl;
		return;
	}

	mySocket = socketType;
}
