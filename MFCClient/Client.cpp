#include "stdafx.h"
#include "Client.h"
#include <iostream>
#include <cstring>
#include "MFCClientDlg.h"

#define SERVER_PORT 5015

using namespace std;

DWORD WINAPI receiveThread(LPVOID para) {
	Client *client = (Client *)para;

	int len;
	char package[1024];

	while (1) {
		try {
			//接收数据包的长度
			client->myrecv((char *)&len, sizeof(int));
			//接收数据包除长度外的内容
			client->myrecv(package, len);
		}
		catch (SocketException e) {
			//接收数据包时发生异常，需要先断开连接，并弹出相关消息
			client->myDisconnect(TRUE);
			break;
		}

		int ip;
		short port;

		switch (package[0]) {
		case 0:  //数据包为时间响应
			client->timeResponse(package + 1, len - 1);
			break;
		case 1:  //数据包为名字响应
			client->nameResponse(package + 1, len - 1);
			break;
		case 2:  //数据包为客户列表
			client->listResponse(package + 1, len - 1);
			break;
		case 3:  //数据包为收到消息
			ip = *(int *)(package + 1);
			port = *(short *)(package + 5);
			client->getMessage(ip, port, package + 7, len - 7);
			break;
		case 4:  //数据包为发送消息的返回
			client->sendResponse(package[1]);
			break;
		default:
			break;
		}
	}

	return NULL;
}

string SocketException::getErrorType()
{
	std::string ret;
	switch (errid) {
	case 1: ret = string("WSAStartup() failed!"); break;
	case 2: ret = string("Invalid Winsock version!"); break;
	case 3: ret = string("socket() failed!"); break;
	case 4: ret = string("connect() failed!"); break;
	case 5: ret = string("recv() failed!"); break;
	case 6: ret = string("send() failed!"); break;
	default: ret = string("unkown error"); break;
	}
	return ret;
}

Client::Client()
{
	socketValid = FALSE;
	groupVaild = FALSE;
	userCnt = 0;
}


Client::~Client()
{
	myDisconnect();
}

void Client::start(char *ip)
{
	//try {
	myConnect(ip);
	/*}
	catch (SocketException e) {
		cout << e.getErrorType() << endl;
	}*/

	//cout << "start" << endl;

	CreateThread(NULL, 0, receiveThread, (LPVOID)this, 0, NULL);
}

void Client::myConnect(char *ip)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret;
	SOCKET sClient; //连接套接字
	struct sockaddr_in saServer;//地址信息

								//WinSock初始化：
	wVersionRequested = MAKEWORD(2, 2);//希望使用的WinSock DLL的版本
	ret = WSAStartup(wVersionRequested, &wsaData);
	if (ret != 0)
		throw SocketException(1);

	//确认WinSock DLL支持版本2.2：
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		throw SocketException(2);
	}

	//创建socket，使用TCP协议：
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		WSACleanup();
		throw SocketException(3);
	}

	//构建服务器地址信息：
	saServer.sin_family = AF_INET;//地址家族
	saServer.sin_port = htons(SERVER_PORT);//注意转化为网络字节序
	saServer.sin_addr.S_un.S_addr = inet_addr(ip);


	//连接服务器：
	ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		closesocket(sClient);//关闭套接字
		WSACleanup();
		throw SocketException(4);
	}

	handle = sClient;
	socketValid = TRUE;
}

void Client::myDisconnect(BOOL b)
{
	if (socketValid) {
		closesocket(handle);
		socketValid = false;
	}
	WSACleanup();

	if (b) {
		ui->disconnect();
	}

	//MessageBox(CString("连接已断开"), CString("消息"));
}

void Client::timeResponse(char *t, int len)
{
	char time[20];
	/* if (len != 19)
		cout << "invaild time string" << endl; */
	for (int i = 0; i < len; i++)
		time[i] = t[i];
	time[len] = 0;

	string s;
	s = string("获得时间： ") + string(time);
	ui->add_text(s.c_str());
}

void Client::nameResponse(char *str, int len)
{
	char name[256];
	for (int i = 0; i < len; i++)
		name[i] = str[i];
	name[len] = 0;
	
	//cout << "Receive host name: " << name << endl;

	string s;
	s = string("获得主机名： ") + string(name);
	ui->add_text(s.c_str());
}

void Client::listResponse(char * data, int len)
{
	int ip;
	short port;
	userCnt = len / 6;
	for (int i = 0; i < userCnt; i++) {
		ip = *(int *)(data + i * 6);
		port = *(short *)(data + i * 6 + 4);
		group[i] = OtherClients(ip, port);
	}

	//cout << "Receive list" << endl;
	ui->clear_list();
	for (int i = 0; i < userCnt; i++) {
		char num[12];
		string u;
		sprintf_s(num, 11, "%d", i + 1);
		u = string("用户") + string(num) + string("   ") + group[i].toString();
		ui->list_add(u.c_str());
	}

	ui->add_text("客户列表已获取完毕");
}

void Client::sendResponse(char retValue)
{
	//cout << "Send response " << (int)retValue << endl;
	string s = "发送消息状态： ";
	switch (retValue) {
	case 21:
		s += "发送成功";
		break;
	case 22:
		s += "发送失败，原因: id超出范围";
		break;
	case 23:
		s += "发送失败, 原因: 用户已登出";
		break;
	case 24:
		s += "发送失败, 原因: 发送字节过长";
		break;
	default:
		s += "发送失败且原因不明";
		break;
	}

	ui->add_text(s.c_str());

}

void Client::getMessage(int ip, short port, char *s, int len)
{
	char *mesg = new char[len + 1];
	memcpy(mesg, s, len);
	mesg[len] = 0;

	OtherClients otherClient(ip, port);

	string rtn = string("从") + otherClient.toString() + string("接收到消息： ") + string(mesg);
	ui->add_text(rtn.c_str());
	//cout << "Receive message from " << otherClient.toString() << endl;
	//cout << mesg << endl;

	delete[]mesg;
}

void Client::myrecv(char * mesg, int len)
{
	int ret;
	while (len > 0) {
		ret = (int)recv(handle, mesg, len, 0);
		if (ret <= 0)
			throw SocketException(5);
		len = len - ret;
		mesg = mesg + ret;
	}
}

void Client::mysend(int type)
{
	char package[5];
	int ret;
	*(int *)package = 1;
	package[4] = type;

	ret = send(handle, package, 5, 0);
	if (ret == SOCKET_ERROR)
		throw SocketException(6);
}

void Client::mysend(int type, int id, const char *mesg)
{
	int ret;
	int mesgLen = strlen(mesg);
	char *package = new char[mesgLen + 8];
	*(int *)package = mesgLen + 3;
	package[4] = type;
	*(short *)(package + 5) = (short)id;
	strcpy_s(package + 7, mesgLen + 1, mesg);
	//strcpy(package + 7, mesg);

	ret = send(handle, package, mesgLen + 7, 0);

	delete[]package;

	if (ret == SOCKET_ERROR)
		throw SocketException(6);
}
