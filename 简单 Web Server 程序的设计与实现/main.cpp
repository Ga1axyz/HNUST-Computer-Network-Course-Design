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

#define HOST "127.0.0.1"

#define HEADER "\
HTTP/1.1 200 OK\r\n\
Content-Type: text/html; charset=UTF-8\r\n\
Server: WebServer OptimizedBy Galaxy_Z\r\n\
Content-Length: %ld\r\n\r\n\
"

// 获取文件的大小
long GetFileLength(string strPath)
{
    // fstream类有和open()方法相同的构造函数，在实例化的时侯就可以打开文件，fin是变量名，是fstream类的一个实例对象
    ifstream fin(strPath, ios::in | ios::binary);               // ios::in 文件以输入方式打开(文件数据输入到内存)；ios::binary 以二进制方式打开文件，缺省的方式是文本方式；| 表示或
    fin.seekg(0, ios_base::end);                                // 将读指针指向文件末尾；seekg()用来设置读位置；seekp()用来设置写位置，第一个参数是偏移量，第二个参数是基准位置
    streampos pos = fin.tellg();                                 // 返回当前 get 流指针的位置，此处即返回文件尾位置
    long lSize = static_cast<long>(pos);
    fin.close();
    return lSize;
}

// 获取指定目录下的所有文件名，将其保存在数组中
void getFiles(string path, vector<string>& files)
{
    long   hFile = 0;
    struct _finddata_t fileinfo;    // 文件信息，声明一个存储文件信息的结构体
    string p;                       // 字符串，存放路径                                 // fileinfo是用来存放文件信息的结构体的指针，_findfirst函数成功执行后，函数会把找到的文件的信息放入这个结构体中
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)    // string &assign(const string &s)方法：把字符串s赋给当前字符串
    {                                                                                    // * 是通配符，第一个 \ 用于转义，比如：D:\\test\\*，则表示D盘的test文件夹内的所有文件，实际上的路径即D:\test\*
        do
        {
            if ((fileinfo.attrib &  _A_SUBDIR))     // 如果是目录,迭代之（即文件夹内还有文件夹）
            {
                //文件名不等于"."&&文件名不等于".."  "."表示当前目录  ".." 表示当前目录的父目录，判断时，两者都要忽略，防止死循环
                if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
                    getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
            }
            else
            {
                files.push_back(p.assign(path).append("\\").append(fileinfo.name));
            }
        }
        while (_findnext(hFile, &fileinfo) == 0);   // 获取fileinfo中下一个文件的信息
        _findclose(hFile);      // _findclose函数结束查找
    }
}

int main(int argc, char **argv)
{
    int PORT = 8000;
    string typePort;
    cout<<"请指定 WebServer 的端口号：";
    cin>>typePort;
    PORT=atoi(typePort.c_str());
    cout<<"WebServer访问地址："<<HOST<<":"<<PORT<<"\n"<<endl;

    string webServerPath = "",filename = "",loopChoose = "0";
    cout<<"请指定 WebServer 的根目录（输入 . 进入当前目录）：";
    cin>>webServerPath;

    const char * filePath = webServerPath.c_str();       // c_str()用来将string类对象转换成和C语言兼容的char *类型。这是为了与c语言兼容，在c语言中没有string类型，故必须通过string类对象的成员函数c_str()把string 对象转换成c中的字符串样式
    vector<string> files;

    cout<<"\n该目录下的文件有："<<endl;
    getFiles(filePath, files);                            // 显示该路径下的所有文件
    for (int i = 0; i < files.size(); i++)
    {
        cout << files[i].c_str() << endl;
    }
    cout<<"\n";

    /*
    LOOP: cout<<"\n请选择需要读取的文件：";
    cin>>filename;

    cout<<"\n是否只需要读取此文件（输入1表示是，0表示否）：";
    cin>>loopChoose;

    string filePathAndName = filePath;
    filePathAndName.append("\\").append(filename);

    // 获取文件的大小
    cout<<"\n该文件的大小为："<< GetFileLength(filePathAndName) <<"字节\n"<<endl;
    */

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
    SOCKET socketServer;
    if((socketServer = socket(AF_INET,SOCK_STREAM,0)) == INVALID_SOCKET)   // INVALID_SOCKET意思是无效套接字
    {
        printf("SOCKET_ERROR: %d\n", INVALID_SOCKET);
        return 0;
    }

    // 填充SOCKADDR_IN结构
    sockaddr_in addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.S_un.S_addr = INADDR_ANY;
    addrServer.sin_port = htons(PORT);

    // 绑定套接字到一个 IP 地址和一个端口上
    if(bind(socketServer,(SOCKADDR*)&addrServer,sizeof(SOCKADDR)) == SOCKET_ERROR)
    {
        printf("BIND_ERROR: %d\n", SOCKET_ERROR);
        return 0;
    }

    // 将套接字设置为监听模式等待连接请求，此处使用10个backlog，该参数指定的是完成队列的长度，参考链接：https://zhuanlan.zhihu.com/p/78879634
    if(listen(socketServer,10) == SOCKET_ERROR)
    {
        printf("LISTEN_ERROR: %d\n", SOCKET_ERROR);
        return 0;
    }

    SOCKADDR_IN addrClient;                                 // 客户端结构体
    int len = sizeof(SOCKADDR),status = 0;;

    while(true)
    {
        printf("    WebServer运行中...\n\n");

        // 请求到来后，接受连接请求，返回一个新的对应于此次连接的套接字
        SOCKET socketConn;
        // 如果客户端没有启动，那么程序一直停留在该函数处
        if((socketConn = accept(socketServer,(SOCKADDR*)&addrClient,&len)) == INVALID_SOCKET)   // &addrClient 是缓冲区地址，保存了客户端的IP和端口等信息
        {
            printf("ACCPET_ERROR: %d\n", INVALID_SOCKET);
            closesocket(socketConn);
            return 0;
        }

        // 用返回的套接字和客户端进行通信
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);

        // 接收数据，即客户端发送的请求报文
        status = recv(socketConn, buffer, BUFFER_SIZE, 0);
        if(status > 0)
            printf("接收到的HTTP请求报文：\n%s", buffer);
        else if(status < 0)
        {
            printf("RECV_ERROR: %d\n",  SOCKET_ERROR);
            break;
        }

        int endpos;
        string filePathAndName = filePath,RequestHeader = buffer;
        endpos = RequestHeader.find(" HTTP/1.1");
        filePathAndName.append("\\").append(RequestHeader,5,endpos-4);     // "GET /"占4位
        cout<<"<==========    客户端请求的文件："<<filePathAndName<<"  文件大小："<<GetFileLength(filePathAndName)<<"字节    ==========>\n"<<endl;

        // 发送数据，即WebServer发送的响应报文
        memset(buffer, 0, BUFFER_SIZE);
        sprintf(buffer, HEADER, GetFileLength(filePathAndName));       // 填充 WebServer 的响应头中的长度字段，将响应头存入buffer数组中
        if(send(socketConn, buffer, strlen(buffer), 0) == SOCKET_ERROR)
        {
            printf("HTTP响应包发送失败!\n");
            break;
        }
        else
        {
            printf("发送的HTTP响应报文：\n%s", buffer);
        }

        // 读取客户端请求的文件并发送
        ifstream fin(filePathAndName, ios::in | ios::binary);
        if (fin.is_open())
        {
            memset(buffer, 0, BUFFER_SIZE);
            while (fin.read(buffer, BUFFER_SIZE - 1))           // read()函数返回值为实际读取的字节数，第二个参数为每次读取的长度
            {
                if(send(socketConn, buffer, strlen(buffer), 0) == SOCKET_ERROR)
                {
                    printf("客户端请求的文件发送失败!\n");
                    break;
                }
                //printf("发送的文件数据：\n%s\n\n", buffer);
                memset(buffer, 0, BUFFER_SIZE);
            }
            if(send(socketConn, buffer, strlen(buffer), 0) == SOCKET_ERROR)
            {
                printf("客户端请求的文件发送失败!\n");
                break;
            }
            //printf("发送的文件数据：\n%s\n\n", buffer);
        }
        fin.close();

        // 关闭套接字
        closesocket(socketConn);

        /*
        if(!loopChoose.compare("0"))         // 0，重新选择文件
        {
            closesocket(socketServer);
            WSACleanup();
            goto LOOP;
        }
        */
    }

    closesocket(socketServer);
    WSACleanup();

    return 0;
}
