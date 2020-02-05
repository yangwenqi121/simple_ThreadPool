#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Winsock2.h>
#pragma comment(lib,"Ws2_32.lib")
#include <stdio.h>
#define  MAX_LENGTH 50
int main(void)
{
	/*initialize winsock2 library*/
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		return 1;
	/*step1 create a socket */
	SOCKET sock=::socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
		return 1;
	/*step2 connect to server*/
	sockaddr_in addr_server = { 0 };
	addr_server.sin_addr.S_un.S_addr = inet_addr("192.168.0.108");
	addr_server.sin_family = AF_INET;
	addr_server.sin_port = htons(3000);
	if (::connect(sock, reinterpret_cast<const sockaddr*>(&addr_server),
		sizeof addr_server) == SOCKET_ERROR)
		return 1;
	/*step3 receive and send data via socket*/
	/*prepare to receive */
	char buf[MAX_LENGTH];
	if (::recv(sock, buf, MAX_LENGTH, 0) > 0)
		wprintf(reinterpret_cast<const wchar_t*>(buf));
	/*step 4 close socket*/
	::closesocket(sock);
	sock = INVALID_SOCKET;
	::WSACleanup();
	return 0;
}