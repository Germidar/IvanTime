#include <mega8.h>
#include <delay.h>
#include <twim.c>
#include <timer_mode_manager.c>
#include <Time_and_Clock.c>
#include <1-wire.c>
#include <DS18Bxx.c>


unsigned char Disp[10];      //  0     1     2     3     4     5     6     7     8     9     0.    1.    2.    3.    4.    5.    6.    7.    8.    9.    ��    ��    ��    ��    ��    ��    ��    -    " "
flash unsigned char simv[29] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x40, 0x00};     //���������� ������ ������� �� 0 �� 9 �C
//flash unsigned char simv[29] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90, 0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10, 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0xBF, 0xFF};     //���������� ������ ������� �� 0 �� 9 �C

unsigned char Display_System_Status = 0x01;
unsigned char xscr;
unsigned char bttn;                     // ��� ������ ��������� ������� ������
unsigned char p_bttn = 0x00;            // ����� ���������� ������
unsigned char button_pushed[3];
unsigned char button_hold[3];
unsigned char rt_sec = 0x08;            // ��� ��� ���������� ��������� �����������
unsigned char refr_temp_dev = 0x09;     // ����� ��������� ������� �����������
unsigned char timeToChangeDev = 0x05;   // ��� ����������� � ������ ������� �� �����
unsigned char buttn_M_hold;             // ��� ����������� ������ M ��� �������� �� ������ ������������
unsigned char display_refresh = 0xFF;   // ����� ��������� �������
unsigned char curr_dev = 0x00;          // ����� ������� ����������� ��� �����������
unsigned char conv_need = 0xFF;         // ��������� ����������� ��������� ����������� ����������� ���������
unsigned char TMP = 0x00;
unsigned int  TEMP = 0x00;

void save_settings_to_eeprom ();
void load_settings_from_eeprom ();
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
    Disp[3] = System_time[minutes] % 10;        // ������� ������
    Disp[2] = System_time[minutes] / 10;        // ������� ������
    Disp[1] = System_time[hours] % 10;          // ������� �����
    if (My_SREG & 0x02)                         // ³�������� ������� �����, ���� ������ 10 �����.
        {
        Disp[0] = System_time[hours] / 10;      // ������� �����
        }
    else
        {
        if (System_time[hours] < 10)
            {
            Disp[0] = 28;
            }
        else
            {
            Disp[0] = System_time[hours] / 10;
            }
        }
    // ������� ����������
    Disp[7] = Sys_Temp[curr_dev][3];            // ������� �������
    Disp[6] = Sys_Temp[curr_dev][2] + 10;       // �������
    Disp[5] = Sys_Temp[curr_dev][1];            // �������
    Disp[4] = Sys_Temp[curr_dev][0];            // ����

    Disp[8] = System_date[day] + 19;            // �� �����
    Disp[9] = 21 + curr_dev;                    // ����� ������� ��� �����������
    if (System_time[seconds] % 0x02)            // ������
        {
        Disp[1] = Disp[1] + 0x0A;
        }
    break;

    case 0x02:                                  // ³���������� ����� ����� �� ���� �� ������
    // ������� ����
    Disp[3] = System_date[month] % 10;          //Mounth ED
    Disp[2] = System_date[month] / 10;          //Mounth Dec
    Disp[1] = System_date[date] % 10 + 10;      //Dat ED
    Disp[0] = System_date[date] / 10;           //Dat Dec
    // ������� ����
    Disp[7] = System_date[year] % 10;           //Year ED
    Disp[6] = System_date[year] / 10;           //Year Dec
    Disp[5] = System_date[hYear] % 10;          //Year Sot
    Disp[4] = System_date[hYear] / 10;          //Year Tis
    // ��������� ��� �����
    Disp[8] = System_date[day] + 19;            //�� �����;
    Disp[9] = 0x1C;
    break;

    case 0x10:      // ����������� ����� (0 - 23*)
    Disp[1] = System_time[hours] % 10;          // ������� �����
    if (My_SREG & 0x02)                         // ³�������� ������� �����, ���� ������ 10 �����.
        {
        Disp[0] = System_time[hours] / 10;      // ������� �����
        }
    else
        {
        if (System_time[hours] < 10)
            {
            Disp[0] = 28;
            }
        else
            {
            Disp[0] = System_time[hours] / 10;
            }
        }
    Disp[2] = 0x1C;
    Disp[3] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = System_time[seconds] / 10;
    Disp[6] = System_time[seconds] % 10;
    Disp[7] = 0x1C;
    Disp[8] = 0x15;
    Disp[9] = 0x1C;
        
    break;

    case 0x11:      // ����������� ������ (0 - 59)
    Disp[3] = System_time[minutes] % 10;        // ������� ������
    Disp[2] = System_time[minutes] / 10;        // ������� ������
    Disp[0] = 0x1C;
    Disp[1] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = System_time[seconds] / 10;
    Disp[6] = System_time[seconds] % 10;
    Disp[7] = 0x1C;
    Disp[8] = 0x16;
    Disp[9] = 0x1C;
    break;

    case 0x12:      // ����������� ����� ����� (1 - 31*)
    Disp[1] = System_date[date] % 10 + 10;     //Dat ED
    Disp[0] = System_date[date] / 10;          //Dat Dec
    Disp[2] = 0x1C;
    Disp[3] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = 0x1C;
    Disp[6] = 0x1C;
    Disp[7] = 0x1C;
    Disp[8] = 0x17;
    Disp[9] = 0x1C;
    break;

    case 0x13:      // ����������� ������ ����� (1 - 12)
    Disp[3] = System_date[month] % 10;          //Mounth ED
    Disp[2] = System_date[month] / 10;          //Mounth Dec
    Disp[0] = 0x1C;
    Disp[1] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = 0x1C;
    Disp[6] = 0x1C;
    Disp[7] = 0x1C;
    Disp[8] = 0x18;
    Disp[9] = 0x1C;
    break;

    case 0x14:      // ����������� ����
    Disp[7] = System_date[year] % 10;           //Year ED
    Disp[6] = System_date[year] / 10;           //Year Dec
    Disp[5] = System_date[hYear] % 10;          //Year Sot
    Disp[4] = System_date[hYear] / 10;          //Year Tis
    Disp[0] = 0x1C;
    Disp[1] = 0x1C;
    Disp[2] = 0x1C;
    Disp[3] = 0x1C;
    Disp[8] = 0x19;
    Disp[9] = 0x1C;
    break;

    case 0x15:      // ������������ ��� �����
    Disp[8] = System_date[day] + 19; //�� �����
    Disp[0] = 0x1C;
    Disp[1] = 0x1C;
    Disp[2] = 0x1C;
    Disp[3] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = 0x1C;
    Disp[6] = 0x1C;
    Disp[7] = 0x1C;
    Disp[9] = 0x1C;
    break;

    case 0x16:

    break;

    case 0x17:     // Edit beep hour sigal
    Disp[0] = 0x1C;
    if (My_SREG & 0x02)                     // ³�������� ������� �����, ���� ������ 10 �����.
        {
        Disp[1] = TMP / 10;  // ������� �����
        }
    else
        {
        if (TMP < 10)
            {
            Disp[1] = 28;
            }
        else
            {
            Disp[1] = TMP / 10;
            }
        }
    Disp[2] = TMP % 10;
    Disp[3] = 0x1C;
    Disp[4] = 0x1C;
    Disp[5] = 0x1C;
    Disp[6] = 0x1C;
    Disp[7] = 0x1C;
    Disp[8] = 0x1C;
    if ((beep_permit >> TMP) & 0x01)
        {
        Disp[9] = 0x03;
        }
    else
        {
        Disp[9] = 0x1C;
        }
    break;

    default:
    };
//auto_brightness(); //debug_PWM
}

void SysTemp_incr (void)
{
rt_sec++;
//timeToDisplayDevice++;
}

interrupt [TIM0_OVF] void timer0_ovf_isr(void)  // ��������� ������� + Sound multipler
{
TCNT0=0xC1; // 0xC1 = 2 ms
if (PORTC >=0x09)   //Standart 0x08; 0�09 - device identifer; Debug 0x0F
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
        auto_brightness();      // ���������� ����� �� 2 ��! ������������ ��������.
        }
    }
}

interrupt [TIM1_COMPA] void timer1_compa_isr(void)  // 1000 ms
{
SysTime_incr();     // ������� ���� ���������
SysTemp_incr();     // ��������� ��� ���������� ������� �����������
Display_refr();     // ��������� ������ ����� ��� �������

}

void push_button_K (void)
{
switch (Display_System_Status)
    {
    case 0x01:
    if (curr_dev > 0x00)
        {
        curr_dev--;
        }
    break;

    case 0x02:      // ³���������� ����� ����� �� ���� �� ������

    break;

    case 0x10:      // ����������� ����� (0 - 23*)
    if (System_time[hours] <= 0)
        {
        System_time[hours] = 23;
        }
    else
        {
        System_time[hours]--;
        }
    break;

    case 0x11:      // ����������� ������ (0 - 59)
    if (System_time[minutes] <= 0x00)
        {
        System_time[minutes] = 59;
        }
    else
        {
        System_time[minutes]--;
        }
    break;

    case 0x12:      // ����������� ����� ����� (1 - 31*)
    if (System_date[date] <= 1)
        {
        System_date[date] = Day_in_Mounth[System_date[month]];
        }
    else
        {
        System_date[date]--;
        }
    break;

    case 0x13:      // ����������� ������ ����� (1 - 12)
    if (System_date[month] <= 1)
        {
        System_date[month] = 12;
        }
    else
        {
        System_date[month]--;
        }
    break;

    case 0x14:      // ����������� ����
    if (System_date[year] <= 0)
        {
        System_date[year] = 99;
        if (System_date[hYear] <= 0)
            {
            System_date[hYear] = 99;
            }
        else
            {
            System_date[hYear]--;
            }
        }
    else
        {
        System_date[year]--;
        }
    break;

    case 0x15:      // ������������ ��� �����
    if (System_date[day] <= 1)
        {
        System_date[day] = 7;
        }
    else
        {
        System_date[day]--;
        }
    break;

    case 0x16:

    break;

    case 0x17:      // ������������ ����������� �������
    if (TMP > 0)
        {
        TMP--;
        }
    else
        {
        TMP = 23;
        }
    break;

    default:
    };
Display_refr();
}

void push_button_L (void)
{
switch (Display_System_Status)
    {
    case 0x01:
    if (curr_dev < 0x01) // dev_count
        {
        curr_dev++;
        }
    break;

    case 0x02:      // ³���������� ����� ����� �� ���� �� ������

    break;

    case 0x10:      // ����������� ����� (0 - 23*)
    if (System_time[hours] >= 23)
        {
        System_time[hours] = 0;
        }
    else
        {
        System_time[hours]++;
        }
    break;

    case 0x11:      // ����������� ������ (0 - 59)
    if (System_time[minutes] >= 59)
        {
        System_time[minutes] = 0;
        }
    else
        {
        System_time[minutes]++;
        }
    break;

    case 0x12:      // ����������� ����� ����� (1 - 31*)
    if (System_date[date] >= Day_in_Mounth[System_date[month]])
        {
        System_date[date] = 1;
        }
    else
        {
        System_date[date]++;
        }
    break;

    case 0x13:      // ����������� ������ ����� (1 - 12)
    if (System_date[month] >= 12)
        {
        System_date[month] = 1;
        }
    else
        {
        System_date[month]++;
        }
    break;

    case 0x14:      // ����������� ����
    if (System_date[year] >= 99)
        {
        System_date[year] = 0;
        if (System_date[hYear] >= 99)
            {
            System_date[hYear] = 0;
            }
        else
            {
            System_date[hYear]++;
            }
        }
    else
        {
        System_date[year]++;
        }
    break;

    case 0x15:      // ������������ ��� �����
    if (System_date[day] >= 7)
        {
        System_date[day] = 1;
        }
    else
        {
        System_date[day]++;
        }
    break;

    case 0x16:

    break;

    case 0x17:      // ������������ ����������� �������
    if (TMP < 23)
        {
        TMP++;
        }
    else
        {
        TMP = 0x00;
        }
    break;

    default:
    };
Display_refr();
}

void push_button_M (void)
{
switch (Display_System_Status)
    {
    case 0x01:
    Display_System_Status = 0x02;
    break;

    case 0x02:
    if (button_pushed[2] > 0xFE)
        {
        Display_System_Status = 0x10;
        beep_sound(800,3800);  // ������ ��� ������ ������������
        button_pushed[2] = 0x00;
        button_hold[2] = 0x00;
        buttn_M_hold = 0x00;
        }
    break;

    case 0x15:
    Display_System_Status = 0x10;
    break;

    case 0x17:
    if (buttn_M_hold > 0x01)
        {
        save_settings_to_eeprom();
        Display_System_Status = 0x10;
        button_pushed[2] = 0x00;
        button_hold[2] = 0x00;
        buttn_M_hold = 0x00;
        }
    else
        {
        if (TMP < 16)
            {
            beep_permit ^= 1 << TMP;
            }
        else
            {
            TEMP = beep_permit;
            beep_permit = (beep_permit >> 16);
            TMP -= 16;
            beep_permit ^= 1 << TMP;
            TMP += 16;
            //delay_ms(500);
            beep_permit = (beep_permit << 16);
            beep_permit += TEMP;
            }
        button_pushed[2] = 0x00;
        button_hold[2] = 0x00;
        buttn_M_hold = 0x00;
        }
    break;

    default:
    if (Display_System_Status >= 0x10)
        {
        if (buttn_M_hold > 0x09)
            {
            beep_sound(800,3800);  // ������ ������ � ������ ������������
            Set_RTC_time();
            Display_System_Status = 0x01;
            button_pushed[2] = 0x00;
            button_hold[2] = 0x00;
            buttn_M_hold = 0x00;
            }
        else
            {
            Display_System_Status++;
            button_pushed[2] = 0x00;
            button_hold[2] = 0x00;
            buttn_M_hold = 0x00;
            }
        }
    break;
    }
Display_refr();
}

void button_manager (void)  // ����� �������� ������
{
bttn = (0x16 & PINB);       // ���������� ������
delay_ms(0x03);     // 3ms zbs � ����. ��������.
switch (bttn)
    {
    case 0x16:  // ����� ������ �� ���������
    bttn = (0x16 & PINB);
    if (bttn == 0x16)
        {
        p_bttn = 0x00;
        button_pushed[0] = 0x00;
        button_pushed[1] = 0x00;
        button_pushed[2] = 0x00;
        button_hold[0] = 0x00;
        button_hold[1] = 0x00;
        button_hold[2] = 0x00;
        buttn_M_hold = 0x00;
        if (Display_System_Status == 0x02)
            {
            Display_System_Status = 0x01;
            Display_refr();
            }
        if (Display_System_Status < 0x10)
            {
            Display_System_Status = 0x01;
            }
        }
    break;

    case 0x14:  // ��������� ������ K "-"
    if (bttn == 0x14)
        {
        if (p_bttn == 0x01) // ������ ���� ��������� �����?
            {
            if (button_hold[0] > 0xD0)
                {
                button_pushed[0]++;
                if (button_pushed[0] > 0x40)
                    {
                    push_button_K();
                    button_pushed[0] = 0x00;
                    }
                }
            else
                {
                button_hold[0]++;
                }
            }
        else
            {
            if (My_SREG & 0x04)
                {
                beep_sound(10,3800);
                }
            push_button_K();    // ³����������� ���������� ������
            }
        p_bttn = 0x01;          // ���������� ���� ���������� ������
        }
    break;

    case 0x12:  // ��������� ������ L "+"
    if (bttn == 0x12)
        {
        if (p_bttn == 0x02)
            {
            if (button_hold[1] > 0xD0)
                {
                button_pushed[1]++;
                if (button_pushed[1] > 0x40)
                    {
                    push_button_L();
                    button_pushed[1] = 0x00;
                    }
                }
            else
                {
                button_hold[1]++;
                }
            }
        else
            {
            if (My_SREG & 0x04)
                {
                beep_sound(10,3800);
                }
            push_button_L();
            }
        p_bttn = 0x02;
        }
    break;

    case 0x06:  // ��������� ������ M "Mute/set/option"
    if (bttn == 0x06)
        {
        if (p_bttn == 0x04) // ������ ���� ��������� �����?
            {
            if (button_hold[2] > 0xFE)  // ��� ����������� ����/����
                {
                if (buttn_M_hold > 0x09)
                    {
                    button_pushed[2]++;
                    push_button_M();
                    }
                else
                    {
                    buttn_M_hold++;
                    button_hold[2] = 0x00;
                    }
                }
            else
                {
                button_hold[2]++;
                }
            }
        else
            {
            if (My_SREG & 0x04)
                {
                beep_sound(10,3800);
                }
            push_button_M();
            }
        p_bttn = 0x04;
        }
    break;

    case 0x10:  // �������� ������ KL "-" "+"
    if (bttn == 0x10)
        {
        if(Display_System_Status == 0x10 | Display_System_Status == 0x11)   // Seconds reset
            {
            System_time[seconds] = 0x00;
            TCNT1 = 0x0000;
            Display_refr();
            }
        }
    break;

    case 0x04:  // �������� ������ KM "-" "Mute/set/option"
    if (bttn == 0x04)
        {
        if (Display_System_Status > 0x10)   // ������� � ����� ����������� ����������� �������
            {
            Display_System_Status = 0x17;
            }
        }
    break;

    case 0x02:  // �������� ������ LM "+" "Mute/set/option"
    if (bttn == 0x02)
        {

        }
    break;

    case 0x00:  // �������� �� ������ KLM "-" "+" "Mute/set/option"
    if (bttn == 0x00)
        {

        }
    break;
    }
}

void load_settings_from_eeprom (void)
{
My_SREG =           EEPROM_read(0x0010);
cfg_pwm =           EEPROM_read(0x0011);
buttn_M_hold =      EEPROM_read(0x0012);
refr_temp_dev =     EEPROM_read(0x0013);
//var_1 =           EEPROM_read(0x0014);
//var_2 =           EEPROM_read(0x0015);
//abval =           EEPROM_read(0x0016);
display_refresh =   EEPROM_read(0x0017);
beep_permit = EEPROM_read(0x001B);
beep_permit = beep_permit << 8;
TMP = EEPROM_read(0x001A);
beep_permit += TMP;
TMP = EEPROM_read(0x0019);
beep_permit = beep_permit << 8;
beep_permit += TMP;
TMP = EEPROM_read(0x0018);
beep_permit = beep_permit << 8;
beep_permit += TMP;
TMP = 0x00;
}

void save_settings_to_eeprom (void)
{
unsigned char dta = 0xFF;
EEPROM_write(0x0010,My_SREG);
EEPROM_write(0x0011,cfg_pwm);
EEPROM_write(0x0012,buttn_M_hold);
EEPROM_write(0x0013,refr_temp_dev);
//EEPROM_write(0x0014,var);
//EEPROM_write(0x0015,var);
//EEPROM_write(0x0016,abval);
EEPROM_write(0x0017,display_refresh);

dta = beep_permit;
EEPROM_write(0x0018,dta);
dta = beep_permit >> 8;
EEPROM_write(0x0019,dta);
dta = beep_permit >> 16;
EEPROM_write(0x001A,dta);
dta = beep_permit >> 24;
EEPROM_write(0x001B,dta);
}

void main (void)
{
DDRB = 0b10101000;
PORTB = 0b00010110;     // ������� PullUP ���������
DDRC = 0x0F;
DDRD = 0xFF; // Segments

//Timer/Counter 2 initialization
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

//Timer/Counter 0 initialization
//Clock source: System Clock
//Clock value: 31,250 kHz
TCCR0=(1<<CS02) | (0<<CS01) | (0<<CS00);
TCNT0=0xFF;

// Global enable interrupts
#asm("sei")

if(PINB.4)
    {
    load_settings_from_eeprom();    // ������������ ������������ �������� � EEPROM
    }
TWI_MasterInit(100);
Get_RTC_time();
Display_refr();     // ��� ���������� ��������� ���� ��� �������� ��������
dev_count = Search_ROM();
TIMSK=0x11; // ��� - 91 ���� 11


while(1)
    {
    button_manager();

    if (Display_System_Status < 10)
        {
        if (rt_sec > refr_temp_dev)
        	{
            Update_Temperature_data();  // ������� ����������� ������� � ��������
            rt_sec = 0x00;		        // chiki-piki need 0x01
            conv_need = 0xFF;
        	Display_refr();
            }
        else
        	{
        	if (conv_need && (rt_sec >= refr_temp_dev))   // + ��������� ������� ������������� �����������.
                {
        		Convert_Temperature();                  // Convert T
                conv_need = 0x00;
        		}
        	}
		}
    }
}
