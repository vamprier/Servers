/*
 * Message.cpp
 *
 *  Created on: Mar 14, 2016
 *      Author: root
 */

#include "Message.h"

Message::Message(u_16 port,double t):Server(port),timeout(t)
{
	// TODO Auto-generated constructor stub
}

Message::~Message()
{
	// TODO Auto-generated destructor stub
}

//===========================================
//ChangeMessage函数说明
//函数功能：交换包信息
//参数：    mspNeedChangeLeft：需要交换信息的数据包
//		    mspNeedChangeRight：需要交换信息的数据包
//函数返回： 成功或者失败
//===========================================
void
Message::ChangeMessage( RequestMessagePackage* mspNeedChangeLeft, RequestMessagePackage* mspNeedChangeRight)
{
	mspNeedChangeLeft->messageContent.remoteAddr.ip = mspNeedChangeRight->messageContent.localAddr.ip;
	mspNeedChangeLeft->messageContent.remoteAddr.port = mspNeedChangeRight->messageContent.localAddr.port;
	mspNeedChangeLeft->messageContent.remoteNatAddr.ip = mspNeedChangeRight->messageContent.localNatAddr.ip;
	mspNeedChangeLeft->messageContent.remoteNatAddr.port = mspNeedChangeRight->messageContent.localNatAddr.port;
   //交换信息
	mspNeedChangeRight->messageContent.remoteAddr.ip = mspNeedChangeLeft->messageContent.localAddr.ip;
	mspNeedChangeRight->messageContent.remoteAddr.port = mspNeedChangeLeft->messageContent.localAddr.port;
	mspNeedChangeRight->messageContent.remoteNatAddr.ip = mspNeedChangeLeft->messageContent.localNatAddr.ip;
	mspNeedChangeRight->messageContent.remoteNatAddr.port = mspNeedChangeLeft->messageContent.localNatAddr.port;
}

//===========================================
//WaitMessage函数说明
//函数功能：主消息循环，接收并交换信息
//参数：    localPortSocket：socket连接
//		   timeOut：超时时间
//函数返回： 无
//===========================================
void
Message::WaitMessage( Socket localPortSocket, double timeOut)
{
   //主消息循环
	while (1)
	{
		int mspSize = sizeof( RequestMessagePackage);
		NetAddr from;
		memset( &rmsg, 0x00, mspSize);
		bool isok =  getMessage( localPortSocket, (char*)(&rmsg), &mspSize, &from.ip, &from.port);
		//获得消息接收时间
		time_t startTime = GetTime();
		//消息接收成功
		if ( isok)
		{
			//记录收到包的总数
			++totalRecvNumber;
			//收到地址交换请求数据包后，判断包头、包尾格式,解析配对信息，在队列中查找是否存在同样配对的请求信息
			if( CheckRequestMessage(&rmsg) && rmsg.dataType == EXCHANG_TYPE)
			{
				//如果接收到信息的ip和port已经存在的话，不做处理直接进入下次循环
				vector<pair<unsigned int,unsigned short> >::iterator vit = find(ipPortVec.begin(),ipPortVec.end(),make_pair(from.ip, from.port));
				if( vit != ipPortVec.end())
				{
					continue;
				}
				else
				{
					ipPortVec.push_back(make_pair(from.ip, from.port));
				}
				memset(writeChar,0x00,MSG_TOTAL_LENGTH);
				sprintf(writeChar,"receive from : [ %d ] [%d ]\n",from.ip,from.port);
				Log[0].Log(writeChar);
				cout<<rmsg.messageContent.pairingFlag<<endl;
				//在不可用地址中查找配对码
				map<unsigned int,int>::iterator it = unAvailabeAddress.find( rmsg.messageContent.pairingFlag);
				//找到同样配对信息，将双方信息打包后发送给双方，并且在消息队列中删除找到的配对信息
				if ( it != unAvailabeAddress.end())
				{
					cout<<"find same pairing message from list\n";
					int index = it->second;
					//找到配对信息后，从不可用map中将该地址删除
					unAvailabeAddress.erase(it++);
					//可用地址中加入该地址
					AvailabeAddress.insert(make_pair( messageVector[index].msg.messageContent.pairingFlag, index));
					//交换信息
					ChangeMessage(&rmsg,&( messageVector[index].msg));
					usleep(1);
					bool issendA = sendMessage(localPortSocket, (char*)(&rmsg), mspSize, from.ip, from.port);
					memset(writeChar,0x00,MSG_TOTAL_LENGTH);
					sprintf(writeChar,"sendMessage to [ %d ] [%d ] is %d\n",from.ip,from.port,issendA);
					Log[0].Log(writeChar);
					usleep(1);
					bool issendB = sendMessage(localPortSocket, (char*)(&messageVector[index]), mspSize, messageVector[index].ip, messageVector[index].port);
					memset(writeChar,0x00,MSG_TOTAL_LENGTH);
					sprintf(writeChar,"sendMessage to [ %d ] [%d ] is %d\n",
							messageVector[index].ip,messageVector[index].port,issendB);
					Log[0].Log(writeChar);
					//ipPortVec中删除配对成功的信息
					vit = find(ipPortVec.begin(),ipPortVec.end(),make_pair(from.ip, from.port));
					ipPortVec.erase(vit++);
					vit = find(ipPortVec.begin(),ipPortVec.end(),make_pair(messageVector[index].ip, messageVector[index].port));
					ipPortVec.erase(vit++);
					totalSendNumber +=2;
				}//end if
				else //未找到配对信息
				{
					//如果消息队列的数量没有到达上限
					if ( AvailabeAddress.size() != 0)
					{
						//收到的消息加入到消息队列后，不可用地址的map增加1,可用地址的map减1
						map<unsigned int,int>::iterator it = AvailabeAddress.begin();
						int index = it->second;
						messageVector[index].msg = rmsg;
						messageVector[index].msgTime = startTime;
						messageVector[index].ip = from.ip;
						messageVector[index].port = from.port;
						unAvailabeAddress.insert( make_pair(rmsg.messageContent.pairingFlag, index));
						AvailabeAddress.erase( it++);
						cout<<"add message to list\n";
					}
					else
					{
						Log[0].Log("message list has reached the limit\n");
					}
				}
			}//end if
		}//end if
		else //接收消息失败
		{
			Log[0].Log("receive message false\n");
			time_t endTime = GetTime();
			for ( map<unsigned int,int>::iterator it = unAvailabeAddress.begin();it != unAvailabeAddress.end();)
			{
				int index = it->second;
				double diff = difftime( endTime, messageVector[index].msgTime);
				if ( diff >= timeOut)
				{
					Log[0].Log("time out\n");
					int index = it->second;
					unAvailabeAddress.erase(it++);
					AvailabeAddress.insert(make_pair( messageVector[index].msg.messageContent.pairingFlag, index));
				}
				else
				{
					++it;
				}
			}//end for
		}//end else
	}//end while
}

//===========================================
//StartServer函数说明
//函数功能：***
//参数：    ***
//			***
//			***
//函数返回：***
//===========================================
bool
Message::StartServer( unsigned short localPort, double timeOut)
{
	//UDP服务器打开消息服务端口号进行监听
	Socket localPortSocket;
	localPortSocket = openPort( localPort, 0);
	bool isok = !( localPortSocket == INVALID_SOCKET);
	if (isok)
	{
		//清空内存
		memset( messageVector, 0, sizeof(MessageContent)*MAG_NUMBER);
		//可用地址队列赋值
		for ( int i=0; i<MAG_NUMBER; i++)
		{
			AvailabeAddress.insert( make_pair(i,i));
		}
		try
		{
			char time[256];
			GetDate(&time[0]);
			char filename[512];
			sprintf(filename,"%d %s.Log[0]",localPort,time);
			Log[0].CreateFile(filename);
			WaitMessage( localPortSocket, timeOut);
		}
		catch (exception* e)
		{
			Log[0].CreateFile("WaitMessage false\n");
			isok = false;
		}
   }
   if ( localPortSocket != INVALID_SOCKET)
   {
   	CloseSocket( localPortSocket);
   }
	return isok;
}

void Message::start()
{
	StartServer(localPort,timeout);
}
