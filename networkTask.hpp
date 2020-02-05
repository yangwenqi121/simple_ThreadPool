#pragma once
#include<WinSock2.h>
#include "ITask.h"
#include <stdio.h>
using namespace threading;

class NetWorkTask:public ITask
{
public:
	NetWorkTask(SOCKET sock,SOCKADDR_IN addr):_sock(sock),_addr(addr)
	{

	}
	void execute(void)
	{
		/*step 5 sending and receiving data */
		::send(_sock, reinterpret_cast<const char*>(L"hello TCP IP"),
			(wcslen(L"hello TCP IP") + 1) * sizeof(wchar_t), 0);
		printf("IP:%s,port:%i\n", inet_ntoa(_addr.sin_addr), ntohs(_addr.sin_port));

		/*step 6 close socket */
		::closesocket(_sock);
		_sock = INVALID_SOCKET;
	}
private:
	SOCKET _sock;
	SOCKADDR_IN _addr;
};