/*
 * Turn.cpp
 *
 *  Created on: Mar 14, 2016
 *      Author: root
 */

#include "Turn.h"

Turn::Turn(u_16 port) : Server(port)
{
	// TODO Auto-generated constructor stub

}

Turn::~Turn() {
	// TODO Auto-generated destructor stub
}

//===========================================
//MainProc函数说明
//函数功能： 线程调用函数
//参数：    lpParameter：传入参数，包括scoket连接、buffer数组下标、文件名等信息
//函数返回： 无
//===========================================
void* Turn::MainProc(void* lpParameter)
{
	Socket portSocket = ((RECVPARAM*)lpParameter)->sock;
	u_16 number = ((RECVPARAM*)lpParameter)->number;
	Turn* pThis = (Turn*)((RECVPARAM*)lpParameter)->pThis;
	while(1)
	{
		//第一步：查找状态为0的buffer
		int findNumber = -1;
		bool isok = pThis->FindBuffer(&(pThis->buffer[number][0]),MAX_MESSAGE_NUMBER,&findNumber,AVAILABLE_BUFFER_FLAG);
		if( isok) //找到了
		{
			NetAddr from;
			int msgSize = MSG_TOTAL_LENGTH;
			memset(pThis->temprmsg,0x00,msgSize);
			bool ret = getMessage(portSocket,(char*)(&(pThis->temprmsg)),&msgSize,&from.ip,&from.port);
			if ( ret) //接收到数据
			{
				//记录收到包的总数
				++pThis->totalRecvNumber;
				u_8 dataType;
				memcpy(&dataType,&pThis->temprmsg[1],sizeof(u_8));
				if(dataType == REQUEST_TYPE) //请求Nat信息
				{
					memcpy(&(pThis->buffer[number][findNumber].rmsg),&pThis->temprmsg[0],sizeof(RequestMessagePackage));
					if( CheckRequestMessage(&(pThis->buffer[number][findNumber].rmsg)))
					{
						memset(pThis->writeChar,0x00,MSG_TOTAL_LENGTH);
						sprintf(pThis->writeChar,"Request Received Buffer From IP:[ %d ], PORT: [ %d ]\n",from.ip,from.port);
						pThis->Log[number].Log(pThis->writeChar);
						//该buffer的状态置为2
						pThis->buffer[number][findNumber].flag = SEND_BUFFER_FLAG;
						//填写该buffer的信息
						pThis->buffer[number][findNumber].rmsg.messageContent.localNatAddr.ip = from.ip;
						pThis->buffer[number][findNumber].rmsg.messageContent.localNatAddr.port = from.port;
					}
				}
				else if(dataType == EXCHANG_TYPE) //握手过程
				{
					memset(pThis->writeChar,0x00,MSG_TOTAL_LENGTH);
					sprintf(pThis->writeChar,"Exchange Received Buffer From IP:[ %d ], PORT: [ %d ]\n",from.ip,from.port);
					pThis->Log[number].Log(pThis->writeChar);
					int rmsgSize = sizeof(RequestMessagePackage);
					memset(&(pThis->rmsg),0x00,rmsgSize);
					memcpy(&(pThis->rmsg),&(pThis->temprmsg[0]),rmsgSize);
					if( CheckRequestMessage(&(pThis->rmsg)))
					{
						usleep(1);
						sendMessage(portSocket,(char*)(&(pThis->rmsg)),rmsgSize,
								pThis->rmsg.messageContent.remoteNatAddr.ip,
								pThis->rmsg.messageContent.remoteNatAddr.port);
						memset(pThis->writeChar,0x00,MSG_TOTAL_LENGTH);
						sprintf(pThis->writeChar,"Exchange Send TO IP:[ %d ], PORT: [ %d ]\n",
								pThis->rmsg.messageContent.remoteNatAddr.ip,
								pThis->rmsg.messageContent.remoteNatAddr.port);
						pThis->Log[number].Log(pThis->writeChar);
					}
				}
				else if(dataType == TRANSFORM_TYPE) //传输数据
				{
					memset(pThis->writeChar,0x00,MSG_TOTAL_LENGTH);
					sprintf(pThis->writeChar,"Transform Received Buffer From IP:[ %d ], PORT: [ %d ]\n",from.ip,from.port);
					pThis->Log[number].Log(pThis->writeChar);
					int msgSize = sizeof(MessagePackage);
					memset(&(pThis->msg),0x00,msgSize);
					memcpy(&(pThis->msg),&(pThis->temprmsg[0]),msgSize);
					if( CheckMessage(&(pThis->msg)))
					{
						usleep(1);
						sendMessage(portSocket,(char*)(&(pThis->msg)),msgSize,
								pThis->msg.messageContent.remoteNatAddr.ip,
								pThis->msg.messageContent.remoteNatAddr.port);
						memset(pThis->writeChar,0x00,MSG_TOTAL_LENGTH);
						sprintf(pThis->writeChar,"Transform Send TO IP:[ %d ], PORT: [ %d ]\n",
								pThis->msg.messageContent.remoteNatAddr.ip,
								pThis->msg.messageContent.remoteNatAddr.port);
						pThis->Log[number].Log(pThis->writeChar);
					}
				}
			}
		}
		//查找状态为2的buffer,找到后发送
		isok = pThis->FindAndSend(&(pThis->buffer[number][0]),MAX_MESSAGE_NUMBER,portSocket,number);
		if( isok)
		{
			pThis->Log[number].Log("Find Send Buffer,Starting Send Message......\n");
		}
		usleep(1);
	}
	return NULL;
}

//===========================================
//ThreadProc函数说明
//函数功能： 打开端口，启动线程
//参数：    startPort：起始端口
//		   portNumber：打开端口的个数
//函数返回： 成功或者失败
//===========================================
bool Turn::ThreadProc( u_16 startPort, u_16 portNumber)
{
	if (portNumber == 0)
	{
		return false;
	}
	char time[256];
	for ( int i=0;i<portNumber;i++)
	{
		Socket localSocket = openPort(startPort+i,0);
		if (localSocket == INVALID_SOCKET)
		{
			continue;
		}
		pRecvParam[i].sock = localSocket;
		pRecvParam[i].number = i;
		pRecvParam[i].startPort = startPort+i;
		pRecvParam[i].pThis = this;
		GetDate(&time[0]);
		char writeChar[1024];
		sprintf(writeChar,"%d %s.log",startPort+i,time);
		Log[i].CreateFile(writeChar);
		//接收发送线程
		int ret = pthread_create(&threadHandle[i],NULL,MainProc,&pRecvParam[i]);
		if( ret != 0)
		{
			cout<<"false"<<endl;
			continue;
		}
		usleep(1);
	}
	for(int i=0;i<portNumber;i++)
	{
		pthread_join(threadHandle[i],NULL);
	}
	for ( int i=0;i<SOCKET_NUMBER;i++)
	{
		if ( pRecvParam[i].sock != INVALID_SOCKET)
		{
			CloseSocket( pRecvParam[i].sock);
		}
	}
	return true;
}

void Turn::start()
{
	ThreadProc( localPort, SOCKET_NUMBER);
}

