//#include <WinSock2.h>
//#include <ws2tcpip.h>
//#include <iostream>
//#include <thread>
//#include <vector>
//
//#pragma comment(lib,"Ws2_32.lib")
//
//#define MAX_PATH  256
//#define PORT      7000
//#define MAX_BUF   100
//
//std::vector<std::thread*> vecClientThreads;
//std::vector<SOCKET> vecClientSoket;
//
//void clientWork(const SOCKET& socket);
//std::wstring getIP();
//
//int main() {
//	bool isListen = true;
//
//	WSADATA wsaData;
//	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//		std::cout << "Fail WSA init" << std::endl;
//		return 0;
//	}
//
//	getIP();
//
//	SOCKET listenSoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (listenSoket == INVALID_SOCKET) {
//		std::cout << "Fail Socket Create" << std::endl;
//		return 0;
//	}
//
//	SOCKADDR_IN serverAddr;
//	serverAddr.sin_family = AF_INET;
//	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
//	serverAddr.sin_port = htons(PORT);
//
//	int result = bind(listenSoket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
//	if (result == SOCKET_ERROR) {
//		closesocket(listenSoket);
//		std::cout << "Fail Socket Bind" << std::endl;
//		return 0;
//	}
//
//	char serverIP[16];
//	inet_ntop(AF_INET, &serverAddr.sin_addr, serverIP, sizeof(serverIP));
//	std::cout << "Server IP: " << serverIP << std::endl;
//
//	std::thread* listenThread = new std::thread([&]() {
//		while (true) {
//			if (listen(listenSoket, 10) == SOCKET_ERROR) {
//				std::cout << "Fail Socket listen" << std::endl;
//				closesocket(listenSoket);
//				break;
//			}
//
//			SOCKADDR_IN clientAddr;
//			memset(&clientAddr, 0, sizeof(clientAddr));
//			int addrSize = sizeof(clientAddr);
//
//			SOCKET clientSocket = accept(listenSoket, (SOCKADDR*)&clientAddr, &addrSize);
//			if (clientSocket == INVALID_SOCKET) {
//				std::cout << "ClientSoket Error" << std::endl;
//				break;
//			}
//
//			char clientIP[16];
//			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
//			std::cout << "Client Connected. IP: " << clientIP << std::endl;
//
//			vecClientSoket.emplace_back(clientSocket);
//			std::thread* cliThread = new std::thread(&clientWork, clientSocket);
//			vecClientThreads.emplace_back(cliThread);
//		}
//		isListen = false;
//		return 0;
//	});
//
//	while (isListen) {
//		Sleep(100);
//	}
//
//	WSACleanup();
//	return 0;
//}
//
//void clientWork(const SOCKET& socket) {
//	while (true) {
//		char buf[MAX_BUF];
//		int recvSize = recv(socket, buf, MAX_BUF, 0);
//		if (recvSize < 0) {
//			break;
//		}
//
//		std::cout << "data : " << buf << std::endl;
//
//		for (SOCKET& socket : vecClientSoket) {
//			int resultCode = send(socket, buf, sizeof(buf), 0);
//			if (resultCode == SOCKET_ERROR) {
//				int errCode = WSAGetLastError();
//				std::cout << "Send ErrorCode: " << errCode << std::endl;
//			}
//		}
//	}
//	return;
//}
//
//std::wstring getIP() {
//	char host[MAX_PATH] = { 0, };
//	int result = gethostname(host, sizeof(host));
//
//	char port_str[16] = { 0, };
//	sprintf_s(port_str, "%d", 7000);
//	addrinfo hint, * pResult;
//	memset(&hint, 0, sizeof(hint));
//
//	result = getaddrinfo(host, "7000", &hint, &pResult);
//
//	addrinfo* ptr = nullptr;
//	sockaddr_in* pIpv4;
//	sockaddr_in6* pIpv6;
//
//	wchar_t ip_str[46];
//	DWORD ip_size = sizeof(ip_str);
//	memset(&ip_str, 0, ip_size);
//
//	std::wstring ip;
//
//	for (ptr = pResult; ptr != NULL; ptr = ptr->ai_next)
//	{
//		switch (ptr->ai_family)
//		{
//		case AF_UNSPEC:
//			break;
//		case AF_INET:
//			pIpv4 = (struct sockaddr_in*)ptr->ai_addr;
//			//inet_ntoa(pIpv4->sin_addr);
//			InetNtopW(AF_INET, &pIpv4->sin_addr, ip_str, ip_size);
//			ip = ip_str;
//			break;
//		case AF_INET6:
//			// the InetNtop function is available on Windows Vista and later
//			pIpv6 = (struct sockaddr_in6*)ptr->ai_addr;
//			InetNtopW(AF_INET6, &pIpv6->sin6_addr, ip_str, ip_size);
//			break;
//		case AF_NETBIOS:
//			break;
//		default:
//			break;
//		}
//	}
//
//	return std::wstring();
//}