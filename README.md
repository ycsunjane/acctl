简介
=======

Openwrt ac 控制器(acctl)， 包含服务端和客户端两大块。支持以下特性:

* 同一网段，二层发现，自动配置ip
* 客户端主动链接，穿透路由


目录
=======

ac: 服务器代码
ap: 客户端代码
lib: 共用代码
include: 共用头文件
scripts: 编译等辅助脚本


ac
================

ac 为多线程，包含以下线程:

* net_recv: 接收二层或者三层数据包
* net_netlisten: 监听tcp链接
* net_dllbrd: 二层广播
* message_travel: 定时处理收到的报文
* 主线程： 用于命令行查看系统状态

- net_dllbrd 线程广播探测报文, 报文中携带ac的地址和监听端口
- ap接收到探测报文后，发起tcp链接
- ac 将ap信息插入aphash
- ap 定时汇报状态
- ac 从数据库中提出配置，与状态比较，并发出命令
- 当tcp链接断开时，认为ap掉线
- 删除aphash记录


ap
================

ap 为多线程，包含以下线程:

* net_recv: 接收二层或者三层数据包
* message_travel: 定时处理收到的报文
* 主线程: 状态查询
