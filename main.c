#include <8052.h>

#define uchar unsigned char
#define uint unsigned int
volatile unsigned char sending;
unsigned char Buff[20]; //数据缓冲区
#define addr1 0x01
#define addr2 0x02
#define _SUCC_ 0x0f //数据传送成功
#define _ERR_ 0xf0  //数据传送失败
#define led P1_0
#define led1 P1_1
uchar buf;
// sbit led = P1 ^ 0;
// sbit led1 = P1 ^ 1;

uchar address_ok;
uchar status, t0_num = 0, time_out, add_flag, address_repeat_flag = 0;


void delay_1ms(unsigned int i)
{
    unsigned int x, y;
    for (x = i; x > 0; x--)
        for (y = 110; y > 0; y--)
            ;
}

void uartinit(void) //串口初始化
{
    EA = 0;

    TMOD = 0x21;
    // SCON = 0x50;
    SCON = 0xd0; //串口工作于方式3
    TL1 = 0xfd;
    TH1 = 0xfd;
    PCON = 0x00;
    ES = 1; // open serial interrupt
    TR1 = 1;
    REN = 1;
    EA = 1;

    // //定时器
    // ET0 = 1; //开定时器中断
    // TH0 = (65536 - 50000) / 256;
    // TL0 = (65536 - 50000) % 256;
}

void main()
{
    uartinit();
    address_repeat_flag = 0;
    while (1)
    {
        address_ok = 0;
        add_flag = 1;
        buf = 0xff;

        while (add_flag)
        {
            if (address_repeat_flag == 0)
            {
                TB8 = 1;
                SBUF = addr1;
                while (!TI)
                    ;
                TI = 0;
            }
            else
            {
                TB8 = 1;
                SBUF = addr2;
                while (!TI)
                    ;
                TI = 0;
            }

            delay_1ms(250);
            if (address_ok == 1)
            {
                add_flag = 0;
            }
            else
            {
                led = 1;
            }
            address_repeat_flag = ~address_repeat_flag;
        }
        led = 0;
    }
}
void uart() __interrupt 4 //串口中断
{
    ES = 0;
    if (RI) //  收到数据
    {
        if (TB8 == 1)
        {
            buf = SBUF;
            if (address_repeat_flag == 0)
            {
                if (buf == addr1)
                {
                    address_ok = 1;
                }
            }
            else
            {
                if (buf == addr2)
                {
                    address_ok = 1;
                }
            }
        }
        RI = 0; // 清空中断
    }
    ES = 1;
}
