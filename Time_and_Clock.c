#pragma used+

#define seconds 2
#define minutes 1
#define hours   0

#define day     3
#define date    0
#define month   1
#define year    2

void SysTime_incr (void);                                       // ϳ������� ������ ���������� ����

unsigned char My_SREG = 0b00000101;                             // ��������� ������ ����������� ���������.
                                                                // bit 0: =1 beep_per_hour ON       | =0 beep_per_hour OFF
                                                                // bit 1: =1 hide_zerro_hour OFF    | =0 hide_zerro_hour ON
                                                                // bit 2: =1 sound_buttons ON       | =0 sound_buttons OFF

unsigned char System_time [3] = {0, 0, 0};                      // �����, �������, �������
unsigned char System_date [5] = {0, 0, 0, 1, 20};               // �����, ̳����, г� (�������), ���� ����, г� (����)
unsigned char I2C_Buff[9];
unsigned char Day_in_Mounth[13] = {0xFF, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};    // ����� ��� � ������
//unsigned int Day_in_Mounth = 0b0000101011010101;  ����� ��� � ����������� �����
//                                   0.       F       7     1
unsigned long beep_permit = 0b00000001101111111111111110000000; // ����� �� ���������� ������

void Get_RTC_time (void)                                                    // ��������� ����� � ���
{
I2C_Buff[0] = 0b11010000;
I2C_Buff[1] = 0x00;
TWI_SendData(I2C_Buff,2);

I2C_Buff[0] = 0b11010001;
TWI_SendData(I2C_Buff,8);
TWI_GetData(I2C_Buff,8);
#asm("cli")
System_time[seconds] = ((0x0F & I2C_Buff[1]) + ((0x70 & I2C_Buff[1])>>4) * 10);     // �������
System_time[minutes] = ((0x0F & I2C_Buff[2]) + ((0x70 & I2C_Buff[2])>>4) * 10);     // �������
System_time[hours] = ((0x0F & I2C_Buff[3]) + ((0x70 & I2C_Buff[3])>>4) * 10);       // ������

System_date[date] = ((0x0F & I2C_Buff[5]) + ((0x70 & I2C_Buff[5])>>4) * 10);        // ���� �����
System_date[month] = ((0x0F & I2C_Buff[6]) + ((0x70 & I2C_Buff[6])>>4) * 10);       // ̳����
System_date[year] = ((0x0F & I2C_Buff[7]) + ((0x70 & I2C_Buff[7])>>4) * 10);        // г� (�������)
System_date[day] = I2C_Buff[4];                                                     // ���� ����

if (System_date[year] % 4)     // �������� �� ���������� ��
    {
    Day_in_Mounth[2] = 28;
    }
else
    {
    Day_in_Mounth[2] = 29;
    }
#asm("sei")
}

void Set_RTC_time (void)
{
//I2C_Buff[0] = 0b11010000;
//I2C_Buff[1] = 0x00;
//TWI_SendData(I2C_Buff,2);
#asm("cli")
I2C_Buff[2] = ((System_time[seconds] / 10) << 4) | (System_time[seconds] % 10);
I2C_Buff[3] = ((System_time[minutes] / 10) << 4) | (System_time[minutes] % 10);
I2C_Buff[4] = ((System_time[hours] / 10) << 4) | (System_time[hours] % 10);
I2C_Buff[5] =   System_date[day];
I2C_Buff[6] = ((System_date[date] / 10) << 4) | (System_date[date] % 10);
I2C_Buff[7] = ((System_date[month] / 10) << 4) | (System_date[month] % 10);
I2C_Buff[8] = ((System_date[year] / 10) << 4) | (System_date[year] % 10);
#asm("sei")
I2C_Buff[0] = 0b11010000;
I2C_Buff[1] = 0x00;
TWI_SendData(I2C_Buff,9);
}

void SysTime_incr (void)                        // ��������� ���
{
if (System_time[seconds] > 58)                        // �������
    {
    System_time[seconds] = 0x00;
    if (System_time[minutes] > 58)                    // �������
        {
        System_time[minutes] = 0x00;
        if (System_time[hours] > 22)                // ������
            {
            System_time[hours] = 0x00;

            if (System_date[day] > 6)             // ���� ����
                {
                System_date[day] = 0x01;
                }
            else
                {
                System_date[day]++;
                }

            if (System_date[date] >= Day_in_Mounth[System_date[month]])            // ���� (���� �����/����)
                {
                System_date[date] = 0x01;

                if(System_date[month] > 11)                                     // ̳����
                    {
                    System_date[month] = 0x01;

                    if(System_date[year] > 98)                                 // г�
                        {
                        System_date[year] = 0x00;
                        }
                    else
                        {
                        System_date[year]++;
                        }
                    }
                else
                    {
                    System_date[month]++;
                    }
                }
            else
                {
                System_date[date]++;
                }
            }
        else
            {
            System_time[hours]++;
            }
        if (My_SREG & 0x01)   // Beep per Hour
            {
            if ((beep_permit >> System_time[hours]) & 0x01)
                {
                beep_sound(200,3800);
                }
            }
        }
    else
        {
        System_time[minutes]++;
        }
    }
else
    {
    System_time[seconds]++;
    }
}
#pragma used-
