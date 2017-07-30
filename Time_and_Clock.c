#pragma used+
void SysTime_incr (void);                                       // ϳ������� ������ ���������� ����

unsigned char My_SREG = 0x01;                                   // ��������� ������ ����������� ���������.
unsigned char System_time [3] = {0, 0, 0};                      // �����, �������, �������
unsigned char System_date [5] = {0, 0, 0, 1, 20};               // �����, ̳����, г� (�������), ���� ����, г� (����)
unsigned char I2C_Buff[9];
unsigned char Day_in_Mounth[13] = {0xFF, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};    // ����� ��� � ������
//unsigned int Day_in_Mounth = 0b0000101011010101;  ����� ��� � ����������� �����
//                                   0.       F       7     1
unsigned long beep_permit = 0b00000001101111111111111110000000; // �������

void Get_RTC_time (void)                                                    // ��������� ����� � ���
{
I2C_Buff[0] = 0b11010000;
I2C_Buff[1] = 0x00;
TWI_SendData(I2C_Buff,2);

I2C_Buff[0] = 0b11010001;
TWI_SendData(I2C_Buff,8);
TWI_GetData(I2C_Buff,8);
#asm("cli")
System_time[2] = ((0x0F & I2C_Buff[1]) + ((0x70 & I2C_Buff[1])>>4) * 10);   // �������
System_time[1] = ((0x0F & I2C_Buff[2]) + ((0x70 & I2C_Buff[2])>>4) * 10);   // �������
System_time[0] = ((0x0F & I2C_Buff[3]) + ((0x70 & I2C_Buff[3])>>4) * 10);   // ������

System_date[0] = ((0x0F & I2C_Buff[5]) + ((0x70 & I2C_Buff[5])>>4) * 10);   // ���� �����
System_date[1] = ((0x0F & I2C_Buff[6]) + ((0x70 & I2C_Buff[6])>>4) * 10);   // ̳����
System_date[2] = ((0x0F & I2C_Buff[7]) + ((0x70 & I2C_Buff[7])>>4) * 10);   // г� (�������)
System_date[3] = I2C_Buff[4];                                               // ���� ����

if (System_date[2] % 4)     // �������� �� ���������� ��
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
I2C_Buff[2] = ((System_time[2] / 10) << 4) | (System_time[2] % 10);
I2C_Buff[3] = ((System_time[1] / 10) << 4) | (System_time[1] % 10);
I2C_Buff[4] = ((System_time[0] / 10) << 4) | (System_time[0] % 10);
I2C_Buff[5] =   System_date[3];
I2C_Buff[6] = ((System_date[0] / 10) << 4) | (System_date[0] % 10);
I2C_Buff[7] = ((System_date[1] / 10) << 4) | (System_date[1] % 10);
I2C_Buff[8] = ((System_date[2] / 10) << 4) | (System_date[2] % 10);
#asm("sei")
I2C_Buff[0] = 0b11010000;
I2C_Buff[1] = 0x00;
TWI_SendData(I2C_Buff,9);
}

void SysTime_incr (void)                        // ��������� ���
{
if (System_time[2] > 58)                        // �������
    {
    System_time[2] = 0x00;
    if (System_time[1] > 58)                    // �������
        {
        System_time[1] = 0x00;
        if ((My_SREG && 0x01) & (beep_permit >> System_time[0]+1) & 0x01)   // Beep per Hour
            {
            beep_sound(200,3800);
            }
        if (System_time[0] > 22)                // ������
            {
            System_time[0] = 0x00;

            if (System_date[3] > 6)             // ���� ����
                {
                System_date[3] = 0x01;
                }
            else
                {
                System_date[3]++;
                }
            if (System_date[0] >= Day_in_Mounth[System_date[1]])            // ����
                {
                System_date[0] = 0x01;
                if(System_date[2] > 98)         // г�
                    {
                    System_date[2] = 0x00;
                    }
                else
                    {
                    System_date[2]++;
                    }
                }
            else
                {
                System_date[0]++;
                }
            }
        else
            {
            System_time[0]++;
            }
        }
    else
        {
        System_time[1]++;
        }
    }
else
    {
    System_time[2]++;
    }
}
#pragma used-