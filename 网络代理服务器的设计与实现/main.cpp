#pragma comment(lib, "WS2_32")

#include <cstdio>
#include <string>
#include <fstream>
#include <WinSock2.h>
#include <iostream>
#include <io.h>
#include <vector>

using namespace std;

#define BUFFER_SIZE 1024

#define PORT 2021

#define HOST "127.0.0.1"

#define ServerPORT 8000

#define ServerHOST "127.0.0.1"

#define HEADER "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Server: WebProxy DesignedBy Galaxy_Z\r\n\
Content-Length: %ld\r\n\r\n\
"

int main(int argc, char **argv)
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

    // 创建套接字
    SOCKET socketProxy;
    if((socketProxy = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)   // INVALID_SOCKET意思是无效套接字
    {
        printf("SOCKET_ERROR: %d\n", INVALID_SOCKET);
        return 0;
    }

    // proxy端
    sockaddr_in addrProxy;
    addrProxy.sin_family = AF_INET;
    addrProxy.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addrProxy.sin_port = htons(PORT);

    // server端
    SOCKADDR_IN addrServer;                                        // 服务器端结构体
    addrServer.sin_family = AF_INET;                               // 设为 AF_INET，告诉WinSock使用的是 IP 地址族
    addrServer.sin_port = htons(ServerPORT);                       // 就是要用来通讯的端口号，short类型，htons()函数用来将端口变量从主机字节顺序转换为网络字节的顺序
    addrServer.sin_addr.S_un.S_addr = inet_addr(ServerHOST);       // 用来通讯的 IP 地址信息

    // 绑定套接字到一个 IP 地址和一个端口上
    if(bind(socketProxy,(SOCKADDR*)&addrProxy,sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        printf("BIND_ERROR: %d\n", SOCKET_ERROR);
        return 0;
    }

    // 将套接字设置为监听模式等待连接请求，此处使用10个backlog，该参数指定的是完成队列的长度，参考链接：https://zhuanlan.zhihu.com/p/78879634
    if(listen(socketProxy,10) == SOCKET_ERROR)
    {
        printf("LISTEN_ERROR: %d\n", SOCKET_ERROR);
        return 0;
    }

    SOCKADDR_IN addrClient;                                 // 客户端结构体
    int len = sizeof(SOCKADDR),status = 0;;

    while(true)
    {
        printf("    WebProxy运行中...\n\n");

        // 请求到来后，接受连接请求，返回一个新的对应于此次连接的套接字
        SOCKET socketConn;
        // 如果客户端没有启动，那么程序一直停留在该函数处
        if((socketConn = accept(socketProxy,(SOCKADDR*)&addrClient,&len)) == INVALID_SOCKET)   // &addrClient 是缓冲区地址，保存了客户端的IP和端口等信息
        {
            printf("ACCPET_ERROR: %d\n", INVALID_SOCKET);
            closesocket(socketConn);
            return 0;
        }

        // 用返回的套接字和客户端进行通信
        char buffer[BUFFER_SIZE], recvBuf[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        memset(recvBuf, 0, BUFFER_SIZE);

        // 接收数据，即客户端发送的HTTP请求报文
        status = recv(socketConn, buffer, BUFFER_SIZE, 0);
        if(status > 0)
            printf("接收到的客户端HTTP请求报文：\n%s", buffer);
        else if(status < 0)
        {
            printf("RECV_ERROR: %d\n",  SOCKET_ERROR);
            break;
        }

        // 修改HTTP请求报文中的HOST
        int startpos, endpos, i;
        char portTemp[20];
        string RequestHeader = buffer, newHost;
        startpos = RequestHeader.find("Host: ");                                        // 获取原HOST值字段的起始索引
        endpos = RequestHeader.find("Connection:");
        itoa(ServerPORT,portTemp,10);                                                   // int 转 string
        newHost.append(ServerHOST).append(":").append(portTemp).append("\n");           // 拼接得到新HOST值
        RequestHeader.replace(startpos+6,endpos-startpos-6,newHost);                   // 将目的地址替换为外界目标服务器的地址
        for(i=0;i<RequestHeader.length();i++)
            buffer[i] = RequestHeader[i];
        buffer[i] = '\0';

        // 创建套接字
        SOCKET socketProxySend;
        if((socketProxySend = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)   // INVALID_SOCKET意思是无效套接字
        {
            printf("SOCKET_ERROR: %d\n", INVALID_SOCKET);
            return 0;
        }

        // 向服务器发出连接请求
        if(connect(socketProxySend,(SOCKADDR*)&addrServer,sizeof(SOCKADDR)) == SOCKET_ERROR)
        {
            printf("CONNECT_ERROR: %d\n", SOCKET_ERROR);
            return 0;
        }
        else
        {
            printf("已连接到服务器端! 外界目标服务器 IP: %s Port:%d \n\n", inet_ntoa(addrServer.sin_addr), ntohs(addrServer.sin_port));
        }

        // 转发数据，将客户端发送的HTTP请求报文转发给外界目标服务器
        if(send(socketProxySend, buffer, strlen(buffer), 0) == SOCKET_ERROR)
        {
            printf("客户端HTTP请求报文转发失败!\n");
            break;
        }
        else
        {
            printf("转发的客户端HTTP请求报文：\n%s", buffer);
        }

        // 接收数据，即外界目标服务器返回的数据
        status = recv(socketProxySend, recvBuf, BUFFER_SIZE, 0);
        if(status > 0)
            printf("接收到的外界目标服务器HTTP响应报文：\n%s\n\n", recvBuf);
        else if(status < 0)
            printf("RECV_ERROR: %d\n",  SOCKET_ERROR);

        // 转发数据，将外界目标服务器返回的数据转发给客户端
        if(send(socketConn, recvBuf, strlen(recvBuf), 0) == SOCKET_ERROR)
        {
            printf("外界目标服务器HTTP响应报文转发失败!\n");
            break;
        }
        else
        {
            printf("转发的外界目标服务器HTTP响应报文：\n%s\n\n", recvBuf);
        }
        memset(recvBuf, 0, BUFFER_SIZE);

        // 继续接收并转发数据，即外界目标服务器返回的网页
        while(recv(socketProxySend, recvBuf, BUFFER_SIZE - 1, 0))
        {
            if(send(socketConn, recvBuf, strlen(recvBuf), 0) == SOCKET_ERROR)
            {
                printf("网页内容转发失败!\n");
                break;
            }
            //printf("转发的文件数据：\n%s\n\n", recvBuf);
            memset(recvBuf, 0, BUFFER_SIZE);
        }

        // 关闭套接字
        closesocket(socketProxySend);
        closesocket(socketConn);
    }

    closesocket(socketProxy);
    WSACleanup();

    return 0;
}
