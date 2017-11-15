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
			//�������ݰ��ĳ���
			client->myrecv((char *)&len, sizeof(int));
			//�������ݰ��������������
			client->myrecv(package, len);
		}
		catch (SocketException e) {
			//�������ݰ�ʱ�����쳣����Ҫ�ȶϿ����ӣ������������Ϣ
			client->myDisconnect(TRUE);
			break;
		}

		int ip;
		short port;

		switch (package[0]) {
		case 0:  //���ݰ�Ϊʱ����Ӧ
			client->timeResponse(package + 1, len - 1);
			break;
		case 1:  //���ݰ�Ϊ������Ӧ
			client->nameResponse(package + 1, len - 1);
			break;
		case 2:  //���ݰ�Ϊ�ͻ��б�
			client->listResponse(package + 1, len - 1);
			break;
		case 3:  //���ݰ�Ϊ�յ���Ϣ
			ip = *(int *)(package + 1);
			port = *(short *)(package + 5);
			client->getMessage(ip, port, package + 7, len - 7);
			break;
		case 4:  //���ݰ�Ϊ������Ϣ�ķ���
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
	SOCKET sClient; //�����׽���
	struct sockaddr_in saServer;//��ַ��Ϣ

								//WinSock��ʼ����
	wVersionRequested = MAKEWORD(2, 2);//ϣ��ʹ�õ�WinSock DLL�İ汾
	ret = WSAStartup(wVersionRequested, &wsaData);
	if (ret != 0)
		throw SocketException(1);

	//ȷ��WinSock DLL֧�ְ汾2.2��
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		throw SocketException(2);
	}

	//����socket��ʹ��TCPЭ�飺
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sClient == INVALID_SOCKET)
	{
		WSACleanup();
		throw SocketException(3);
	}

	//������������ַ��Ϣ��
	saServer.sin_family = AF_INET;//��ַ����
	saServer.sin_port = htons(SERVER_PORT);//ע��ת��Ϊ�����ֽ���
	saServer.sin_addr.S_un.S_addr = inet_addr(ip);


	//���ӷ�������
	ret = connect(sClient, (struct sockaddr *)&saServer, sizeof(saServer));
	if (ret == SOCKET_ERROR)
	{
		closesocket(sClient);//�ر��׽���
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

	//MessageBox(CString("�����ѶϿ�"), CString("��Ϣ"));
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
	s = string("���ʱ�䣺 ") + string(time);
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
	s = string("����������� ") + string(name);
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
		u = string("�û�") + string(num) + string("   ") + group[i].toString();
		ui->list_add(u.c_str());
	}

	ui->add_text("�ͻ��б��ѻ�ȡ���");
}

void Client::sendResponse(char retValue)
{
	//cout << "Send response " << (int)retValue << endl;
	string s = "������Ϣ״̬�� ";
	switch (retValue) {
	case 21:
		s += "���ͳɹ�";
		break;
	case 22:
		s += "����ʧ�ܣ�ԭ��: id������Χ";
		break;
	case 23:
		s += "����ʧ��, ԭ��: �û��ѵǳ�";
		break;
	case 24:
		s += "����ʧ��, ԭ��: �����ֽڹ���";
		break;
	default:
		s += "����ʧ����ԭ����";
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

	string rtn = string("��") + otherClient.toString() + string("���յ���Ϣ�� ") + string(mesg);
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
