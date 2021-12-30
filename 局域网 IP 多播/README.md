## 原理
&emsp;&emsp; 在同一个多播组中，由其中一台主机发出的任何数据均会一模一样地复制一份，发给组内的每个成员，也可以发送给其自身。<br>

&emsp;&emsp; 对于每一个应用程序，创建一个 SOCK_DGRAM 类型的套接字。将其绑定到一个 IP 地址和端口上，以接收其它多播组成员发送的多播数据，最后将该套接字加入到具有同一个多播地址的多播组中。通过设置 sendto()方法的目标地址为多播组的地址，即可向多播组发送数据，通过 recvfrom()方法即可接收其它主机发送的多播数据。

## 使用方法

+ 在局域网的不同主机上同时运行 `IpMulticast.exe` 

## 示例

<h4>主机 1 运行界面：</h4>

[![TRPZNQ.png](https://s4.ax1x.com/2021/12/30/TRPZNQ.png)](https://imgtu.com/i/TRPZNQ)

<h4>主机 2 运行界面：</h4>

[![TRPehj.png](https://s4.ax1x.com/2021/12/30/TRPehj.png)](https://imgtu.com/i/TRPehj)
