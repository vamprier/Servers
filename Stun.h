/*
 * Stun.h
 *
 *  Created on: Mar 14, 2016
 *      Author: root
 */

#ifndef STUN_H_
#define STUN_H_

#include "Server.h"

class Stun : public Server{
public:
	Stun(u_16 port);
	~Stun();
public:
	virtual void start();
private:
	static void* MainProc(void* lpParameter);
	static void* SendProc(void* lpParameter);
	bool ThreadProc( u_16 startPort, u_16 portNumber);
};

#endif /* STUN_H_ */
