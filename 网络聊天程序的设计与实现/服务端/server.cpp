#pragma comment(lib,"ws2_32")

#include <stdio.h>
#include <winsock2.h>

#define BUFFSIZE 1024

int main()
{
    WORD wVersionRequested;                                 // 双字节
    WSADATA wsaData;                                        // WinSock 库版本的相关信息
    wVersionRequested = MAKEWORD( 1, 1 );                   // 使用的 WinSock 的版本

    // 加载套接字库
    int err = WSAStartup( wVersionRequested, &wsaData );    // 加载 WinSock 库并确定 WinSock 版本，系统会把数据填入 wsaData 中
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

    // 创建套接字,socket()的三个参数分别为 AF_INET（表示使用的是 TCP/IP 地址族）、Socket类型（流式、数据报）、与特定的地址族相关的协议（若指定为0，系统会自动选择一个合适的协议）
    SOCKET socketServer;
    if((socketServer = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)   // INVALID_SOCKET意思是无效套接字
    {
        printf("SOCKET_ERROR: %d\n", INVALID_SOCKET);
        return 0;
    }

    // 填充SOCKADDR_IN结构
    SOCKADDR_IN addrServer;                                   // 服务器端结构体
    addrServer.sin_family = AF_INET;                          // 设为 AF_INET，告诉WinSock使用的是 IP 地址族
    addrServer.sin_port = htons(6000);                        // 就是要用来通讯的端口号，short类型，htons()函数用来将端口变量从主机字节顺序转换为网络字节的顺序
    addrServer.sin_addr.S_un.S_addr = htonl(INADDR_ANY);      // 用来通讯的 IP 地址信息，long类型， htonl()函数作用同上,INADDR_ANY 就是指定地址为0.0.0.0的地址，这个地址事实上表示不确定地址,也就是表示本机的所有IP

    // 绑定套接字到一个 IP 地址和一个端口上
    if(bind(socketServer,(SOCKADDR*)&addrServer,sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        printf("BIND_ERROR: %d\n", SOCKET_ERROR);
        return 0;
    }

    // 将套接字设置为监听模式等待连接请求，此处使用5个backlog，该参数指定的是完成队列的长度，参考链接：https://zhuanlan.zhihu.com/p/78879634
    if(listen(socketServer,5) == SOCKET_ERROR)
    {
        printf("LISTEN_ERROR: %d\n", SOCKET_ERROR);
        return 0;
    }

    SOCKADDR_IN addrClient;                                 // 客户端结构体
    int len = sizeof(SOCKADDR);

    printf("监听中...\n");
    // 请求到来后，接受连接请求，返回一个新的对应于此次连接的套接字
    SOCKET socketConn;
    // 如果客户端没有启动，那么程序一直停留在该函数处
    if((socketConn = accept(socketServer,(SOCKADDR*)&addrClient,&len)) == INVALID_SOCKET)   // &addrClient 是缓冲区地址，保存了客户端的IP和端口等信息
    {
        printf("ACCPET_ERROR: %d\n", INVALID_SOCKET);
        closesocket(socketConn);
        return 0;
    }
    printf("已连接! 客户端 IP: %s Port:%d ，输入 quit 即可断开连接\n", inet_ntoa(addrClient.sin_addr), ntohs(addrClient.sin_port));

    // 用返回的套接字和客户端进行通信
    char recvBuf[BUFFSIZE], sendBuf[BUFFSIZE];
    int status = 0;
    while(true)
    {

        // 接收数据
        status = recv(socketConn, recvBuf, BUFFSIZE, 0);
        if(status > 0)
            printf("Client: %s\n", recvBuf);
        else if(status < 0)
            printf("RECV_ERROR: %d\n",  SOCKET_ERROR);
        else
        {
            printf("对方断开连接，通信结束\n");
            break;
        }

        // 发送数据
        printf("Server: ");
        scanf("%[^\n]", sendBuf);
        getchar();
        if(strcmp(sendBuf, "quit") == 0)   // 退出
            break;
        if(send(socketConn, sendBuf, BUFFSIZE, 0) == SOCKET_ERROR)
        {
            printf("消息发送失败!\n");
            break;
        }
    }

    // 关闭套接字
    closesocket(socketConn);
    closesocket(socketServer);

    // 关闭加载的套接字库
    WSACleanup();

    return 0;
}
