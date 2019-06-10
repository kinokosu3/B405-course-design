// #include <8052.h>
#include <reg52.h>
#include <intrins.h>
//#include <STC12C5A.h>
#include <string.h>
// #include <ds18b20.h>
// #include <AD.h>
//#include <Time.h>
#define uchar unsigned char
#define uint unsigned int
#define addr1 'b'
#define _SUCC_ 0x0f //数据传送成功
#define _ERR_ 0xf0  //数据传送失败

// #define led P1_0
// #define led1 P1_1
// #define led2 P1_2
// sbit led = P1 ^ 0;
// sbit led1 = P1 ^ 1;
// sbit led2 = P1 ^ 2;
// sbit led3 = P1 ^ 3;
// sbit led4 = P1 ^ 4;
uchar buf;
// unsigned char Buff[50]; //数据缓冲区
// uchar DATA[13] = {"hello world$"};
uchar smod_buf[15];
uchar ch4_buf[15];
uchar address_ok;
uchar RECE_status;
uchar address_status, command_status, i;

/*---------------------------------------------------*/
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
/*---------------------------------------------------*/
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
    unsigned int adcy = 1;
    float smodValue;
    float CH4Value;

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
    

    ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START; //配置ADC_CONTR寄存器 并启动AD转换
    adcy = DigitalFiltering(1);
    ADC_CONTR = 1;
    CH4Value = (float)adcy * (4.8 / 1024);
    CH4Value = (CH4Value - 2) * 2000; //0.4为测量空气的中甲烷浓度值时的数值
    if (CH4Value <= 0)
        CH4Value = 0;
    adcy = CH4Value;
    ch4_buf[0] = adcy % 10000 / 1000 + 0x30;
    ch4_buf[1] = adcy % 1000 / 100 + 0x30;
    ch4_buf[2] = adcy % 100 / 10 + 0x30;
    ch4_buf[3] = adcy % 10 + 0x30;
    ch4_buf[4] = 'p';
    ch4_buf[5] = 'p';
    ch4_buf[6] = 'm';
    ch4_buf[7] = '$';
    
}


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

        SM2 = 0; //开始接收数据帧

        Adc_Action();
        //refreshTemp();
        // 发送数据
        
        // led2 = 0;
        Sends(smod_buf);
        delay_1ms(100);
        Sends(ch4_buf);
        // //delay_1ms(100);
        delay_1ms(2700); //2000 标志量
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
