#include   <windows.h>  
#include   <mmsystem.h>
#include <stdlib.h>
#include <process.h>  
#include   <stdio.h>    
#include   <conio.h>    
#include	 <iostream>
#include	 <assert.h>
#include "NetDefine.h"
#include <sstream>
#include <fstream>
#include <cassert>
#include <tchar.h>
//#include <WinSock2.h>
#pragma comment( lib, "ws2_32.lib" )

#pragma comment(lib,"WinMM.Lib")

UINT WINAPI   Thread(void*   pvoid);
UINT WINAPI	RunSendTimeMsgThread(void* pvoid);
HANDLE  g_HEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
DWORD startTime = 0;
int ic_waitForTime = 400;
int ic_imgCount = 4;
int i_timeTip = 5;

const std::string g_NumberSound[10] = {"018k8b.wav","028k8b.wav", "038k8b.wav", "048k8b.wav", "058k8b.wav", "068k8b.wav" };
void ReadConfigTxt(const char* str_filePath,int i_time,int i_picCount,int i_timeTip);
inline BOOL MByteToWChar(LPCSTR lpcszStr, LPWSTR lpwszStr, DWORD dwSize)
{
	// Get the required size of the buffer that receives the Unicode 
	// string. 
	DWORD dwMinSize;
	dwMinSize = MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, NULL, 0);
	assert(dwSize >= dwMinSize);

	// Convert headers from ASCII to Unicode.
	MultiByteToWideChar(CP_ACP, 0, lpcszStr, -1, lpwszStr, dwMinSize);
	return TRUE;
}

inline void PlayNumberSound(const std::string& strSoundPath)
{

	wchar_t wText[30] = { 0 };
	MByteToWChar(strSoundPath.c_str(), wText, sizeof(wText) / sizeof(wText[0]));
	PlaySound(wText, NULL, SND_FILENAME | SND_SYNC);
}
/************************************************************************/
/*链接算法的TCP
* （需要确定链接成功后才开始几时程序）
*/
/************************************************************************/
UINT WINAPI RunSendTimeMsgThread(void*)
{
	SetEvent(g_HEvent);
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return -1;
	}
	// 创建套接字
	mServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (mServerSocket == INVALID_SOCKET)
	{
		std::cout << "创建套接字失败!" << std::endl;
		return -1;
	}

	//lookup host to get address information
	if ((gethostbyname(SERVER_IP)) == NULL) {
		fprintf(stderr, "Host lookup failed for '%s'.\n", SERVER_IP);
		return -1;
	}
	//make sure serv_addr is cleared and then set values for each member
	memset(&mServerAddr, 0, sizeof(mServerAddr));
	// 填充服务器的IP和端口号
	mServerAddr.sin_family = AF_INET;
	mServerAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	mServerAddr.sin_port = htons((u_short)SERVER_PORT);

	// 连接到服务器
	while (!(connect(mServerSocket, (struct sockaddr*)&mServerAddr, sizeof(mServerAddr))) <= 0)
	{
		Sleep(100);
	}
	std::string strStart = "startClock";
	send(mServerSocket, strStart.c_str(), strStart.length(), 0);
	startTime = GetTickCount();
	/*while (1)
	{
		MSG   smsg;
		PeekMessage(&smsg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
		if (WM_QUIT == smsg.message)
		{
			
			break;
		}
	}*/

	return 1;
}
/************************************************************************/
/*开始定时的线程
*/
/************************************************************************/
UINT WINAPI   Thread(void*   pvoid)
{
	SetEvent(g_HEvent);
	MSG   msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	UINT   timerid = SetTimer(NULL, 111, 1000*i_timeTip, NULL);//没5秒发出WM_TIMER消息，但是不定义相应函数
	BOOL   bRet;
	int   count = 0;
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			//   handle   the   error   and   possibly   exit    
		}
		else if (msg.message == WM_TIMER)
		{
			count++;
			std::ostringstream stream;
			if (count >ic_waitForTime /i_timeTip)
				break;
			else if (count % 2 == 1)
			{
				PlayNumberSound(g_NumberSound[(count / 2) % ic_imgCount]);
				stream << ((count / 2) % ic_imgCount + 1) << ',' ;
				std::cout << (count / 2)  + 1 << std::endl;
			}
			else if (count % 2 == 0)
			{
				MessageBeep(MB_ICONHAND);
				stream << 0 << ',';
			}
			if (startTime != 0)
			{
				DWORD dw_time = GetTickCount();
				stream << dw_time - startTime;
				//AllocConsole();//生产一个控制台用于显示当前是哪个图片开始显示和熄灭 ，屏蔽这里可以去掉console显示
				//freopen("CONOUT$", "w", stdout);
				//std::cout << "ssssb333333333333bb" << std::endl;
				//std::cout << dw_time  -startTime << std::endl;
				std::string args = stream.str();
				send(mServerSocket, args.c_str(), args.length(), 0);
			}
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	closesocket(mServerSocket);
	WSACleanup();
	KillTimer(NULL, timerid);
	printf("thread   end   here\n");
	return   0;
}
void TCHAR2Char(const TCHAR* tchar, char* _char)
{
	int iLength;
	//获取字节长度   
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	//将tchar值赋给_char    
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}

std::string GetInstanceFolderPath(void)
{
	std::string exePath = "";
	TCHAR tcFullPath[MAX_PATH];
	char* pChPath = new char[MAX_PATH];
	memset(pChPath, '\0', MAX_PATH);
	/** 获取当前程序的执行路径exe路径 */
	GetModuleFileName(NULL, tcFullPath, MAX_PATH);
	/** 将tchar转为char */
	TCHAR2Char(tcFullPath, pChPath);
	exePath = std::string(pChPath);

	std::string dirPath = "";
	size_t iPos = exePath.rfind("\\");
	dirPath = exePath.substr(0, iPos);
	/** 释放资源 */
	delete[] pChPath;
	return dirPath;
}




void ReadConfigTxt(const char* str_filePath, int i_time, int i_picCount,int i_timeTip)
{
	std::string str_configPath = GetInstanceFolderPath();
	//(std::_tcsrchr(szFilePath, _T('\\')))[1] = 0; // 删除文件名，只获得路径字串
	//std::CString str_url = szFilePath;

	char buffer[512];
	str_configPath = str_configPath + "\\" + str_filePath;
	std::ifstream myfile(str_configPath);


	if (!myfile)
	{
		std::cout << "Unable to open myfile";
		exit(1); // terminate with error  
	}
	int i = 0, j = 0;
	int data[6][2];
	while (!myfile.eof())
	{
		myfile.getline(buffer, 128);
		sscanf(buffer, "%d %d %d", &i_time, &i_picCount,&i_timeTip);
	}
	myfile.close();
}

void   main()
{
	unsigned   dwThreadId_1;
	unsigned   dwThreadId_2;
	printf("简单计时器\n");
	RunSendTimeMsgThread(&dwThreadId_1);
	//HANDLE   hThread1 = (HANDLE)_beginthreadex(NULL, 0, RunSendTimeMsgThread, nullptr, 0, &dwThreadId_1);
	//DWORD   dwwait1 = WaitForSingleObject(hThread1, INFINITE);
	
	const char* strc_filePath = "config.txt";
	ReadConfigTxt(strc_filePath,ic_waitForTime, ic_imgCount, i_timeTip);

	HANDLE   hThread = (HANDLE)_beginthreadex(NULL, 0, Thread, nullptr, 0, &dwThreadId_2);
	//HANDLE   hThread = CreateThread(
	//	NULL,                                                 //   no   security   attributes      
	//	0,                                                       //   use   default   stack   size        
	//	Thread,                                     //   thread   function      
	//	0,                                 //   argument   to   thread   function      
	//	0,                                                       //   use   default   creation   flags      
	//	&dwThreadId);
	DWORD   dwwait = WaitForSingleObject(hThread, 1000 * ic_waitForTime);
	switch (dwwait)
	{
	case   WAIT_ABANDONED:
		printf("main   thread   WaitForSingleObject   return   WAIT_ABANDONED\n");
		break;
	case   WAIT_OBJECT_0:
		printf("main   thread   WaitForSingleObject   return   WAIT_OBJECT_0\n");
		break;
	case   WAIT_TIMEOUT:
		printf("main   thread   WaitForSingleObject   return   WAIT_TIMEOUT\n");
		break;
	}
	CloseHandle(hThread);
	//CloseHandle(hThread1);
	_getch();
}

