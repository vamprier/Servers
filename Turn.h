/*
 * Turn.h
 *
 *  Created on: Mar 14, 2016
 *      Author: root
 */

#ifndef TURN_H_
#define TURN_H_

#include "Server.h"

class Turn  : public Server{
public:
	Turn(u_16 port);
	~Turn();
public:
	virtual void start();
private:
	static void* MainProc(void* lpParameter);
	bool ThreadProc( u_16 startPort, u_16 portNumber);
};

#endif /* TURN_H_ */
