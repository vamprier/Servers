/*
 * Stun.cpp
 *
 *  Created on: Mar 14, 2016
 *      Author: root
 */

#include "Stun.h"

Stun::Stun(u_16 port):Server(port)
{
	// TODO Auto-generated constructor stub

}

Stun::~Stun()
{
	// TODO Auto-generated destructor stub
}

//===========================================
//MainProc函数说明
//函数功能： 线程调用函数
//参数：    lpParameter：传入参数，包括scoket连接、buffer数组下标、文件名等信息
//函数返回： 无
//===========================================
void* Stun::MainProc(void* lpParameter)
{
	Socket portSocket = ((RECVPARAM*)lpParameter)->sock;
	u_16 number = ((RECVPARAM*)lpParameter)->number;
	Stun* pThis = (Stun*)((RECVPARAM*)lpParameter)->pThis;
	bool isok;
	while(1)
	{
		//第一步：查找状态为0的buffer
		int findNumber = -1;
		isok = pThis->FindBuffer(&(pThis->buffer[number][0]),MAX_MESSAGE_NUMBER,&findNumber,AVAILABLE_BUFFER_FLAG);
		if( isok) //找到了
		{
			NetAddr from;
			int msgSize = sizeof( RequestMessagePackage);
			bool ret = getMessage(portSocket,(char*)(&(pThis->buffer[number][findNumber].rmsg)),&msgSize,&from.ip,&from.port);
			if ( ret) //接收到数据
			{
				//记录收到包的总数
				++pThis->totalRecvNumber;
				//判断消息格式是否正确
				if( CheckRequestMessage(&( pThis->buffer[number][findNumber].rmsg)) &&
						 pThis->buffer[number][findNumber].rmsg.dataType == REQUEST_TYPE)
				{
					memset( pThis->writeChar,0x00,MSG_TOTAL_LENGTH);
					sprintf( pThis->writeChar,"Received Buffer From IP:[ %d ], PORT: [ %d ]\n",from.ip,from.port);
					 pThis->Log[number].Log( pThis->writeChar);
					//该buffer的状态置为2
					 pThis->buffer[number][findNumber].flag = SEND_BUFFER_FLAG;
					//填写该buffer的信息
					 pThis->buffer[number][findNumber].rmsg.messageContent.localNatAddr.ip = from.ip;
					 pThis->buffer[number][findNumber].rmsg.messageContent.localNatAddr.port = from.port;
				}
			}
		}
		//查找状态为2的buffer,找到后发送
		isok =  pThis->FindAndSend(&(pThis->buffer[number][0]),MAX_MESSAGE_NUMBER,portSocket,number);
		if( isok)
		{
			 pThis->Log[number].Log("Find Send Buffer,Starting Send Message......\n");
		}
		usleep(1);
	}
	return NULL;
}

void* Stun::SendProc(void* lpParameter)
{
	Socket portSocket = ((RECVPARAM*)lpParameter)->sock;
	u_16 port = ((RECVPARAM*)lpParameter)->startPort;
	Stun* pThis = (Stun*)((RECVPARAM*)lpParameter)->pThis;
	while(1)
	{
		bool isok = pThis->SendRecvAndSendNum(portSocket,10023);
		//cout<<isok<<" , "<<pThis->GettotalRecv()<<" , "<<pThis->GettotalSend()<<endl;
		usleep(1);
	}
	return NULL;
}

//===========================================
//ThreadProc函数说明
//函数功能：打开端口，并创建线程
//参数：    startPort：起始端口号
//		   portNumber：打开的端口个数
//函数返回： 成功或者失败
//===========================================
bool Stun::ThreadProc( u_16 startPort, u_16 portNumber)
{
	if (portNumber == 0)
	{
		return false;
	}
	char time[256];
	for ( int i=0;i<portNumber;i++)
	{
		//打开端口
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
		char filename[512];
		sprintf(filename,"%d %s.log",startPort+i,time);
		Log[i].CreateFile(filename);
		//接收发送线程
		int ret ;
		if( i < SOCKET_NUMBER)
		{
			ret = pthread_create(&threadHandle[i],NULL,MainProc,&pRecvParam[i]);
		}
		else
		{
			ret = pthread_create(&threadHandle[i],NULL,SendProc,&pRecvParam[i]);
		}
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

void Stun::start()
{
	ThreadProc( localPort, SOCKET_NUMBER+1);
}

