#pragma comment(lib,"ws2_32")

#include<windows.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<process.h>

#define BUFFSIZE 1024

void sendMsg(void* sock);
void recvMsg(void* sock);

bool QUIT = false;

int main()
{
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD( 1, 1 );                   // 使用的 WinSock 的版本

    // 加载套接字库
    int err = WSAStartup( wVersionRequested, &wsaData );    // 初始化 WinSock DLL 库
    if ( err != 0 )
    {
        printf("WSASTARTUP_ERROR: %d\n", err);
        return 0;
    }
    if ( LOBYTE( wsaData.wVersion ) != 1 || HIBYTE( wsaData.wVersion ) != 1 )
    {
        WSACleanup( );
        return 0;
    }

    // 创建套接字
    SOCKET socketClient;
    if((socketClient = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)   // INVALID_SOCKET意思是无效套接字
    {
        printf("SOCKET_ERROR: %d\n", INVALID_SOCKET);
        return 0;
    }

    // 填充SOCKADDR_IN结构
    SOCKADDR_IN addrServer;                                         // 服务器端结构体
    addrServer.sin_family = AF_INET;                                // 设为 AF_INET，告诉WinSock使用的是 IP 地址族
    addrServer.sin_port = htons(6000);                              // 就是要用来通讯的端口号，short类型，htons()函数用来将端口变量从主机字节顺序转换为网络字节的顺序
    addrServer.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");       // 用来通讯的 IP 地址信息

    // 向服务器发出连接请求
    if(connect(socketClient,(SOCKADDR*)&addrServer,sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		printf("CONNECT_ERROR: %d\n", SOCKET_ERROR);
        return 0;
    }
	else
	{
		printf("已连接! 服务器端 IP: %s Port:%d ，输入 quit 即可断开连接\n", inet_ntoa(addrServer.sin_addr), ntohs(addrServer.sin_port));
	}

    // 多线程
    HANDLE hHandle[2];
    // _beginthread() 方法用于创建一个线程，三个参数分别为：线程函数的地址、新线程的堆栈大小（设为0表示与主线程使用一样的堆栈）、参数列表（没有参数时为NULL）
    hHandle[0]=(HANDLE)_beginthread(sendMsg,0,(void*)socketClient);
    if(QUIT)
    {
        closesocket(socketClient);
        WSACleanup();
        return 0;
    }
    hHandle[1]=(HANDLE)_beginthread(recvMsg,0,(void*)socketClient);

    // 等待发送线程执行完毕
    WaitForSingleObject(hHandle[0],INFINITE);

    // 关闭套接字
    closesocket(socketClient);

    // 关闭加载的套接字库
    WSACleanup();

    return 0;
}

// 发送函数
void sendMsg(void* sock){

    SOCKET socketClient = (SOCKET)sock;

    char sendBuf[BUFFSIZE];

    while(true)
    {
        // 发送数据
        scanf("%[^\n]", sendBuf);
        getchar();
        if(strcmp(sendBuf, "quit") == 0)   // 退出
        {
            send(socketClient, sendBuf, BUFFSIZE, 0);
            QUIT = true;
            break;
        }
        if(send(socketClient, sendBuf, BUFFSIZE, 0) == SOCKET_ERROR)
        {
            printf("消息发送失败!\n");
            break;
        }
        Sleep(200);     // 稍等一会，用于进程之间的先后
    }
}

// 接收函数
void recvMsg(void* sock){

    SOCKET socketClient = (SOCKET)sock;

    char recvBuf[BUFFSIZE];
    int status = 0;

    while(true)
    {
        if(QUIT)
            break;
        // 接收数据
        status = recv(socketClient, recvBuf, BUFFSIZE, 0);
        if(status > 0)
        {
            if(strcmp(recvBuf, "quit") == 0)   // 退出
            {
                QUIT = true;
                printf("对方断开连接，通信结束\n");
                break;
            }
            printf("Server: %s\n", recvBuf);
        }
        else if(status < 0)
            printf("RECV_ERROR: %d\n",  SOCKET_ERROR);
        else
        {
            printf("断开连接，通信结束\n");
            break;
        }
    }
}
