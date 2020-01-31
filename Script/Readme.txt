GKD计系操作系统实验课用脚本（MIPS框架）

2091.1.9更新：
①增加了对-n和-s和-r和-l选项的支持，分别对应网桥分配、发包（要求pktRxTx文件夹和start_code文件夹同级，或自行调整路径）、收包、gzram文件替换（要求把gzram放入start_code文件夹，或自行调整路径）。虽然实验课已经结束&明年就要换riscv框架了，但还是把现在的完整版更新上来吧

2019.11.12更新：
①增加了编译错误时终止脚本运行的设置，防止后续操作全部木大
②不需要额外借助gdb配置文件，脚本会自动生成和清除
③修改脚本名为pass.sh，让你的每次运行都能获得祈祷一般的玄学加成
④支持-c选项，执行后会在~/目录下生成连接向start_code的脚本，且自带sudo mount...指令，重启虚拟机/shell重连后不再需要手动完成共享文件夹的挂载
⑤支持-f选项，一键实现编译和sd卡拷贝
⑥优化了部分变量调用，方便自行调整配置



运行方法：
将pass.sh文件拷贝到start_code目录下，输入"bash pass.sh"即可看到使用说明。脚本也支持自定义配置，包括：
·源码第4行用户密码password，默认为123456，如果有不同请自行修改
·源码第5行QEMU目录qemu_path，默认为~/QEMULoongson，如果有不同请自行修改
·源码第7~8行GDB启动预配置setting，如果有其他固定需求可以自行添加(symbol-file已默认导入)

使用说明另附如下：

This is a script for UCAS's OS projects. USAGE:

    bash pass.sh [option]

Options:

    -q:     Compile the image and test in QEMU of Loongson.
            (Make sure there has been a visual "disk" file under your qemu_path.)
    -g:     Connect GDB with symbol-file under arch mips.
            (The fucking useless version info will not be printed.)
    -c:     Create "pass.sh" under "~/".
            (Bash it with the same usage. No need inputting "sudo mount..." after reconnection.)
    -n:     Connect net bridge tap0.
            (Bash it before using parameter "-q" in P5)
    -s:     Execute pktRxTx with parameter "-m -2" to send packages.
            (Bash with parameter "-n" first)
    -r:     Call tcpdump to check package recieving in tap0.
            (Bash with parameter "-n" first)
    -l:     Copy "gzram" file to qemu's bios directory.
    -f:     Make clean, make all and make floppy.
    -h:     Print this message.

Report bugs to "<cacaogattoxy@gmail.com>". The author never checks his mailbox though.XDD
