#pragma once
#include <winsock2.h>

#include "Socket.h"

constexpr int MaxReceiveLength = 1024;

enum class EOperationType {
	ACCEPT,
	RECV,
	SEND,
	NONE
};

struct IOContext
{
	OVERLAPPED Overlapped;
	EOperationType OperationType;

	IOContext() {
		ZeroMemory(&Overlapped, sizeof(Overlapped));
		OperationType = EOperationType::NONE;
	}
};

struct OverlappedContext : IOContext {
	WSABUF WsaBuf;
	char ReceiveBuffer[MaxReceiveLength];

	OverlappedContext(EOperationType opType){
		OperationType = opType;
		WsaBuf.buf = ReceiveBuffer;
		WsaBuf.len = MaxReceiveLength;
		ZeroMemory(ReceiveBuffer, MaxReceiveLength);
	}
};

struct AcceptContext : IOContext {
	char buffer[2 * (sizeof(SOCKADDR_STORAGE) + 16)];
	Socket clientSocket;

	AcceptContext() {
		OperationType = EOperationType::ACCEPT;
		ZeroMemory(buffer, sizeof(buffer));
	}
};
