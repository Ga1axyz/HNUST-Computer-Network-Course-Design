#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <string>
#include <WinSock2.h>
#include <stdio.h>
#include "unistd.h"

#define BUFFSIZE 2048

using namespace std;

// 定义 Base64 数据格式
struct Base64Date6
{
    unsigned int d4:6;
    unsigned int d3:6;
    unsigned int d2:6;
    unsigned int d1:6;
};

// 将二进制代码转换成 ASCII 码
char ConvertToBase64(char uc)
{
    if(uc <26)
    {
        return'A'+uc;
    }
    if(uc <52)
    {
        return'a'+(uc -26);
    }
    if(uc <62)
    {
        return'0'+(uc -52);
    }
    if(uc ==62)
    {
        return'+';
    }
    if(uc ==63)
    {
        return '/';
    }
}

// Base64 的实现
void  EncodeBase64(char*dbuf,char*buf128,int len)
{
    struct  Base64Date6*ddd =NULL;
    int i =0;
    char buf[256]= {0};                     // 数组uf里面的值全部初始化为0
    char *tmp =NULL;
    char cc ='\0';                          // 对应的ASCII值为0，是字符串结束的标志
    memset(buf,0,256);                      // 初始化函数。作用是将某一块内存中的内容全部设置为指定的值。memset()函数通常为新申请的内存做初始化工作。
    strcpy(buf,buf128);                     // 把从buf128地址开始且含有NULL结束符的字符串复制到以dest开始的地址空间
    for(i =1; i <=len/3; i++)
    {
        tmp =buf +(i-1)*3;
        cc =tmp[2];
        tmp[2]=tmp[0];
        tmp[0]=cc;
        ddd =(struct Base64Date6*)tmp;
        dbuf[(i-1)*4+0]=ConvertToBase64((unsigned int)ddd->d1);
        dbuf[(i-1)*4+1]=ConvertToBase64((unsigned int)ddd->d2);
        dbuf[(i-1)*4+2]=ConvertToBase64((unsigned int)ddd->d3);
        dbuf[(i-1)*4+3]=ConvertToBase64((unsigned int)ddd->d4);
    }
    if(len %3==1)
    {
        tmp =buf +(i-1)*3;
        cc =tmp[2];
        tmp[2]=tmp[0];
        tmp[0]=cc;
        ddd =(struct Base64Date6*)tmp;
        dbuf[(i-1)*4+0]=ConvertToBase64((unsigned int)ddd->d1);
        dbuf[(i-1)*4+1]=ConvertToBase64((unsigned int)ddd->d2);
        dbuf[(i-1)*4+2]='=';
        dbuf[(i-1)*4+3]='=';
    }
    if(len%3==2)
    {
        tmp =buf+(i-1)*3;
        cc =tmp[2];
        tmp[2]=tmp[0];
        tmp[0]=cc;
        ddd =(struct Base64Date6*)tmp;
        dbuf[(i-1)*4+0]=ConvertToBase64((unsigned int)ddd->d1);
        dbuf[(i-1)*4+1]=ConvertToBase64((unsigned int)ddd->d2);
        dbuf[(i-1)*4+2]=ConvertToBase64((unsigned int)ddd->d3);
        dbuf[(i-1)*4+3]='=';
    }
    return;
}

int main()
{
    string message, info, mail, subject, imail, icode;

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

    HOSTENT* pHostent;                                      // hostent是host entry的缩写，该结构记录主机的信息，包括主机名、别名、地址类型、地址长度和地址列表

    cout<<"请输入发件/查询者邮箱：";
    cin>>imail;
    cout<<"请输入邮箱授权码：";          // yffzdykoufhfeabg
    cin>>icode;

    while (true)
    {
        char buff[BUFFSIZE], temp[BUFFSIZE];                                      // 定义缓冲区

        cout << "\n请选择服务：1.发送邮件 2.查看邮箱  0.退出：";
        int call;
        cin >> call;
        if (call == 1)
        {
            // 创建套接字
            SOCKET sockClient;
            if((sockClient = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)   // INVALID_SOCKET意思是无效套接字
            {
                printf("SOCKET_ERROR: %d\n", INVALID_SOCKET);
                return 0;
            }

            HOSTENT* pHostent;                                                   // hostent是host entry的缩写，该结构记录主机的信息，包括主机名、别名、地址类型、地址长度和地址列表
            pHostent = gethostbyname("smtp.qq.com");                             // 获得qq邮箱服务器的地址信息

            // 填充SOCKADDR_IN结构
            sockaddr_in addrServer;
            addrServer.sin_family = AF_INET;
            addrServer.sin_addr.S_un.S_addr = *((DWORD*)pHostent->h_addr_list[0]);          // 得到 SMTP 服务器的网络字节序的ip地址
            addrServer.sin_port = htons(25);

            // 向服务器发出连接请求
            if(connect(sockClient,(SOCKADDR*)&addrServer,sizeof(SOCKADDR)) == SOCKET_ERROR)
            {
                printf("CONNECT_ERROR: %d\n", SOCKET_ERROR);
                return 0;
            }
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';                               // 接收返回值

            // 登录邮件服务器
            message = "ehlo qq.com\r\n";
            send(sockClient, message.c_str(), message.length(), 0);                         // EHLO
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';

            message = "auth login\r\n";
            send(sockClient, message.c_str(), message.length(), 0);                         // AUTH LOGIN
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';

            memset(buff, 0, BUFFSIZE);
            memset(temp, 0, BUFFSIZE);

            for(int i = 0; i < sizeof(imail); i++)
                buff[i] = imail[i];
            EncodeBase64(temp,buff,strlen(buff));                                           // 发件人邮箱
            sprintf(buff, "%s\r\n", temp);
            send(sockClient, buff, strlen(buff), 0);
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';

            memset(buff, 0, BUFFSIZE);
            memset(temp, 0, BUFFSIZE);

            for(int i = 0; i < sizeof(icode); i++)
                buff[i] = icode[i];
            EncodeBase64(temp,buff,strlen(buff));                                           // 邮箱授权码
            sprintf(buff, "%s\r\n", temp);
            send(sockClient, buff, strlen(buff), 0);
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';

            memset(buff, 0, BUFFSIZE);
            memset(temp, 0, BUFFSIZE);

            cout<<"\n请输入收件人邮箱：";
            cin >> mail;
            message = "MAIL FROM:<";
            message.append(imail);
            message.append(">\r\nRCPT TO:<");
            message.append(mail);
            message.append("> \r\n");
            send(sockClient, message.c_str(), message.length(), 0);
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';

            // 邮件内容
            message = "DATA\r\n";
            send(sockClient, message.c_str(), message.length(), 0);                         // DATA
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';

            message = "From: ";
            message.append(imail);
            message.append("\r\n\To:");
            message.append(mail);
            message.append("\r\n\subject:");

            getchar();
            cout << "请输入邮件主题：";
            getline(cin,subject);
            message.append(subject);
            message.append("\r\n\r\n");
            cout << "请输入邮件内容：";
            getline(cin,info);
            message.append(info);
            message.append("\r\n.\r\n");
            send(sockClient, message.c_str(), message.length(), 0);

            message = "QUIT\r\n";
            send(sockClient, message.c_str(), message.length(), 0);
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';

            cout << "邮件发送成功！" << endl;
        }
        if (call == 2)
        {
            // 创建套接字
            SOCKET sockClient;
            if((sockClient = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)   // INVALID_SOCKET意思是无效套接字
            {
                printf("SOCKET_ERROR: %d\n", INVALID_SOCKET);
                return 0;
            }

            HOSTENT* pHostent;
            pHostent = gethostbyname("pop.qq.com");

            // 填充SOCKADDR_IN结构
            sockaddr_in addrServer;
            addrServer.sin_family = AF_INET;
            addrServer.sin_addr.S_un.S_addr = *((DWORD*)pHostent->h_addr_list[0]);          // 得到 POP3 服务器的网络字节序的ip地址
            addrServer.sin_port = htons(110);

            // 向服务器发出连接请求
            if(connect(sockClient,(SOCKADDR*)&addrServer,sizeof(SOCKADDR)) == SOCKET_ERROR)
            {
                printf("CONNECT_ERROR: %d\n", SOCKET_ERROR);
                return 0;
            }
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';                               // 接收返回值

            // 查询者账户
            message = "user ";
            message.append(imail);
            message.append("\r\n");
            send(sockClient, message.c_str(), message.length(), 0);
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';

            memset(buff, 0, BUFFSIZE);
            memset(temp, 0, BUFFSIZE);

            // 查询者邮箱授权码
            message = "pass ";
            message.append(icode);
            message.append("\r\n");
            send(sockClient, message.c_str(), message.length(), 0);
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';

            memset(buff, 0, BUFFSIZE);
            memset(temp, 0, BUFFSIZE);

            // 返回邮件数量和每个邮件的大小
            message = "list\r\n";
            send(sockClient, message.c_str(), message.length(), 0);
            buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';

            cout << "\n邮件列表 - 邮件大小：" << buff << endl;
            while (1)
            {
                int num;
                cout << "请输入序号读取相应邮件，输入 0 退出：";
                cin >> num;

                if (num == 0)
                    break;

                message = "retr ";
                message.append(to_string(num));
                message.append("\r\n");
                send(sockClient, message.c_str(), message.length(), 0);

                buff[recv(sockClient, buff, BUFFSIZE, 0)] = '\0';
                cout << "该邮件内容为：" << buff << endl;

                memset(buff, 0, BUFFSIZE);
            }
        }
        if (call == 0)
        {
            return 0;
        }
    }

    return 0;
}
