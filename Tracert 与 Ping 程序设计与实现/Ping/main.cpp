#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <ws2tcpip.h>
#include <winsock2.h>
#include <iostream>
#include <windows.h>

using namespace std;

//IP报头
typedef struct IP_HEADER
{
    unsigned char hdr_len:4;       //4位头部长度
    unsigned char version:4;       //4位版本号
    unsigned char tos;             //8位服务类型
    unsigned short total_len;      //16位总长度
    unsigned short identifier;     //16位标识符
    unsigned short frag_and_flags; //3位标志加13位片偏移
    unsigned char ttl;             //8位生存时间
    unsigned char protocol;        //8位上层协议号
    unsigned short checksum;       //16位校验和
    unsigned long sourceIP;        //32位源IP地址
    unsigned long destIP;          //32位目的IP地址
} IP_HEADER;

//ICMP报头
typedef struct ICMP_HEADER
{
    unsigned char type;    //8位类型字段
    unsigned char code;    //8位代码字段
    USHORT cksum; //16位校验和
    USHORT id;    //16位标识符
    USHORT seq;   //16位序列号
} ICMP_HEADER;

//报文解码结构
typedef struct DECODE_RESULT
{
    USHORT usSeqNo;        //序列号
    unsigned long dwRoundTripTime; //往返时间
    in_addr dwIPaddr;      //返回报文的IP地址
} DECODE_RESULT;

//计算网际校验和函数
USHORT checksum( USHORT *pBuf, int iSize )
{
    unsigned long cksum = 0;
    while( iSize > 1 )
    {
        cksum += *pBuf++;
        iSize -= sizeof(USHORT);
    }
    if( iSize )//如果 iSize 为正，即为奇数个字节
    {
        cksum += *(UCHAR *)pBuf; //则在末尾补上一个字节，使之有偶数个字节
    }
    cksum  = ( cksum >> 16 ) + ( cksum&0xffff );
    cksum += ( cksum >> 16 );
    return (USHORT)( ~cksum );
}

//对数据包进行解码
BOOL DecodeIcmpResponse(char * pBuf, int iPacketSize, DECODE_RESULT &DecodeResult,unsigned char ICMP_ECHO_REPLY, unsigned char ICMP_TIMEOUT)
{
    //检查数据报大小的合法性
    IP_HEADER* pIpHdr = ( IP_HEADER* )pBuf;
    int iIpHdrLen = pIpHdr->hdr_len * 4;    //ip报头的长度是以4字节为单位的

    //若数据包大小 小于 IP报头 + ICMP报头，则数据报大小不合法
    if ( iPacketSize < ( int )( iIpHdrLen + sizeof( ICMP_HEADER ) ) )
        return FALSE;

    //根据ICMP报文类型提取ID字段和序列号字段
    ICMP_HEADER *pIcmpHdr = ( ICMP_HEADER * )( pBuf + iIpHdrLen );//ICMP报头 = 接收到的缓冲数据 + IP报头的长度
    USHORT usID, usSquNo;

    if( pIcmpHdr->type == ICMP_ECHO_REPLY )    //ICMP回显应答报文
    {
        usID = pIcmpHdr->id;        //报文ID
        usSquNo = pIcmpHdr->seq;    //报文序列号

        //cout<< "Debug:收到ICMP回显应答报文\n" << flush;
    }
    else if( pIcmpHdr->type == ICMP_TIMEOUT )//ICMP超时差错报文
    {
        char * pInnerIpHdr = pBuf + iIpHdrLen + sizeof( ICMP_HEADER ); //载荷中的IP头
        int iInnerIPHdrLen = ( ( IP_HEADER * )pInnerIpHdr )->hdr_len * 4; //载荷中的IP头长
        ICMP_HEADER * pInnerIcmpHdr = ( ICMP_HEADER * )( pInnerIpHdr + iInnerIPHdrLen );//载荷中的ICMP头

        usID = pInnerIcmpHdr->id;        //报文ID
        usSquNo = pInnerIcmpHdr->seq;    //序列号

        //cout<< "Debug:收到ICMP超时差错报文\n" << flush;
    }
    else
    {
        return false;
    }

    //检查ID和序列号以确定收到期待数据报
    if( usID != ( USHORT )GetCurrentProcessId() || usSquNo != DecodeResult.usSeqNo )
    {
        return false;
    }
    //记录IP地址并计算往返时间
    DecodeResult.dwIPaddr.s_addr = pIpHdr->sourceIP;
    DecodeResult.dwRoundTripTime = GetTickCount() - DecodeResult.dwRoundTripTime;
    //cout<< "Debug:收到ICMP响应报文中的TTL = " << (int)pIpHdr->ttl << "\n" << flush;
    //cout<< "Debug:ICMP响应报文的发送端IP地址 = " << inet_ntoa(DecodeResult.dwIPaddr) << "\n" << flush;

    //处理正确收到的ICMP数据报
    if ( pIcmpHdr->type == ICMP_ECHO_REPLY || pIcmpHdr->type == ICMP_TIMEOUT )
    {
        //输出往返时间信息
        if(DecodeResult.dwRoundTripTime)
            cout<< "     " << DecodeResult.dwRoundTripTime << "ms" <<flush;
        else
            cout<< "     " << "<1ms" << flush;
    }
    return true;
}

int main()
{
    //初始化Windows sockets网络环境
    WSADATA wsa;
    WSAStartup( MAKEWORD(2,2), &wsa );
    char IpAddress[255];
    cout<<"请输入一个IP地址或域名：";
    cin>>IpAddress;

    //得到IP地址
    u_long ulDestIP = inet_addr( IpAddress );

    //转换不成功时按域名解析
    if( ulDestIP == INADDR_NONE )
    {
        hostent * pHostent = gethostbyname( IpAddress );
        if( pHostent )
        {
            ulDestIP = ( *( in_addr* )pHostent->h_addr).s_addr;
        }
        else
        {
            cout<<"输入的IP地址或域名无效!"<<endl;
            WSACleanup();
            return 0;
        }
    }
    cout<<"Tracing roote to "<<IpAddress<<" with a maximum of 30 hops.\n"<<endl;

    //填充目的端socket地址
    sockaddr_in destSockAddr;
    ZeroMemory( &destSockAddr, sizeof( sockaddr_in ) );
    destSockAddr.sin_family = AF_INET;
    destSockAddr.sin_addr.s_addr = ulDestIP;
    cout<< "Debug:目的端 IP地址 = " << inet_ntoa(destSockAddr.sin_addr) << "\n\n" << flush;

    //创建原始套接字
    SOCKET sockRaw;
    if((sockRaw = WSASocket( AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED )) == INVALID_SOCKET)   // INVALID_SOCKET意思是无效套接字
    {
        cout<< "创建原始套接字失败\n" << flush;
        return 0;
    }
    //SOCKET sockRaw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    //超时时间
    int iTimeout = 3000;

    //设置接收超时时间
    setsockopt(sockRaw,SOL_SOCKET,SO_RCVTIMEO,(char *)&iTimeout,sizeof(iTimeout));

    //设置发送超时时间
    setsockopt(sockRaw,SOL_SOCKET,SO_SNDTIMEO,(char *)&iTimeout,sizeof(iTimeout));

    //构造ICMP回显请求消息，并以TTL递增的顺序发送报文
    //ICMP类型字段
    const unsigned char ICMP_ECHO_REQUEST = 8;    //请求回显
    const unsigned char ICMP_ECHO_REPLY   = 0;    //回显应答
    const unsigned char ICMP_TIMEOUT      = 11;   //传输超时

    //其他常量定义
    const int DEF_ICMP_DATA_SIZE   = 32;    //ICMP报文默认数据字段长度
    const int MAX_ICMP_PACKET_SIZE = 1024;  //ICMP报文最大长度（包括报头）
    const unsigned long DEF_ICMP_TIMEOUT   = 3000;  //回显应答超时时间
    const int DEF_MAX_HOP          = 30;    //最大跳站数

    //填充ICMP报文中每次发送时不变的字段
    char IcmpSendBuf[ sizeof( ICMP_HEADER ) + DEF_ICMP_DATA_SIZE ];//发送缓冲区
    memset( IcmpSendBuf, 0, sizeof( IcmpSendBuf ) );               //初始化发送缓冲区
    char IcmpRecvBuf[ MAX_ICMP_PACKET_SIZE ];                      //接收缓冲区
    memset( IcmpRecvBuf, 0, sizeof( IcmpRecvBuf ) );               //初始化接收缓冲区

    ICMP_HEADER * pIcmpHeader = ( ICMP_HEADER* )IcmpSendBuf;
    pIcmpHeader->type = ICMP_ECHO_REQUEST; //类型为请求回显
    pIcmpHeader->code = 0;                //代码字段为0
    pIcmpHeader->id   = (USHORT)GetCurrentProcessId();    //ID字段为当前进程号
    memset( IcmpSendBuf + sizeof( ICMP_HEADER ), 'E', DEF_ICMP_DATA_SIZE );//数据字段

    USHORT usSeqNo      = 0;            //ICMP报文序列号
    int iTTL = 1;                       //TTL初始值为1
    int len = sizeof(int);
    int setedttl = iTTL;
    BOOL bReachDestHost = FALSE;        //循环退出标志
    int iMaxHot         = DEF_MAX_HOP;  //循环的最大次数
    DECODE_RESULT DecodeResult;    //传递给报文解码函数的结构化参数
    while( !bReachDestHost && iMaxHot-- )
    {
        //cout<< "\nDebug:iTTL = " << iTTL << "\n" << flush;

        getsockopt(sockRaw,IPPROTO_IP,IP_TTL,(char *)&setedttl,&len);
        cout<< "Debug:设置前的TTL = " << setedttl << "\n" << flush;

        cout<< "Debug:设置TTL为：" << iTTL << "\n" << flush;
        //设置IP报头的TTL字段
        if(setsockopt(sockRaw,IPPROTO_IP, IP_TTL, (char *)&iTTL, sizeof(iTTL)) == SOCKET_ERROR){
            cout<< "无法设置 sockRaw" << "\n" << flush;
            return 0;
        }
        //cout<<iTTL<<flush;    //输出当前序号,flush表示将缓冲区的内容马上送进cout,把输出缓冲区刷新

        getsockopt(sockRaw,IPPROTO_IP,IP_TTL,(char *)&setedttl,&len);
        cout<< "Debug:设置后的TTL = " << setedttl << "\n" << flush;

        //填充ICMP报文中每次发送变化的字段
        ((ICMP_HEADER *)IcmpSendBuf)->cksum = 0;                   //校验和先置为0
        ((ICMP_HEADER *)IcmpSendBuf)->seq   = htons(usSeqNo++);    //填充序列号
        ((ICMP_HEADER *)IcmpSendBuf)->cksum = checksum( ( USHORT * )IcmpSendBuf, sizeof( ICMP_HEADER ) + DEF_ICMP_DATA_SIZE ); //计算校验和

        //记录序列号和当前时间
        DecodeResult.usSeqNo         = ( ( ICMP_HEADER* )IcmpSendBuf )->seq;    //当前序号
        DecodeResult.dwRoundTripTime = GetTickCount();                          //当前时间

        //DecodeResult.dwIPaddr.s_addr = inet_addr("127.0.0.1");////////////////////////////////////////////////////////////

        //发送TCP回显请求信息
        if(sendto(sockRaw, IcmpSendBuf, sizeof(IcmpSendBuf), 0, (sockaddr*)&destSockAddr, sizeof(destSockAddr)) == SOCKET_ERROR)
        {
            cout<< "Debug:ICMP报文发送错误 错误代码：" << WSAGetLastError() << "\n" << flush;
            return 0;
        }

        //接收ICMP差错报文并进行解析处理
        sockaddr_in from;           //对端socket地址
        int iFromLen = sizeof(from);//地址结构大小
        int iReadDataLen;           //接收数据长度
        while(1)
        {
            //接收数据
            iReadDataLen = recvfrom( sockRaw, IcmpRecvBuf, MAX_ICMP_PACKET_SIZE, 0, (sockaddr*)&from, &iFromLen );

            if( iReadDataLen != SOCKET_ERROR )//有数据到达
            {
                cout<< "+-------------------------------------+\n     " << iTTL << flush;
                //对数据包进行解码
                if(DecodeIcmpResponse( IcmpRecvBuf, iReadDataLen, DecodeResult, ICMP_ECHO_REPLY, ICMP_TIMEOUT ) )
                {
                    //到达目的地，退出循环
                    if( DecodeResult.dwIPaddr.s_addr == destSockAddr.sin_addr.s_addr )
                        bReachDestHost = true;
                    //输出IP地址
                    cout<<'\t'<< " " << inet_ntoa( DecodeResult.dwIPaddr )<<endl;
                    cout<< "+-------------------------------------+\n";
                    break;
                }
            }
            else if( WSAGetLastError() == WSAETIMEDOUT )    //接收超时，输出*号
            {
                cout<< "+-------------------------------------+\n     " << iTTL << flush;
                cout<<"     *"<<'\t'<< " " <<"Request timed out."<<endl;
                cout<< "+-------------------------------------+\n";
                break;
            }
            else
            {
                break;
            }
        }
        iTTL++;    //递增TTL值
        //cout<< "Debug:已收到ICMP响应报文，将TTL值加1 ITTL = " << iTTL << "\n" << flush;
        cout<< "Debug:bReachDestHost = " << bReachDestHost << "\n" << flush;
        cout<< "Debug:iMaxHot = " << iMaxHot << "\n\n" << flush;
    }

    return 0;
}
