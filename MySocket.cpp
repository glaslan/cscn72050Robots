#include "MySocket.h"

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

using namespace TermProject;

MySocket::MySocket(SocketType socketType, std::string ipAddress, unsigned int port, ConnectionType connectType, unsigned int size = 0)
{
	mySocket = socketType;
	IPAddr = ipAddress;
	Port = port;
	connectionType = connectType;

	if (size > 0)
		MaxSize = size; // Default if size 0 is given
	else
		MaxSize = DEFAULT_SIZE;

	bTCPConnect = false;
	ConnectionSocket = INVALID_SOCKET;
	WelcomeSocket = INVALID_SOCKET;

	Buffer = new char[MaxSize];

	SvrAddr.sin_family = AF_INET;
	SvrAddr.sin_port = htons(Port);
	SvrAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());

	if (connectionType == TCP && socketType == SERVER)
	{ // Server TCP
		WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (bind(WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr)) == -1)
		{
			std::cout << "ERROR: Failed to bind TCP server socket" << std::endl;
			return;
		}

		listen(WelcomeSocket, 1);
	}
	else if (connectionType == TCP)
	{ // Client TCP
		ConnectionSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else
	{ // UDP Connection
		ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (socketType == SERVER)
		{
			bind(ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr));
		}
	}
}

TermProject::MySocket::~MySocket()
{
	delete[] Buffer;
	close(WelcomeSocket);
	close(ConnectionSocket);
}

void TermProject::MySocket::ConnectTCP()
{
	if (connectionType == UDP)
	{
		std::cerr << "ERROR: Cannot establish a UDP connection through TCP" << std::endl;
		return;
	}

	if (GetType() == SERVER)
	{ // SERVER uses TCP
		ConnectionSocket = accept(WelcomeSocket, NULL, NULL);
		bTCPConnect = ConnectionSocket != SOCKET_ERROR;
	}
	else
	{
		if (connect(ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR)
		{
			std::cerr << "ERROR: Connection attempted failed" << std::endl;
			return;
		}
		else
		{
			bTCPConnect = true;
		}
	}
}

void TermProject::MySocket::DisconnectTCP()
{
	if (connectionType == UDP || !bTCPConnect)
	{
		std::cerr << "ERROR: Cannot disconnect a UDP connection through TCP" << std::endl;
		return;
	}

	close(ConnectionSocket);
	bTCPConnect = false;
}

void TermProject::MySocket::SendData(const char *data, int size)
{
	int result;
	if (connectionType == TCP && bTCPConnect)
		send(ConnectionSocket, data, size, 0);
	else if (connectionType == UDP)
		result = sendto(ConnectionSocket, data, size, 0, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr));
	std::cout << result << std::endl;
}

int TermProject::MySocket::GetData(char *buffer)
{
	int bytesWritten = 0;

	if (connectionType == TCP && bTCPConnect)
	{
		bytesWritten = recv(ConnectionSocket, Buffer, MaxSize, 0);
	}
	else if (connectionType == UDP)
	{
		socklen_t addr_len = sizeof(SvrAddr);
		bytesWritten = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr *)&SvrAddr, &addr_len);
	}

	if (bytesWritten > 0)
	{
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
	if (bTCPConnect)
	{
		std::cout << "Cannot change IP Address while there is a connection established" << std::endl;
		return;
	}

	IPAddr = ipAddress;
}

void TermProject::MySocket::SetPort(int port)
{
	if (bTCPConnect)
	{
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
	if (bTCPConnect)
	{
		std::cout << "Cannot change Socket Type while there is a connection established" << std::endl;
		return;
	}

	mySocket = socketType;
}
