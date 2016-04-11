/*
 * Server.h
 *
 *  Created on: Mar 14, 2016
 *      Author: root
 */

#ifndef SERVER_H_
#define SERVER_H_

#include "typedef.h"
#include "logfile.h"
#include <time.h>
#include <map>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <fstream>

using namespace std;

class Server;

struct RECVPARAM
{
	Socket sock;  //
	u_16    number;
	u_16    startPort;
	Server* pThis;
};

typedef struct
{
	u_8	flag; //
	RequestMessagePackage rmsg; //
}MessageBuffer;

class Server {
public:
	Server(u_16 port);
	virtual ~Server();
public:
	int GettotalRecv();
	int GettotalSend();
protected:
	bool FindBuffer(MessageBuffer* buffer, u_32 bufferNumber, int* findNumber,u_8 findflag);
	bool FindAndSend(MessageBuffer* buffer, u_16 bufferSize, Socket portSocket,u_16 number);
	bool SendRecvAndSendNum(Socket fd,u_16 port);
protected:
	 size_t totalRecvNumber;//接收的数据总数
	 size_t totalSendNumber;//发送的数据总数
	 pthread_t threadHandle[SOCKET_NUMBER+1]; //线程数组
	 RECVPARAM pRecvParam[SOCKET_NUMBER+1];//
	 MessageBuffer buffer[SOCKET_NUMBER][MAX_MESSAGE_NUMBER];//
	 Logger Log[SOCKET_NUMBER];
	 char writeChar[MSG_TOTAL_LENGTH];
	 u_16 localPort;
	 char temprmsg[MSG_TOTAL_LENGTH];
	 RequestMessagePackage rmsg;
	 MessagePackage msg;
public:
	virtual void start()=0;
};

#endif /* SERVER_H_ */
