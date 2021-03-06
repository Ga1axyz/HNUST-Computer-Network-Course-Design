## 原理
&emsp;&emsp; 服务器端和客户端首先建立 WinSock 通信，建立连接以后，客户端和服务器端就可以使用 send()方法和 recv( )方法来接收和发送数据，实现点对点的通信。当需要退出通信时，使用 closeSocket()方法来关闭相应的套接字句柄即可。

&emsp;&emsp; 服务端 WinSock 编程的基本流程：
1. 加载套接字库
2. 创建套接字
3. 创建并填充服务端的 SOCKADDR_IN 结构，包含地址族、IP 和端口信息，然后将其作为参数，将套接字绑定到一个 IP 地址和端口上
4. 将套接字设置为监听模式等待客户端的连接请求
5. 服务端接受客户端的连接请求后，即可使用返回的套接字和客户端进行通信
6. 通信结束后，关闭套接字，关闭加载的套接字库

&emsp;&emsp; 客户端 WinSock 编程的基本流程：
1. 加载套接字库
2. 创建套接字
3. 创建并填充服务端的 SOCKADDR_IN 结构
4. 将 SOCKADDR_IN 作为参数，向服务器发出连接请求
5. 待服务端接受连接请求后，即可使用步骤二创建的套接字和服务端进行通信
6. 通信结束后，关闭套接字，关闭加载的套接字库

## 使用方法

+ 先运行服务端 socketServer
+ 再运行客户端 socketClient
+ 只支持半双工通信
+ 客户端首先发言

## 示例
<h4>socketServer</h4>

[![T2ix3R.png](https://s4.ax1x.com/2021/12/29/T2ix3R.png)](https://imgtu.com/i/T2ix3R)

<h4>socketClient</h4>

[![T2irct.png](https://s4.ax1x.com/2021/12/29/T2irct.png)](https://imgtu.com/i/T2irct)
