  | Big_Bonus/
    |
    |--pktRxTx/ 修改后的pktRxTx程序源码，编译后选择网卡、输入send指令即可发送plane程序
      |
      |--pktRxTx.exe Win下可执行程序，以pktRxTx.exe -m 2命令运行，选择网卡后可以通过输入send命令将同一目录的plane文件进行发送
      |
      |--libs&includes/ Win下编译依赖文件。如果需要重新编译，环境配置见其中的readme.txt文件
    |
    |--start_code/ 实验源码目录
      |
      |--pass.sh：自制脚本，以bash pass.sh命令执行。-f等效为make all&floppy，并将编译出的plane文件拷贝至pktRxTx目录下
      |
      |--test/ 测试程序相关源码
        |
        |--bonus.c 网卡初始化任务及文件接收任务
        |
        |--test_bonus/ drawing.c程序源码及部分依赖函数、头文件。编译选项在start_code/的Makefile中，入口地址设定为0x40000000




启动操作：
	1、[linux]在start_code/目录下执行bash pass.sh -f(重编译及执行floppy，并将新生成的plane文件拷贝到pktRxTx/目录下)
	2、[linux]执行sudo minicom命令，输入loadboot
	3、[linux]执行mkfs命令(文件系统初始化。在板卡上有一定概率卡死，插拔sd卡或重启板卡可以解决)
	4、[linux]执行exec 0命令(网卡初始化)
	5、[linux]执行exec 1命令(启动收包任务)
	6、[Windows]执行pktRxTx -m 2，选择对应网卡后输入send命令(需要保证pktRxTx/目录下有拷贝好的plane文件)
	7、[linux]执行ls指令可以看到已有plane文件存在，执行ps指令可以看到仅有shell一个进程。此时可以重启开发板
	8、[linux]执行./plane 12命令，启动飞机。这里的12可以换成0~12的任意参数。由于shell范围及屏幕范围设置，超过这一范围可能会有屏幕打印错误
	8.1、[linux]如果要让屏幕上同时显示4架飞机，四次执行的参数需分别为0/4/8/12，顺序可以自由变动