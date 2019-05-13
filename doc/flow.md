### ServerGuard
- (Main) connect TaskManager 建立 TCP 连接 con1, recv(con1) 等待数据
- (Main) TaskManager 连接并发送 Task 数据后, 编译代码并执行 Task
- (Main) 启动 **本地** socket UDP server (con2), recv(con2) 等待数据
- (Main) 接收到 Task 发送的远程数据获取指令
  - send(con1) 发送给 TaskManager 所需数据信息
  - recv(con1) 等待接受数据或操作结果, 接收后存入共享内存
  - send(con2) 通知 Task 共享内存 ID, 继续 recv(con2) 等待确认
  - task 确认传输完毕, 卸载、删除共享内存区
- (Main) 接收到 Task 发送的建立有名内存区指令
  - send(con1) 发送给 TaskManager 请求
  - recv(con1) 等待执行结果, 转发执行结果给 Task
- (Main) 接收到 Task 发送的写入有名内存区指令
  - 根据 Task 给出的共享内存 ID 读取待写数据
  - send(con1) 发送给 TaskManager 内存区 ID 和数据
  - recv(con1) 等待执行结果, 转发执行结果给 Task
- (Main) 接收到 Task 发送的结束指令
  - send(con1) 通知 TaskManager server 已闲置

### TaskManager
- (Main) 建立线程 Th.0
    - (Th.0) 作为服务端监听 TCP 连接
    - (Th.0) 与某 server 建立连接 con_i, 新建线程 Th.i
    - (Th.i) 阻塞等待主线程信号
- (Main) 建立线程 Th.X
    - (Th.X) 阻塞等待信号唤醒
    - (Th.X) 得到信号后选取执行程序的在线 server, 传送任务数据, signal 相应线程 Th.i, 返回上一步
- (Main) 循环输出服务器列表和状态, 等待用户按键
- (Main) 用户按键后停止循环输出, 等待用户指定入口代码文件, 随后 signal Th.X 安排任务
- (Th.i) send(con_i) 向 server 发送任务相关数据, recv(con_i) 等待 server 的请求
- (Th.i) server 请求进行操作
    - 内存操作(新建.读取.写入): 返回结果或数据
    - 对象操作(Mutex ..): 返回结果
    - 新建线程, signal Th.X (此处保证互斥唤醒 Th.X), 返回结果
- (Th.i) 服务器发送闲置通知, 在服务器列表中修改该服务器的状态
- (Th.i) 连接断开 服务器下线 退出线程

### About the Task
- 一个 Task 需要: 给出 make pipe; 指明将被执行的可执行程序; 其他附带文件;
- 将 Task 文件打包 zip
- Server 解压 zip, 执行 make, 然后执行目标程序
- 向 TaskManager 指定任务时直接指定 zip 文件路径
- 在 TaskManager 上提交主进程，主进程运行后通过 API 指示 TaskManager 启动各子进程；