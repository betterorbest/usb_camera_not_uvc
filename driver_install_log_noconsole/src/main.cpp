// driver_install.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "setup.h"

//#include <iostream>
//using namespace std;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	//cout << "-----------welcome to the driver installation------------" << endl;
	UpdateDriver();
	
	//system("pause");
	return 0;
}

