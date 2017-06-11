#include <mega8.h>
#include <delay.h>
#include <twim.c>
#include <timer_mode_manager.c>
#include <Time_and_Clock.c>
#include <1-wire.c>
#include <DS18Bxx.c>


unsigned char Disp[9];       //  0     1     2     3     4     5     6     7     8     9     0.    1.    2.    3.    4.    5.    6.    7.    8.    9.    ��    ��    ��    ��    ��    ��    ��    -    " "
flash unsigned char simv[29] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x40, 0x00};     //���������� ������ ������� �� 0 �� 9 �C


unsigned char Display_System_Status = 0x01;
unsigned char xscr;
unsigned char rt_sec = 10;
unsigned int button[3];
unsigned char button_hold[3];
unsigned int TEMP = 0x00;

void reset_system (void)
{
#asm("rjmp 0") //test_program_reset
}

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
switch (Display_System_Status)
    {
    case 0x01:
    // ������� ���������
    Disp[3] = System_time[1] % 10;          // ������� ������
    Disp[2] = System_time[1] / 10;          // ������� ������
    Disp[1] = System_time[0] % 10;          // ������� �����
    Disp[0] = System_time[0] / 10;          // ������� �����
    // ������� ����������
    Disp[7] = Sys_Temp[3];                  // ������� �������
    Disp[6] = Sys_Temp[2] + 10;             // �������
    Disp[5] = Sys_Temp[1];                  // �������
    Disp[4] = Sys_Temp[0];                  // �����

    Disp[8] = System_date[3] + 19; //��� ����

    //--- debug mode begin
//    Disp[9] = TEMP;    // space in debug first
//    Disp[10] = System_date[0] / 10;         //Dat Dec
//    Disp[11] = System_date[0] % 10 +10;     //Dat ED
//    Disp[12] = System_date[1] / 10;         //Mounth Dec
//    Disp[13] = System_date[1] % 10 +10;     //Mounth ED
//    Disp[14] = System_date[2] / 10;         //Year Dec
//    Disp[15] = System_date[2] % 10;         //Year ED
    //--- debug mode end

    if (0x00 == System_time[2] % 0x02)      // ������
        {
        Disp[1] = Disp[1] + 0x0A;
        }
    break;

    case 0x02:                              // ³���������� ����� ����� �� ���� �� ������
    // ������� ����
    Disp[3] = System_date[1] % 10;          //Mounth ED
    Disp[2] = System_date[1] / 10;          //Mounth Dec
    Disp[1] = System_date[0] % 10 + 10;     //Dat ED
    Disp[0] = System_date[0] / 10;          //Dat Dec
    // ������� ����
    Disp[7] = System_date[2] % 10;          //Year ED
    Disp[6] = System_date[2] / 10;          //Year Dec
    Disp[5] = System_date[4] % 10;          //Year Sot
    Disp[4] = System_date[4] / 10;          //Year Tis
    break;

    case 0x10:      // ����������� ������ (0 - 59)
    Disp[3] = System_time[1] % 10;          // ������� ������
    Disp[2] = System_time[1] / 10;          // ������� ������
    Disp[0] = 0x1C;
    Disp[1] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = 0x1C;
    Disp[6] = 0x1C;
    Disp[7] = 0x1C;
    Disp[8] = 0x1C;

    break;

    case 0x11:      // ����������� ����� (0 - 23*)
    Disp[1] = System_time[0] % 10;          // ������� �����
    Disp[0] = System_time[0] / 10;          // ������� �����
    Disp[2] = 0x1C;
    Disp[3] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = 0x1C;
    Disp[6] = 0x1C;
    Disp[7] = 0x1C;
    Disp[8] = 0x1C;
    break;

    case 0x12:      // ����������� ����� ����� (1 - 31*)
    Disp[1] = System_date[0] % 10 + 10;     //Dat ED
    Disp[0] = System_date[0] / 10;          //Dat Dec
    Disp[2] = 0x1C;
    Disp[3] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = 0x1C;
    Disp[6] = 0x1C;
    Disp[7] = 0x1C;
    Disp[8] = 0x1C;
    break;

    case 0x13:      // ����������� ������ ����� (1 - 12)
    Disp[3] = System_date[1] % 10;          //Mounth ED
    Disp[2] = System_date[1] / 10;          //Mounth Dec
    Disp[0] = 0x1C;
    Disp[1] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = 0x1C;
    Disp[6] = 0x1C;
    Disp[7] = 0x1C;
    Disp[8] = 0x1C;
    break;

    case 0x14:      // ����������� ������� ����
    Disp[7] = System_date[2] % 10;          //Year ED
    Disp[6] = System_date[2] / 10;          //Year Dec
    Disp[0] = 0x1C;
    Disp[1] = 0x1C;
    Disp[2] = 0x1C;
    Disp[3] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = 0x1C;
    Disp[8] = 0x1C;

    break;

    case 0x15:      // ����������� ������ ����
    Disp[5] = System_date[4] % 10;          //Year Sot
    Disp[4] = System_date[4] / 10;          //Year Tis
    Disp[0] = 0x1C;
    Disp[1] = 0x1C;
    Disp[2] = 0x1C;
    Disp[3] = 0x1C;
    Disp[6] = 0x1C;
    Disp[7] = 0x1C;
    Disp[8] = 0x1C;
    break;

    case 0x16:      // ������������ ��� ����
    Disp[8] = System_date[3] + 19; //��� ����
    Disp[0] = 0x1C;
    Disp[1] = 0x1C;
    Disp[2] = 0x1C;
    Disp[3] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = 0x1C;
    Disp[6] = 0x1C;
    Disp[7] = 0x1C;
    break;

    default:
    };
//auto_brightness(); //debug_PWM
}

void Get_sys_temp (void)
{
rt_sec++;
}

interrupt [TIM0_OVF] void timer0_ovf_isr(void)  // ��������� ������� + Sound multipler
{
TCNT0=0xA2; // 0xA2 - 3,008 ms
if (PORTC >=0x08)   //Standart 0x08; Debug 0x0F
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

if (beep > 1)
    {
    beep--;
    }
else
    {
    if (cfg_pwm & 0x01)
        {

        }
    else
        {
        cfg_pwm |= 0x01;        // bit0 = 1 "PWM_mode"
        auto_brightness();
        }
    }
}

interrupt [TIM1_COMPA] void timer1_compa_isr(void)  // 1000 ms
{
SysTime_incr();     // ������� ���� ���������
Get_sys_temp();     // ��������� ��� ���������� ������� �����������
Display_refr();     // ��������� ������ ����� ��� �������

}

void push_button_K (void)
{
switch (Display_System_Status)
    {
    case 0x01:

    break;

    case 0x02:      // ³���������� ����� ����� �� ���� �� ������

    break;

    case 0x10:      // ����������� ������ (0 - 59)
    if (System_time[1] <= 0x00)
        {
        System_time[1] = 59;
        }
    else
        {
        System_time[1]--;
        }
    break;

    case 0x11:      // ����������� ����� (0 - 23*)
    if (System_time[0] <= 0)
        {
        System_time[0] = 23;
        }
    else
        {
        System_time[0]--;
        }
    break;

    case 0x12:      // ����������� ����� ����� (1 - 31*)
    if (System_date[0] <= 1)
        {
        System_date[0] = Day_in_Mounth[System_date[1]];
        }
    else
        {
        System_date[0]--;
        }
    break;

    case 0x13:      // ����������� ������ ����� (1 - 12)
    if (System_date[1] <= 1)
        {
        System_date[1] = 12;
        }
    else
        {
        System_date[1]--;
        }
    break;

    case 0x14:      // ����������� ������� ����
    if (System_date[2] <= 0)
        {
        System_date[2] = 99;
        }
    else
        {
        System_date[2]--;
        }
    break;

    case 0x15:      // ����������� ������ ����
    if (System_date[4] <= 0)
        {
        System_date[4] = 99;
        }
    else
        {
        System_date[4]--;
        }
    break;

    case 0x16:      // ������������ ��� ����
    if (System_date[3] <= 1)
        {
        System_date[3] = 7;
        }
    else
        {
        System_date[3]--;
        }
    break;

    default:
    };
#asm("cli") //test
#asm("RCALL 6") //test
}

void push_button_L (void)
{
switch (Display_System_Status)
    {
    case 0x01:

    break;

    case 0x02:      // ³���������� ����� ����� �� ���� �� ������

    break;

    case 0x10:      // ����������� ������ (0 - 59)
    if (System_time[1] >= 59)
        {
        System_time[1] = 0;
        }
    else
        {
        System_time[1]++;
        }
    break;

    case 0x11:      // ����������� ����� (0 - 23*)
    if (System_time[0] >= 23)
        {
        System_time[0] = 0;
        }
    else
        {
        System_time[0]++;
        }
    break;

    case 0x12:      // ����������� ����� ����� (1 - 31*)
    if (System_date[0] >= Day_in_Mounth[System_date[1]])
        {
        System_date[0] = 1;
        }
    else
        {
        System_date[0]++;
        }
    break;

    case 0x13:      // ����������� ������ ����� (1 - 12)
    if (System_date[1] >= 12)
        {
        System_date[1] = 1;
        }
    else
        {
        System_date[1]++;
        }
    break;

    case 0x14:      // ����������� ������� ����
    if (System_date[2] >= 99)
        {
        System_date[2] = 0;
        }
    else
        {
        System_date[2]++;
        }
    break;

    case 0x15:      // ����������� ������ ����
    if (System_date[4] >= 99)
        {
        System_date[4] = 0;
        }
    else
        {
        System_date[4]++;
        }
    break;

    case 0x16:      // ������������ ��� ����
    if (System_date[3] >= 7)
        {
        System_date[3] = 1;
        }
    else
        {
        System_date[3]++;
        }
    break;

    default:
    };
#asm("cli") //test
#asm("RCALL 6") //test
}

void push_button_M (void)
{
// TEST begin
if (PINB.4 == 0x00)
    {
    button_hold[2]++;
    }
else
    {

    }
if ((Display_System_Status < 0x03) & button[2] > 0x00 & button_hold[2] > 0x00)
    {
    if (button_hold[2] < 0x05)
        {
        Display_System_Status = 0x02;
        }
    else
        {
        Display_System_Status = 0x10;
        }
    }
else
    {
    if (Display_System_Status >= 0x10 & button[2] > 0x00)
        {
        if (Display_System_Status == 0x16)
            {
            Set_RTC_time();
            Display_System_Status = 0x01;
            }
        else
            {
            Display_System_Status++;
            }
        }
    }
// TEST end
#asm("cli") //test
#asm("RCALL 6") //test
}

void manager_push_button (void)
{
if (PINB.1 == 0x00) // Button "K" -
    {
    button[0]++;
    }
else
    {
    button[0] = 0x00;
    }

if (PINB.2 == 0x00) // Button "L" +
    {
    button[1]++;
    }
else
    {
    button[1] = 0x00;
    }

if (PINB.4 == 0x00) // Button "M" Mute/set/option
    {
    button[2]++;
    }
else
    {
    button[2] = 0x00;
    button_hold[2] = 0x00;
    if (Display_System_Status < 0x03)
        {
        Display_System_Status = 0x01;
        }
    }

if (button[0] > 0x7FFF)     // ��������� �������� ��� �������� �������� �������� (������� ���������� ������.)
    {
    push_button_K();
    button[0] = 0x01;
    }
if (button[1] > 0x7FFF)
    {
    push_button_L();
    button[1] = 0x01;
    }
if (button[2] > 0x7FFF)
    {
    push_button_M();
    button[2] = 0x01;
    }
}

void main (void)
{
DDRB = 0b10101000;
PORTB = 0b00010110;     // �������� PullUP ���������
DDRC = 0x0F;
DDRD = 0xFF; // Segments

//#asm("cli")
//EEPROM_write (0x0001, 0x12);
//EEPROM_write (0x0002, 0x7A);
//#asm("sei")

// Timer/Counter 2 initialization
ASSR=0<<AS2;
TCCR2 = 0b11101010;
TCNT2=0x00;
OCR2=0xFF;       // ������ ���������

// Timer/Counter 1 initialization
TCCR1A=0x00;
TCCR1B=0x0C;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x7A;    //EEPROM_read (0x0002);  // 0x7A
OCR1AL=0x12;    //EEPROM_read (0x0001);  // 0x12
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
Display_refr();     // ��� ���������� ��������� ���� ��� ��������� ��������
TIMSK=0x11; // ��� - 91 ����� 11

while(1)
    {
    manager_push_button();
    if (rt_sec > 9)
        {
        Gov_No();
        rt_sec = 0x00;
        }

    }
}