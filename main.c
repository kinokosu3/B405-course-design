// #include <8052.h>
#include <reg52.h>
#include <string.h>
#define uchar unsigned char
#define uint unsigned int
volatile unsigned char sending;
#define BUFF_MAX 30
unsigned char Buff[BUFF_MAX]; //数据缓冲区
#define addr1 'a'
#define addr2 'b'
// #define led P1_0
// #define led1 P1_1
// #define led2 P1_2
// #define led3 P1_3
// #define led4 P1_4
// #define led5 P1_5

uchar buf;
sbit led = P1 ^ 0;
sbit led1 = P1 ^ 1;
sbit led2 = P1 ^ 2;
sbit led3 = P1 ^ 3;
sbit led4 = P1 ^ 4;
sbit led5 = P1 ^ 5;
sbit led6 = P1 ^ 6;
sbit bee = P3 ^ 6;
uchar address_ok, flag;
uchar status, t0_num = 0, time_out;
uchar add_flag, address_repeat_flag = 0, rev_data_status, j = 0;
uchar i = 0, address_error = 0;
uint warning_cout = 0;
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
    memset(Buff, 0, (BUFF_MAX * sizeof(Buff)));
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
        rev_data_status = 0;
        led3 = 1;

        warning_cout = 0;
        while (add_flag) // 轮询地址
        {
            led1 = 1;
            led2 = 1;
            if (address_repeat_flag != 1 && address_repeat_flag != 0)
            {
                address_error++;
                if (address_error == 1)
                {
                    address_repeat_flag = 0;
                }
                else if (address_error == 2)
                {
                    address_repeat_flag = 1;
                    address_error = 0;
                }
            }
            if (address_repeat_flag == 0)
            {
                led2 = 1;
                TB8 = 1;
                SBUF = addr1;
                while (!TI)
                    ;
                TI = 0;
            }
            else if (address_repeat_flag == 1)
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
            // address_repeat_flag = ~address_repeat_flag;
            if (address_repeat_flag == 0)
            {
                address_repeat_flag = 1;
            }
            else if (address_repeat_flag == 1)
            {
                address_repeat_flag = 0;
            }
        }
        led = 0;

        rev_data_status = 1;
        flag = 0;
        delay_1ms(500); //500 较稳

        //发送数据到上位
        // 报警

        // if (rev_data_status == 1){
        //     warning_cout = Buff[0] * 1000 + warning_cout;
        //     warning_cout = Buff[1] * 100 + warning_cout;
        //     warning_cout = Buff[2] * 10 + warning_cout;
        //     warning_cout = Buff[3] + warning_cout;
        //     if(warning_cout > 500){
        //         bee = 0;
        //         delay_1ms(250); //500 较稳
        //     }else{
        //         bee = 1;
        //         delay_1ms(250); //500 较稳
        //     }
        // }
        if (address_repeat_flag == 0)
        {

            led3 = 0;
            if (strstr(Buff, "ppm") != NULL)
                led4 = 0;
            if (strstr(Buff, "C") != NULL)
                led5 = 0;
            Leasts_Sends(Buff);
            Clear_Buf();
        }
    }
}

void uart() interrupt 4
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
                    led1 = 0;
                    address_ok = 1;
                }
            }
            else if (address_repeat_flag == 1)
            {
                if (buf == addr2)
                {
                    led2 = 0;
                    address_ok = 1;
                }
            }
        }
        if (rev_data_status == 1)
        {
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
