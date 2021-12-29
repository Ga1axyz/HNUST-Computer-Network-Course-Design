## 原理

&emsp;&emsp; WebServer 是 B/S 结构中的服务端，可以利用浏览器作为本题的客户端以减少工作量。WebServer 主要包括两部分的功能，一是处理客户端的请求，该部分负责监听系统的端口，接收客户端的请求，二是生成并发送对客户请求的响应，该部分负责分析请求中的各个协议参数，根据请求的分析结果查找资源，生成响应和发送响应。<br>
&emsp;&emsp; WebServer 和客户端首先建立 WinSock 通信，建立连接后即可实现相互之间的信息交换。将 HTTP 响应报文和请求报文作为数据利用 Socket 进行互相传递，即可实现 Web 服务器和客户端之间的通信。WebServer 对客户端发送的 HTTP 请求报文进行解析处理，寻找到其请求的相应文件，将文件作为数据发送给客户端即可实现客户端对服务器中的资源请求。

## 使用方法

+ 控制台中可设置 WebServer 的端口、根目录
+ 支持客户端通过 Get 方法来请求服务器中的资源，即浏览器 url

## 示例

[![T2iHBT.png](https://s4.ax1x.com/2021/12/29/T2iHBT.png)](https://imgtu.com/i/T2iHBT)
[![T2i7uV.png](https://s4.ax1x.com/2021/12/29/T2i7uV.png)](https://imgtu.com/i/T2i7uV)
[![T2ioj0.png](https://s4.ax1x.com/2021/12/29/T2ioj0.png)](https://imgtu.com/i/T2ioj0)
[![T2i53n.png](https://s4.ax1x.com/2021/12/29/T2i53n.png)](https://imgtu.com/i/T2i53n)
