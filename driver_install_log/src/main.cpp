// driver_install.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "setup.h"

#include <iostream>
using namespace std;
//���ؿ���̨����
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) // ������ڵ�ַ  
int _tmain(int argc, _TCHAR* argv[])
{
	//cout << "-----------welcome to the driver installation------------" << endl;
	UpdateDriver();
	
	//system("pause");
	return 0;
}

