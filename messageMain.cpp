//============================================================================
// Name        : Servers.cpp
// Author      : zhaojuan
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "Message.h"

#include <iostream>
using namespace std;

int main()
{
	Message message(10030,4000);
	message.start();
	return 0;
}
