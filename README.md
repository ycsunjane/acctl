简介
=======

Openwrt ac 控制器(acctl)， 包含服务端和客户端两大块。支持以下特性:

* 同一网段，二层发现，自动配置ip
* 自动二层/三层发送控制信息
* 二层主动探测网络中ap，可接管其他ac控制ap
* 客户端主动链接，并保持长链接


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

启动过程:

* 初始化ac控制器uuid
* 初始化epoll
* 初始化二层socket, 加入epoll
* 启动net_recv线程，收包（此时只接收二层数据包）
* 初始化三层tcp listen线程, 等待tcp 链接
* 初始化二层广播，汇报服务器地址，端口，ac uuid

ap
================

ap 为多线程，包含以下线程:

* net_recv: 接收二层或者三层数据包
* message_travel: 定时处理收到的报文
* 主线程: 状态查询


chap 二层保护
=================

编译时makefile会要求输入password， 客户端和服务端必须密码一致，用于报文确认。random 用于防止重放攻击。

1、发送broadcast报文， 生成broadcast `random0`
2、ap接收到broadcast报文, 生成`random1`, 对`数据+ random0 + password`计算`md5sum1`, 发送ap reg 报文。
3、ac收到ap reg报文，提取`md5sum1`并置原报文位置为0, 对`报文 + random0 + password`计算`md5sum2`, 如果md5sum1 与 md5sum2 不一致，则丢弃。ac 生成reg response 报文，生成`random2`, 对`数据 + random1 + password`计算`md5sum3`
4、ap 收到reg response 报文，提取`md5sum3`并置原报文位置为0, 对`报文 + random1 + password`计算`md5sum3`, 如果md5sum3 与 md5sum4 不一致，则丢弃

