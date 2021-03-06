## 原理
&emsp;&emsp; Tracert 程序关键是对 IP 头部生存时间(time to live)TTL 字段的使用，程序实现时是向目地主机发送一个 ICMP 回显请求消息，初始时 TTL 等于 1，这样当该数据报抵达途中的第一个路由器时，TTL 的值就被减为 0，导致发生超时错误，因此该路由生成一份 ICMP 超时差错报文返回给源主机。随后，主机将数据报的 TTL 值递增 1，以便 IP 报能传送到下一个路由器，并由下一个路由器生成 ICMP 超时差错报文返回给源主机。不断重复这个过程，直到数据报达到最终的目地主机，此时目地主机将返回 ICMP 回显应答消息。这样，源主机只需对返回的每一份 ICMP 报文进行解析处理，就可以掌握数据报从源主机到达目地主机途中所经过的路由信息。<br><br>
&emsp;&emsp; Ping 程序只需要在 Tracert 程序的基础上进行部分改动，局域网内两台主机之间进行通信，只需要经过一跳，因此若要想测试本局域网的所有机器是否在线，只需要将报文中的TTL值设为1即可，如果能收到 ICMP 回显应答消息而不是超时差错或不可达报文，即说明该 IP 地址对应的主机在线。

## 注意事项

+ 需要关闭 Windows 防火墙，否则防火墙会过滤 ICMP 超时差错报文，导致接收不到数据报而显示超时
+ 最好不要使用公共 WIFI，好像也会影响效果
+ 使用时请关闭 VPN，可能也会被其过滤超时差错报文
+ ...
+ 总之就是一个很简陋的 Demo，运行有问题的话重新编译运行多试试几次就好了🙁

## 示例

<h4>本 Tracert 程序运行结果</h4>

[![T2i6nf.png](https://s4.ax1x.com/2021/12/29/T2i6nf.png)](https://imgtu.com/i/T2i6nf)
[![T2isjP.png](https://s4.ax1x.com/2021/12/29/T2isjP.png)](https://imgtu.com/i/T2isjP)
[![T2igHS.png](https://s4.ax1x.com/2021/12/29/T2igHS.png)](https://imgtu.com/i/T2igHS)
[![T2iWNQ.png](https://s4.ax1x.com/2021/12/29/T2iWNQ.png)](https://imgtu.com/i/T2iWNQ)
[![T2iRAg.png](https://s4.ax1x.com/2021/12/29/T2iRAg.png)](https://imgtu.com/i/T2iRAg)

<h4>CMD Tracert 命令运行结果参考</h4>

[![T2ifhj.png](https://s4.ax1x.com/2021/12/29/T2ifhj.png)](https://imgtu.com/i/T2ifhj)

<h4> Ping 命令运行结果</h4>

[![T2i49s.png](https://s4.ax1x.com/2021/12/29/T2i49s.png)](https://imgtu.com/i/T2i49s)
[![T2iIcq.png](https://s4.ax1x.com/2021/12/29/T2iIcq.png)](https://imgtu.com/i/T2iIcq)
