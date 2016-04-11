/*
 * Server.cpp
 *
 *  Created on: Mar 14, 2016
 *      Author: root
 */

#include "Server.h"

Server::Server(u_16 port):localPort(port)
{
	// TODO Auto-generated constructor stub
	totalRecvNumber = 0;
	totalSendNumber = 0;
}

Server::~Server()
{
	// TODO Auto-generated destructor stub
}

int Server::GettotalRecv()
{
	return totalRecvNumber;
}

int Server::GettotalSend()
{
	return totalSendNumber;
}

bool Server::SendRecvAndSendNum(Socket fd,u_16 port)
{
	MessagePackage msg;
	msg.headFlag = PACKAGE_HEADER_FLAG;
	msg.dataType = TRANSFORM_TYPE;
//	char ch[256];
//	sprintf(ch,"%d %d\0",totalRecvNumber,totalSendNumber);
//	memcpy(msg.messageContent.dataContent,ch,strlen(ch));
	msg.messageContent.localAddr.ip = totalRecvNumber;
	msg.messageContent.remoteAddr.ip = totalSendNumber;
//	msg.realMessageSize = strlen(ch);
	msg.tailFlag = PACKAGE_TAIL_FLAG;
	int msgSize = sizeof(MessagePackage);
	u_32 localIp;
	char name[256];
	GetLocalIp(&localIp,name);
	return sendMessage(fd,(char*)(&msg),msgSize,localIp,port);
}

//===========================================
//FindBuffer函数说明
//函数功能：查找MessageBuffer中状态为findflag的下标
//参数：    buffer：MessageBuffer数组
//			bufferNumber：MessageBuffer长度
//			findNumber：找到并返回的下标
//			findflag：查找条件
//函数返回：true： 函数执行成功
//			false：函数执行失败
//===========================================
bool Server::FindBuffer(MessageBuffer* buffer, u_32 bufferNumber, int* findNumber,u_8 findflag)
{
	*findNumber = -1;
	for (u_32 i=0;i<bufferNumber;i++)
	{
		if (buffer[i].flag == findflag)
		{
			*findNumber = i;
			break;
		}
	}
	if (*findNumber == -1)
	{
		return false;
	}
	return true;
}

//===========================================
//FindAndSend函数说明
//函数功能： 查找状态为2的buffer，如果找到就发送数据
//参数：    buffer: buffer 数组
//		   bufferSize: buffer大小
//		   portSocket: socket连接
//		   fileName: 文件名
//函数返回：    无
//===========================================
bool Server::FindAndSend(MessageBuffer* buffer, u_16 bufferSize, Socket portSocket,u_16 number)
{
	int findN = -1;
	bool ret = FindBuffer(buffer,bufferSize,&findN,SEND_BUFFER_FLAG);
	if ( ret) // 找到了
	{
		int msgSize = sizeof( RequestMessagePackage);
		//发送
		ret = sendMessage(portSocket,(char*)(&(buffer[findN].rmsg)),msgSize,
			buffer[findN].rmsg.messageContent.localNatAddr.ip,
			buffer[findN].rmsg.messageContent.localNatAddr.port);
		//发送完后，该buffer的状态置为0
		buffer[findN].flag = AVAILABLE_BUFFER_FLAG;
		++totalSendNumber;
		usleep(1);
		return true;
	}
	return false;
}

