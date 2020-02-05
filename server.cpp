#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "networkTask.hpp"
#include <stdio.h>
#pragma comment(lib,"Ws2_32.lib")
#include "ThreadPool.h"
using namespace threading;

int main(void)
{
	ThreadPool::getSingleton()->Initialize(10, 20);
	/*step 0.initialize winsock2 library*/
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		return 1;
	/*step1 create a socket AF=Address Family ,PF=Protocol Family*/
	SOCKET sock = ::socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		return 1;
	/*step2 bind socket to a specified address and port*/
	sockaddr_in addr_local = { 0 };
	/*fill out the structure of address*/
	addr_local.sin_addr.S_un.S_addr = ::inet_addr("192.168.0.108");
	addr_local.sin_family = AF_INET;
	addr_local.sin_port = htons(3000); //将htons是将整型变量
	//从主机字节顺序转变成网络字节顺序， 就是整数在地址空间存储方式变为高位字节存放在内存的低地址处。

	if (::bind(sock, reinterpret_cast<const sockaddr*>(&addr_local), sizeof addr_local) == SOCKET_ERROR)
		return 1;
	/*step3 listen*/
	if(::listen(sock, 5)==SOCKET_ERROR)
		return 1;
	while (true)
	{
		/*step 4 accept*/
		sockaddr_in sockaddr_client = { 0 };
		int length = sizeof sockaddr_client;
		/*receive a new working socket for transferring data*/
		SOCKET sock_client = ::accept(sock, reinterpret_cast<sockaddr*>(&sockaddr_client), &length);
		if (sock_client == INVALID_SOCKET)
			return 1;
		
		ITask* task = new NetWorkTask(sock_client, sockaddr_client);
		ThreadPool::getSingleton()->addTask(std::shared_ptr<ITask>(task));
	}
	
	
	::closesocket(sock);

	sock = INVALID_SOCKET;
	::WSACleanup();
		return 0;
}




