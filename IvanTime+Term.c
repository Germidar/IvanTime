#include <mega8.h>
#include <delay.h>
#include <twim.c>
#include <Time_and_Clock.c>
#include <1-wire.c>
#include <DS18Bxx.c>

unsigned char Disp[16];//  0     1     2     3     4     5     6     7     8     9     0.    1.    2.    3.    4.    5.    6.    7.    8.    9.    Пн    Вт    Ср    Чт    Пт    Сб    Нд    -
unsigned char simv[28] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x40};     //Оголошення масиву символів від 0 до 9 СC

unsigned char x;

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

// Timer1 output compare A interrupt service routine
interrupt [TIM1_COMPA] void timer1_compa_isr(void)
{
SysTime_incr();
// Дисплей годинника
Disp[3] = System_time[1] % 10;
Disp[2] = System_time[1] /10;
Disp[1] = System_time[0] % 10;
Disp[0] = System_time[0] / 10;
// Дисплей темератури
Disp[7] = System_time[2] % 10;
Disp[6] = System_time[2] / 10;
Disp[5] = Day_in_Mounth[2] % 10;
Disp[4] = Day_in_Mounth[2] / 10;

Disp[8] = System_date[3] + 19; //Дні неділі

//--- debug mode begin
Disp[9] = 125;    // zero in debug first
Disp[10] = System_date[0] / 10;     //Dat Dec
Disp[11] = System_date[0] % 10 +10; //Dat ED
Disp[12] = System_date[1] / 10;     //Mounth Dec
Disp[13] = System_date[1] % 10 +10; //Mounth ED
Disp[14] = System_date[2] / 10;     //Year Dec
Disp[15] = System_date[2] % 10;     //Year ED
//--- debug mode end

if (0x00 == System_time[2] % 0x02)     // Блимач
    {
    Disp[1] = Disp[1] + 0x0A;
    }
}

interrupt [TIM2_COMP] void timer2_comp_isr(void) // Оновлення дисплею
{
if (PORTC >=0x0F)   //Standart 0x08
    {
    PORTC = 0x00;
    x = 0x00;
    PORTD = simv[Disp[x]];
    }
else
    {
    PORTC++;
    x++;
    PORTD = simv[Disp[x]];
    }
}



void main (void)
{
DDRB = 0b10000000;
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
// Timer(s)/Counter(s) Interrupt(s) initialization
//TIMSK=0x90;

// Global enable interrupts
#asm("sei")

TWI_MasterInit(100);
Get_RTC_time();
TIMSK=0x90;

while(1)
    {
    if (PINB.4 == 0x00)
        {
        Set_RTC_time();
        delay_ms(200);
        }
    if (PINB.3 == 0x00)
        {
        TIMSK=0x80;
        }
    else
        {
        TIMSK=0x90;
        delay_ms(200);
        }
    }   
}