#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>

#pragma comment(lib,"Ws2_32.lib")

#define PORT      7000

int main()
{
	std::string name;
	std::cout << "Enter your Name : ";
	std::cin >> name;
	name += " : ";

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "Fail WSA init" << std::endl;
		return 0;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		std::cout << "Fail Socket Create" << std::endl;
		return 0;
	}

	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
	serverAddr.sin_port = htons(PORT);

	if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		int errCode = WSAGetLastError();
		std::cout << "Connect ErrorCode: " << errCode << std::endl;
		return 0;
	}

	std::cout << "connect !!" << std::endl;

	std::thread* sendThread = new std::thread([&]() {
		while (true) {
			char inputBuffer[100];
			std::cin >> inputBuffer;

			char sendBuffer[100];
			memcpy(sendBuffer, name.c_str(), name.length());
			memcpy(sendBuffer + name.length(), inputBuffer, sizeof(inputBuffer));

			int resultCode = send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);
			if (resultCode == SOCKET_ERROR) {
				int errCode = WSAGetLastError();
				std::cout << "Send ErrorCode: " << errCode << std::endl;
				return 0;
			}

			std::cin.ignore();
		}
	});

	while (true) {
		char recvBuffer[100];
		int recvSize = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (recvSize >= 0) {
			std::cout << recvBuffer << std::endl;
		}
	}

	closesocket(clientSocket);
	WSACleanup();
	
	return 0;
}

