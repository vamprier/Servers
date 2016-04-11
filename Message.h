/*
 * Message.h
 *
 *  Created on: Mar 14, 2016
 *      Author: root
 */

#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "Server.h"

#define MAG_NUMBER 100000

typedef struct
 {
	RequestMessagePackage msg;
 	time_t msgTime;
 	unsigned int ip;
 	unsigned short port;
 }MessageContent;

class Message : public Server {
public:
	Message(u_16 port,double t);
	~Message();
public:
	virtual void start();
private:
	void ChangeMessage( RequestMessagePackage* mspNeedChangeLeft, RequestMessagePackage* mspNeedChangeRight);
	void WaitMessage( Socket localPortSocket, double timeOut);
	bool StartServer( unsigned short localPort, double timeOut);
private:
	 map<unsigned int,int> unAvailabeAddress;//不可用地址
	 map<unsigned int,int> AvailabeAddress;//可用地址
	 MessageContent messageVector[MAG_NUMBER];//消息数组
	 vector<pair<unsigned int,unsigned short> > ipPortVec;//存储ip和port对
	 double timeout;

};

#endif /* MESSAGE_H_ */
