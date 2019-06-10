// #include <8052.h>
#include <reg52.h>
#include <intrins.h>
#include <string.h>

#define uchar unsigned char
#define uint unsigned int
#define addr1 'a'
#define LCD1602_DB  P2  

// #define led P1_0
// #define led1 P1_1
// #define led2 P1_2
// sbit led = P1 ^ 0;
// sbit led1 = P1 ^ 1;
// sbit led2 = P1 ^ 2;
// sbit led3 = P1 ^ 3;
// sbit led4 = P1 ^ 4;
uchar buf;
uchar smod_buf[15];
uchar temp_buf[15];
uchar address_ok;
uchar RECE_status;
uchar address_status, command_status, i;
sbit LCD1602_RS = P0^7;  //1602液晶指令/数据选择引脚
sbit LCD1602_RW = P0^6;  //1602液晶读写引脚
sbit LCD1602_EN = P0^5;  //1602液晶使能引脚
/*--------------------------头文件-------------------------*/
sfr P1ASF = 0x9d;        //P1口模拟功能控制位    Bit7    Bit6    Bit5    Bit4    Bit3    Bit2    Bit1    Bit0
                         //位描述                P17ASF  P16ASF  P15ASF  P14ASF  P13ASF  P12ASF  P11ASF  P10ASF
                         //初始值=0000,0000      0       0       0       0       0       0       0       0
sfr ADC_RES = 0xbd;      //ADC结果高字节         Bit7    Bit6    Bit5    Bit4    Bit3    Bit2    Bit1    Bit0
                         //初始值=0000,0000      0       0       0       0       0       0       0       0
sfr ADC_RESL = 0xbe;     //ADC结果低字节         Bit7    Bit6    Bit5    Bit4    Bit3    Bit2    Bit1    Bit0
                         //初始值=0000,0000      0       0       0       0       0       0       0       0
sfr ADC_CONTR = 0xbc;    //ADC控制寄存器         Bit7    Bit6    Bit5    Bit4    Bit3    Bit2    Bit1    Bit0
                         //位描述            ADC_POWER  SPEED1 SPEED0 ADC_FLAG ADC_START CHS2    CHS1    CHS0
                         //初始值=0000,0000      0       0       0       0       0       0       0       0
#define ADC_POWER 0x80   //ADC模块电源控制位
#define ADC_SPEEDLL 0x00 //每次转换需要420个时钟周期
#define ADC_SPEEDL 0x20  //每次转换需要280个时钟周期
#define ADC_SPEEDH 0x40  //每次转换需要140个时钟周期
#define ADC_SPEEDHH 0x60 //每次转换需要70个时钟周期
#define ADC_FLAG 0x10    //ADC转换完成标志
#define ADC_START 0x08   //ADC开始转换控制位
#define ADC_CHS2 0x04    //ADC通道选择位2
#define ADC_CHS1 0x02    //ADC通道选择位1
#define ADC_CHS0 0x01    //ADC通道选择位0
/*-----------------------头文件----------------------------*/

void delay_1ms(unsigned int i)
{
    unsigned int x, y;
    for (x = i; x > 0; x--)
        for (y = 110; y > 0; y--)
            ;
}
unsigned int ADCValue[11] = {0}; //设置缓冲数组

void InitADC() //ADC配置函数
{
    P1ASF = 0x03; //选择P1.0口为模拟量输入
    ADC_RES = 0x00;

    //	ADC_CONTR=ADC_POWER|ADC_SPEEDLL|ADC_START|ch;		//配置ADC_CONTR寄存器 并启动AD转换
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START; //配置ADC_CONTR寄存器 并启动AD转换
                                                     //	EADC=1;
}
void ADConvert(unsigned char ch) //AD值提取函数
{
    unsigned int count = 0, temp = 0; //设置两个int型变量
    while (count < 11)                //取AD值11次
    {
        while (!(ADC_CONTR & ADC_FLAG))
            ;                                                 //判断AD是否转换完毕
        ADC_CONTR &= ~ADC_FLAG;                               //将AD转换标志位置0
        temp = ADC_RES;                                       //取出转换的AD值高位
        temp <<= 2;                                           //左移两位方便与低两位组成10位
        temp |= ADC_RESL;                                     //与上低两位
        ADCValue[count] = temp;                               //将当前的AD值存储到缓冲数组
        temp = 0;                                             //清零中间变量
        ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START | ch; //再次启动AD转换
        ADC_RES = 0;                                          //清零高位
        ADC_RESL = 0;                                         //清零低位
        count++;                                              //转换次数加1
    }
}

unsigned int DigitalFiltering(unsigned char ch) //滤波函数
{
    unsigned int i, j, voltageADC = 0, temp = 0; //定义三个变量
    ADConvert(ch);                               //调用AD转换程序取出转换好的缓冲数据
    for (i = 0; i < 10; i++)                     //排序法，将缓冲区的数据从小到大重新整理放入缓冲数组
    {
        for (j = 0; j < (10 - i); j++)
        {
            if (ADCValue[j] > ADCValue[j + 1])
            {
                temp = ADCValue[j];
                ADCValue[j] = ADCValue[j + 1];
                ADCValue[j + 1] = temp;
            }
        }
    }
    for (i = 2; i < 8; i++) //滤除最小的四个数
    {
        voltageADC += ADCValue[i]; //将当前的6个数相加
    }
    return ADCValue[5]; //返回相加后的值
}

void Adc_Action(void)
{
    unsigned int adcx = 0;
    float smodValue;
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START; //配置ADC_CONTR寄存器 并启动AD转换
    adcx = DigitalFiltering(0);                      //将滤波后的AD值赋值给Value
    ADC_CONTR = 0;
    smodValue = (float)adcx * (4.8 / 1024);
    smodValue = (smodValue - 2) * 2000; //0.4为测量空气的中甲烷浓度值时的数值
    if (smodValue <= 0)
        smodValue = 0;
    adcx = smodValue;
    smod_buf[0] = adcx % 10000 / 1000 + 0x30;
    smod_buf[1] = adcx % 1000 / 100 + 0x30;
    smod_buf[2] = adcx % 100 / 10 + 0x30;
    smod_buf[3] = adcx % 10 + 0x30;
    smod_buf[4] = 'p';
    smod_buf[5] = 'p';
    smod_buf[6] = 'm';
    smod_buf[7] = '$';
}

sbit IO_18B20 = P3 ^ 7; //DS18B20通信引脚

/* STC12系列单片机在12M晶振下产生指定us数的软件延时 */
void Delay_us(unsigned char us)
{
    do
    {
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
        _nop_();
    } while (--us);
}
/* 复位总线，获取存在脉冲，以启动一次读写操作 */
bit Get18B20Ack()
{
    bit ack;

    EA = 0;       //禁止总中断
    IO_18B20 = 0; //产生500us复位脉冲
    Delay_us(250);
    Delay_us(250);
    IO_18B20 = 1;
    Delay_us(60);   //延时60us
    ack = IO_18B20; //读取存在脉冲
    while (!IO_18B20)
        ;   //等待存在脉冲结束
    EA = 1; //重新使能总中断

    return ack;
}
/* 向DS18B20写入一个字节，dat-待写入字节 */
void Write18B20(unsigned char dat)
{
    unsigned char mask;

    EA = 0;                                  //禁止总中断
    for (mask = 0x01; mask != 0; mask <<= 1) //低位在先，依次移出8个bit
    {
        IO_18B20 = 0; //产生2us低电平脉冲
        Delay_us(2);
        if ((mask & dat) == 0) //输出该bit值
            IO_18B20 = 0;
        else
            IO_18B20 = 1;
        Delay_us(60); //延时60us
        IO_18B20 = 1; //拉高通信引脚
    }
    EA = 1; //重新使能总中断
}
/* 从DS18B20读取一个字节，返回值-读到的字节 */
unsigned char Read18B20()
{
    unsigned char dat;
    unsigned char mask;

    EA = 0;                                  //禁止总中断
    for (mask = 0x01; mask != 0; mask <<= 1) //低位在先，依次采集8个bit
    {
        IO_18B20 = 0; //产生2us低电平脉冲
        Delay_us(2);
        IO_18B20 = 1;  //结束低电平脉冲，等待18B20输出数据
        Delay_us(2);   //延时2us
        if (!IO_18B20) //读取通信引脚上的值
            dat &= ~mask;
        else
            dat |= mask;
        Delay_us(60); //再延时60us
    }
    EA = 1; //重新使能总中断

    return dat;
}
/* 启动一次18B20温度转换，返回值-表示是否启动成功 */
bit Start18B20()
{
    bit ack;

    ack = Get18B20Ack(); //执行总线复位，并获取18B20应答
    if (ack == 0)        //如18B20正确应答，则启动一次转换
    {
        Write18B20(0xCC); //跳过ROM操作
        Write18B20(0x44); //启动一次温度转换
    }
    return ~ack; //ack==0表示操作成功，所以返回值对其取反
}
/* 读取DS18B20转换的温度值，返回值-表示是否读取成功 */
bit Get18B20Temp(int *temp)
{
    bit ack;
    unsigned char LSB, MSB; //16bit温度值的低字节和高字节

    ack = Get18B20Ack(); //执行总线复位，并获取18B20应答
    if (ack == 0)        //如18B20正确应答，则读取温度值
    {
        Write18B20(0xCC);              //跳过ROM操作
        Write18B20(0xBE);              //发送读命令
        LSB = Read18B20();             //读温度值的低字节
        MSB = Read18B20();             //读温度值的高字节
        *temp = ((int)MSB << 8) + LSB; //合成为16bit整型数
    }
    return ~ack; //ack==0表示操作应答，所以返回值为其取反值
}
int intT, decT, temp;
bit res = 0;
void refreshTemp()
{
    
    res = Get18B20Temp(&temp); //第一个DS18B20读取当前温度
    Start18B20();              //启动DS18B20
    if (res)                   //读取成功时，刷新当前温度显示
    {
        intT = temp >> 4;         //分离出温度值整数部分
        decT = temp & 0xF;        //分离出温度值小数部分
        decT = (decT * 100) / 16; //二进制的小数部分转换为1位十进制位
    }
    temp_buf[0] = intT % 100 / 10 + 0x30;
    temp_buf[1] = intT % 10 + 0x30;
    temp_buf[2] = '.';
    temp_buf[3] = decT % 100 / 10 + 0x30;
    temp_buf[4] = 'C';
    temp_buf[5] = '$';
}
/* --------------------------------------------LCD1602-----------------------------------------------------------------*/
/* 等待液晶准备好 */
void Lcd1602WaitReady()
{
    unsigned char sta;
    
    LCD1602_DB = 0xFF;
    LCD1602_RS = 0;
    LCD1602_RW = 1;
    do {
        LCD1602_EN = 1;
        sta = LCD1602_DB; //读取状态字
        LCD1602_EN = 0;
    } while (sta & 0x80); //bit7等于1表示液晶正忙，重复检测直到其等于0为止
}
/* 向LCD1602液晶写入一字节命令，cmd-待写入命令值 */
void Lcd1602WriteCmd(unsigned char cmd)
{
    Lcd1602WaitReady();
    LCD1602_RS = 0;
    LCD1602_RW = 0;
    LCD1602_DB = cmd;
    LCD1602_EN  = 1;
    LCD1602_EN  = 0;
}
/* 向LCD1602液晶写入一字节数据，dat-待写入数据值 */
void Lcd1602WriteDat(unsigned char dat)
{
    Lcd1602WaitReady();
    LCD1602_RS = 1;
    LCD1602_RW = 0;
    LCD1602_DB = dat;
    LCD1602_EN  = 1;
    LCD1602_EN  = 0;
}
/* 设置显示RAM起始地址，亦即光标位置，(x,y)-对应屏幕上的字符坐标 */
void Lcd1602SetCursor(unsigned char x, unsigned char y)
{
    unsigned char addr;
    
    if (y == 0)  //由输入的屏幕坐标计算显示RAM的地址
        addr = 0x00 + x;  //第一行字符地址从0x00起始
    else
        addr = 0x40 + x;  //第二行字符地址从0x40起始
    Lcd1602WriteCmd(addr | 0x80);  //设置RAM地址
}
/* 在液晶上显示字符串，(x,y)-对应屏幕上的起始坐标，str-字符串指针 */
void Lcd1602ShowStr(unsigned char x, unsigned char y, unsigned char *str)
{
    Lcd1602SetCursor(x, y);   //设置起始地址
    while (*str != '\0')  //连续写入字符串数据，直到检测到结束符
    {
        Lcd1602WriteDat(*str++);
    }
}

/* 初始化1602液晶 */
void InitLcd1602()
{
    Lcd1602WriteCmd(0x38);  //16*2显示，5*7点阵，8位数据接口
    Lcd1602WriteCmd(0x0C);  //显示器开，光标关闭
    Lcd1602WriteCmd(0x06);  //文字不动，地址自动+1
    Lcd1602WriteCmd(0x01);  //清屏
}

/* --------------------------------------------LCD1602-----------------------------------------------------------------*/
void init()
{
    // SCON = 0x50;
    // SCON = 0xd0; //串口工作于方式3
    SCON = 0xf0;
    PCON = 0x00;

    TMOD = 0x20;
    EA = 1;
    ES = 1; // 串口中断
    TL1 = 0xfd;
    TH1 = 0xfd;
    TR1 = 1;
    REN = 1;
}
void Sends(char *Buff)
{
    for (i = 0; i < strlen(Buff); i++)
    {
        TB8 = 0;
        SBUF = Buff[i];
        while (!TI)
            ;
        TI = 0;
    }
}
void main(void)
{
    init();
    InitADC();
    InitLcd1602();
    while (1)
    {

        SM2 = 1;
        address_status = 1;
        address_ok = 1;
        // led1 = 1;
        while (address_ok)
            ;
        SBUF = addr1;
        while (!TI)
            ;
        TI = 0;
        delay_1ms(250);
        SM2 = 0; //开始接收数据帧

        Adc_Action();
        refreshTemp(); //假设是实时的处理。
        // 发送数据

        Sends(smod_buf);
        Sends(temp_buf);
        Lcd1602ShowStr(0, 0, smod_buf);
        delay_1ms(100);
        Lcd1602ShowStr(0, 1, temp_buf);
        delay_1ms(400);
    }
}

// void serial() __interrupt 4 //串口中断
void serial() interrupt 4
{
    ES = 0;
    if (RI) //  收到数据
    {
        if (address_status == 1) //address frame
        {

            buf = SBUF;
            if (buf == addr1)
            {
                // led1=0;
                address_ok = 0;
                address_status = 0;
            }
        }
        RI = 0;
    }
    ES = 1;
}
