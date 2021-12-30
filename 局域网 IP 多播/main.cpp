#pragma comment(lib, "ws2_32.lib")

#include<windows.h>
#include<winsock2.h>
#include<ws2tcpip.h>
#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<process.h>

using namespace std;

#define MCASTADDR "233.0.0.1"                   // 本例使用的多播组地址
#define MCASTPORT 5150                          // 本地端口号
#define BUFFERSIZE 1024                         // 缓冲区大小


void sendMsg(void* sock);
void recvMsg(void* sock);

int main(int argc,char **argv)
{

    WORD wVersionRequested;                                 // 双字节
    WSADATA wsaData;                                        // WinSock 库版本的相关信息
    wVersionRequested = MAKEWORD( 2, 2 );                   // 使用的 WinSock 的版本

    // 加载套接字库
    int err = WSAStartup( wVersionRequested, &wsaData );    // 加载 WinSock 库并确定 WinSock 版本，系统会把数据填入 wsaData 中
    if ( err != 0 )
    {
        printf("WSASTARTUP_ERROR: %d\n", err);
        return 0;
    }

    // 创建 SOCK_DGRAM 类型的套接字
    SOCKET socketClient;
    socketClient = WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0,WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF|WSA_FLAG_OVERLAPPED);

    // 本地端
    sockaddr_in local;
    local.sin_family=AF_INET;                               // IPv4
    local.sin_port=htons(MCASTPORT);                        // 5150 绑定的本地端口号
    local.sin_addr.s_addr=INADDR_ANY;                       // 本机的所有IP

    // 绑定套接字到一个 IP 地址和一个端口上
    bind(socketClient,(struct sockaddr*)&local,sizeof(local));

    // 多播组
    sockaddr_in remote;
    remote.sin_family=AF_INET;                              // IPv4
    remote.sin_port=htons(MCASTPORT);                       // 5150 绑定的本地端口号
    remote.sin_addr.s_addr=inet_addr(MCASTADDR);            // 本例使用的多播组IP地址

    // 将本机加入多播组,WSAJoinLeaf()各参数分别为：套接口、远端名称、长度、指针、标志位JL_BOTH（表示既是接收者也是发送者）
    if((WSAJoinLeaf(socketClient,(SOCKADDR*)&remote,sizeof(remote),NULL,NULL,NULL,NULL,JL_BOTH)) == INVALID_SOCKET)
    {
        printf("WSAJoinLeaf() failed:%d\n",WSAGetLastError());
        closesocket(socketClient);
        WSACleanup();
        return 0;
    }

    cout<<"正在监听多播组："<<MCASTADDR<<"\n"<<endl;

    // 多线程
    HANDLE hHandle[2];
    // _beginthread() 方法用于创建一个线程，三个参数分别为：线程函数的地址、新线程的堆栈大小（设为0表示与主线程使用一样的堆栈）、参数列表（没有参数时为NULL）
    hHandle[0]=(HANDLE)_beginthread(sendMsg,0,(void*)socketClient);
    hHandle[1]=(HANDLE)_beginthread(recvMsg,0,(void*)socketClient);

    // 等待发送线程执行完毕
    WaitForSingleObject(hHandle[0],INFINITE);

    // 关闭加载的套接字库
    WSACleanup();

    return 0;
}

// 发送函数
void sendMsg(void* sock){

    SOCKET server = (SOCKET)sock;
    char sendBuffer[BUFFERSIZE];

    // 多播组
    sockaddr_in remote;
    memset(&remote, 0, sizeof(remote));
    remote.sin_family=AF_INET;                              // IPv4
    remote.sin_port=htons(MCASTPORT);                       // 5150 绑定的本地端口号
    remote.sin_addr.s_addr=inet_addr(MCASTADDR);            // 本例使用的多播组IP地址

    while(true)
    {
        scanf("%[^\n]", sendBuffer);
        getchar();
        if(strncmp(sendBuffer,"quit",4)==0)                   // 输入quit退出
        {
            printf("\n程序结束\n\n");
            return;
        }
        if(sendto(server,sendBuffer,strlen(sendBuffer),0,(sockaddr*)(&remote),sizeof(remote)) == SOCKET_ERROR)
        {
            printf("消息发送失败!\n");
            break;
        }
        Sleep(200);     // 稍等一会，用于进程之间的先后
    }

    // 关闭套接字
    closesocket(server);
}

// 接收函数
void recvMsg(void* sock){

    SOCKET server = (SOCKET)sock;

    // 接收数据
    sockaddr_in from;                       // 对端 socket 地址
    int iFromLen = sizeof(from);           // 地址结构大小

    char recvBuffer[BUFFERSIZE+1];
    int iReadDataLen;                           // 接收数据长度

    while(true)
    {
        iReadDataLen = recvfrom(server,recvBuffer,BUFFERSIZE,0,(sockaddr*)&from,&iFromLen);
        if(iReadDataLen > 0)                    // 有数据到达
        {
            recvBuffer[iReadDataLen] = '\0';    // 解决换行显示异常的问题
            printf("IP为 %s 的用户说: %s\n",inet_ntoa(from.sin_addr),recvBuffer);
        }
        else if(iReadDataLen < 0)
        {
            cout<<"recvMSG failed:"<<WSAGetLastError()<<endl;
            break;
        }
        memset(recvBuffer, 0, BUFFERSIZE+1);
    }

    // 关闭套接字
    closesocket(server);
}
