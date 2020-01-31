【Win环境下pktRxTx编译环境配置】

必要环境：Visual Studio(本机上使用2019版本)
本机的VC文件夹中头文件及库路径：
%PATH%=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.24.28314\

1、将WpdPack目录复制到D盘下
2、将Include/下所有文件复制到VC文件夹的include\目录下
	(以本机为例：%PATH%include\)
3、将Lib/下所有文件复制到VC文件夹的lib\x86\目录下
	(以本机为例：%PATH%lib\x86\)
4、在Visual Studio自带的命令行(x86 Native Tools Command Prompt for VS 2019)中进入pktRxTx目录，执行build.bat即可