#include <mega8.h>
#include <delay.h>
#include <twim.c>
#include <Time_and_Clock.c>
#include <1-wire.c>
#include <DS18Bxx.c>

unsigned char Disp[16];      //  0     1     2     3     4     5     6     7     8     9     0.    1.    2.    3.    4.    5.    6.    7.    8.    9.    Пн    Вт    Ср    Чт    Пт    Сб    Нд    -    " "
flash unsigned char simv[29] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x40, 0x00};     //Оголошення масиву символів від 0 до 9 СC

unsigned char xscr;
unsigned char rt_sec = 10;
unsigned int TEMP;

void EEPROM_write (unsigned int uiAddress, unsigned char ucData)
{
while(EECR & (1<<EEWE));
EEAR = uiAddress;
EEDR = ucData;
EECR |= (1<<EEMWE);
EECR |= (1<<EEWE);
}

unsigned char EEPROM_read (unsigned int uiAddress)
{
while(EECR & (1<<EEWE));
EEAR = uiAddress;
EECR |= (1<<EERE);
return EEDR;
}

void Display_refr (void)
{
// Дисплей годинника
Disp[3] = System_time[1] % 10;
Disp[2] = System_time[1] / 10;
Disp[1] = System_time[0] % 10;
Disp[0] = System_time[0] / 10;
// Дисплей темератури
Disp[7] = Sys_Temp[3];
Disp[6] = Sys_Temp[2] + 10;
Disp[5] = Sys_Temp[1];
Disp[4] = Sys_Temp[0];

Disp[8] = System_date[3] + 19; //Дні неділі

//--- debug mode begin
Disp[9] = 28;    // space in debug first
Disp[10] = System_date[0] / 10;         //Dat Dec
Disp[11] = System_date[0] % 10 +10;     //Dat ED
Disp[12] = System_date[1] / 10;         //Mounth Dec
Disp[13] = System_date[1] % 10 +10;     //Mounth ED
Disp[14] = System_date[2] / 10;         //Year Dec
Disp[15] = System_date[2] % 10;         //Year ED
//--- debug mode end

if (0x00 == System_time[2] % 0x02)      // Блимач
    {
    Disp[1] = Disp[1] + 0x0A;
    }
}

void Get_sys_temp (void)
{
rt_sec++;
}

interrupt [TIM0_OVF] void timer0_ovf_isr(void)  // Оновлення дисплею 
{
TCNT0=0xA2; // 0xA2 - 3,008 ms
if (PORTC >=0x0F)   //Standart 0x08
    {
    PORTC = 0x00;
    xscr = 0x00;
    PORTD = simv[Disp[xscr]];
    }
else
    {
    PORTC++;
    xscr++;
    PORTD = simv[Disp[xscr]];
    }

}

interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
SysTime_incr();
Get_sys_temp();
Display_refr();

}

interrupt [TIM2_COMP] void timer2_comp_isr(void) // Оновлення дисплею
{

}

void main (void)
{
DDRB = 0b10001000;
DDRC = 0x0F;
DDRD = 0xFF; // Segments

//#asm("cli")
EEPROM_write (0x0001, 0x12);
EEPROM_write (0x0002, 0x7A);
//#asm("sei")

// Timer/Counter 2 initialization
ASSR=0x00;  //
TCCR2=0x0E; // 0x0E
TCNT2=0x00; //
OCR2=0x5D;  // 0x7D

// Timer/Counter 1 initialization
TCCR1A=0x00;
TCCR1B=0x0C;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=EEPROM_read (0x0002);  // 0x7A
OCR1AL=EEPROM_read (0x0001);  // 0x12
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 31,250 kHz
TCCR0=(1<<CS02) | (0<<CS01) | (0<<CS00);
TCNT0=0xA2;

// Global enable interrupts
#asm("sei")

TWI_MasterInit(100);
Get_RTC_time();
TIMSK=0x91; // був 90 

while(1)
    {
    if (rt_sec > 9)
        {
        Gov_No();
        rt_sec = 0x00;
        }

    if (PINB.1 == 0x00) // Button "K" -
        {
        My_SREG--;
        delay_ms(300);
        }
    if (PINB.2 == 0x00) // Button "L" +
        {
        My_SREG++;
        delay_ms(300);
        }
    if (PINB.4 == 0x00) // Button "M" Mute/set/option
        {
        if (My_SREG & 0x01)                 // Beep in Hour
            {
            Disp[9] = My_SREG;    
            }        

        }
    }
}