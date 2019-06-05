#include <8052.h>
#include <string.h>
#define uchar unsigned char
#define uint unsigned int
#define addr1 0x01
#define _SUCC_ 0x0f //数据传送成功
#define _ERR_ 0xf0  //数据传送失败
#define led P1_0
#define led1 P1_1
#define led2 P1_2
// sbit led = P1 ^ 0;
// sbit led1 = P1 ^ 1;
uchar buf;
unsigned char Buff[50]; //数据缓冲区
// uchar DATA[13] = {"hello world$"};
uchar test_smod[] = {"1919ppm$"};
uchar test_temp[] = {"11.4C$"};
uchar address_ok;
uchar RECE_status;
uchar address_status, command_status, i;

void delay_1ms(unsigned int i)
{
    unsigned int x, y;
    for (x = i; x > 0; x--)
        for (y = 110; y > 0; y--)
            ;
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
    while (1)
    {

        SM2 = 1;
        address_status = 1;
        command_status = 0;
        address_ok = 1;
        led = 1;
        while (address_ok)
            ;
        SBUF = addr1;
        while (!TI)
            ;
        TI = 0;

        SM2 = 0; //开始接收数据帧
        command_status = 1;
        delay_1ms(250);
        // 发送数据
        if (buf == 0xff)
        {
            led1 = 0;
            Sends(test_smod);
            // //delay_1ms(100);
            Sends(test_temp);
            // //delay_1ms(100);
        }

        // if(buf == 0xff){
        //     led1=0;
        //     for ( i = 0; i < strlen(DATA); i++)
        //     {
        //         TB8=0;
        //         SBUF = DATA[i];
        //         while(!TI);
        //         TI=0;
        //     }
        // }
    }
}

void serial() __interrupt 4 //串口中断
{
    ES = 0;
    if (RI) //  收到数据
    {
        if (address_status == 1) //address frame
        {

            buf = SBUF;
            if (buf == addr1)
            {
                led = 0;
                address_ok = 0;
                address_status = 0;
            }
        }
        if (command_status == 1)
        {
            buf = SBUF;
            command_status = 0;
        }
        RI = 0;
    }
    ES = 1;
}