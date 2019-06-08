#include <8052.h>
#include <string.h>
// #include "Lcd1602.h"
// #include "config.h"
#define uchar unsigned char
#define uint unsigned int
volatile unsigned char sending;
#define BUFF_MAX 50
unsigned char Buff[BUFF_MAX]; //数据缓冲区
#define addr1 'a'
#define addr2 'b'
#define _SUCC_ 0x0f //数据传送成功
#define _ERR_ 0xf0  //数据传送失败
#define led P1_0
#define led1 P1_1
#define led2 P1_2
#define led3 P1_3
#define led4 P1_4
#define led5 P1_5
uchar buf;
// sbit led = P1 ^ 0;
// sbit led1 = P1 ^ 1;

uchar address_ok, flag;
uchar status, t0_num = 0, time_out;
uchar add_flag, address_repeat_flag = 0, rev_data_status, j = 0;
uchar smod_status, temp_status;
uchar i = 0;
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
    // SCON = 0xd0; //串口工作于方式3
    SCON = 0xf8;
    TL1 = 0xfd;
    TH1 = 0xfd;
    PCON = 0x00;
    ES = 1; // open serial interrupt
    TR1 = 1;
    REN = 1;
    EA = 1;
}

void Leasts_Sends(char *Buff)
{
    for (i = 0; i < strlen(Buff); i++)
    {
        TB8 = 0;
        if (Buff[i] != '$')
        {
            SBUF = Buff[i];
            while (!TI)
                ;
            TI = 0;
        }
    }
}

void Clear_Buf(void)
{
    unsigned char k;
    for (k = 0; k < BUFF_MAX; k++) //将缓存内容清零
    {
        Buff[k] = 0;
    }
}

void main()
{
    uartinit();
    address_repeat_flag = 0;
    while (1)
    {
        address_ok = 0;
        add_flag = 1;
        buf = 'a';
        rev_data_status = 0;
        while (add_flag) // 轮询地址
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

        TI = 0;  //发送数据长度
        TB8 = 0; //发送数据帧
        SBUF = 'c';
        while (!TI)
            ;
        TI = 0;

        rev_data_status = 1;
        smod_status = 1;
        temp_status = 1;
        flag = 0;
        delay_1ms(300);

        if (rev_data_status == 0)
        { //发送数据到上位
            led1 = 0;
            if (strstr(Buff, "1919ppm") != NULL)
                led3 = 0;
            if (strstr(Buff, "11.4C") != NULL)
                led4 = 0;
            Leasts_Sends(Buff);
            Clear_Buf();
        }
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
        if (rev_data_status == 1)
        {
            // buf = SBUF;
            // if (buf != '$')
            // {
            //     Buff[j] = buf;
            //     j++;
            // }
            Buff[i] = SBUF; //将接收到的字符串存到缓存中
            if (Buff[i] == '$')
            {
                flag++;
            }
            i++; //缓存指针向后移动
            if (flag == 2)
            {
                rev_data_status = 0;
                i = 0;
            }
        }

        RI = 0; // 清空中断
    }
    ES = 1;
}
