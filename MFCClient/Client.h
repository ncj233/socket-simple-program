#pragma once
#include "stdafx.h"
#include <string>
//#include "MFCClientDlg.h"

class CMFCClientDlg;

#define MAX_USER 32

struct OtherClients {
	int ip;
	short port;

	OtherClients(int ip, short port) {
		this->ip = ip;
		this->port = port;
	}

	OtherClients() : ip(0), port(0) {}

	inline std::string toString() {
		struct in_addr addr;
		addr.S_un.S_addr = ip;
		char sport[10];
		sprintf_s(sport, 10, "%d", htons(port));
		return std::string(inet_ntoa(addr)) + ':' + sport;
	}
};

class Client
{
public:
	Client();
	~Client();
	void start(char *ip);
	void myrecv(char *mesg, int len);
	void myDisconnect(BOOL b = false);
	void mysend(int type);
	void mysend(int type, int id, const char *mesg);
	void timeResponse(char *t, int len);
	void nameResponse(char *str, int len);
	void listResponse(char *data, int len);
	void sendResponse(char retValue);
	void getMessage(int ip, short port, char *str, int len);

	CMFCClientDlg *ui;

private:
	void myConnect(char *ip);

	OtherClients group[MAX_USER];
	BOOL groupVaild;
	int userCnt;

	SOCKET handle;
	BOOL socketValid;
};

class SocketException {
public:
	SocketException(int i) : errid(i) { }
	std::string getErrorType();
private:
	int errid;
};