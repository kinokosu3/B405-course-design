#include <8052.h>

#define uchar unsigned char
#define uint unsigned int
#define addr 0x02
#define _SUCC_ 0x0f //数据传送成功
#define _ERR_ 0xf0  //数据传送失败
#define led P1_0
#define led1 P1_1
// sbit led = P1 ^ 0;
// sbit led1 = P1 ^ 1;
uchar buf;
unsigned char Buff[20]; //数据缓冲区
uchar address_ok = 1;
uchar RECE_status;
uchar address_status;

void init()
{
    // SCON = 0x50;
    SCON = 0xd0; //串口工作于方式3
    PCON = 0x00;

    TMOD = 0x20;
    EA = 1;
    ES = 1; // 串口中断
    TL1 = 0xfd;
    TH1 = 0xfd;
    TR1 = 1;
    REN = 1;
}
void main(void)
{
    init();
    while (1)
    {
        
        SM2 = 1;
        address_status = 1;
        while (address_ok)
            continue;
        SBUF = addr;
        while (!TI)
            ;
        TI = 0;
        led=0;
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
            if (buf == addr)
            {
                address_ok = 0;
                address_status = 0;
            }
        }
        RI = 0;
    }
    ES = 1;
}