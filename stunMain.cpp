//============================================================================
// Name        : Servers.cpp
// Author      : zhaojuan
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "Stun.h"

#include <iostream>
using namespace std;

int main()
{
	Stun stun(10020);
	stun.start();
	return 0;
}
