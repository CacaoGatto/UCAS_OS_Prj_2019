char value[5] = "2019\0";

void __attribute__((section(".entry_function"))) _start(void)
{
	// Call PMON BIOS printstr to print message "Hello OS!"
	
	/*指针函数调用版本*/
	char* msg = "Hello OS!\nversion:";
	void (*printstr) (char*) = (void*) 0x80011100;
	//定义函数指针指向printstr函数所在地址
	(*printstr) (msg); //调用printstr函数
    msg = &value[0];
    (*printstr) (msg);
//    while(1) ;
//    __asm__("stop: j stop");

	/*嵌入式汇编版本*/
/*
	char* msg = "Hello OS!\n";
	asm("move $a0, %0\n\t"
        "li $t0, %1\n\t"
        "jal $t0"
        :
        :"r"(msg),"i"(0x80011100) //标注所使用的内存数和立即数
        :"a0","t0"); //标注相关寄存器
*/

	return;
}
