#pragma used+
void SysTime_incr (void);                                       // Підтримка роботи Системного часу

unsigned char System_time [3] = {0, 0, 0};                      // Годии, хвилини, секунди
unsigned char System_date [4] = {0, 0, 0, 1};                   // Число, Місяць, Рік (десятки), День неділі
unsigned char I2C_Buff[9];
unsigned char Day_in_Mounth[13] = {0xFF, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};    // Массив днів у місяцях

void Get_RTC_time (void)                                                         // Отримання часту з ГРЧ
{
I2C_Buff[0] = 0b11010000;
I2C_Buff[1] = 0x00;
TWI_SendData(I2C_Buff,2);

I2C_Buff[0] = 0b11010001;
TWI_SendData(I2C_Buff,8);
TWI_GetData(I2C_Buff,8);
System_time[2] = ((0x0F & I2C_Buff[1]) + ((0x70 & I2C_Buff[1])>>4) * 10);   // Секунди
System_time[1] = ((0x0F & I2C_Buff[2]) + ((0x70 & I2C_Buff[2])>>4) * 10);   // Хвилини
System_time[0] = ((0x0F & I2C_Buff[3]) + ((0x70 & I2C_Buff[3])>>4) * 10);   // Години

System_date[0] = ((0x0F & I2C_Buff[5]) + ((0x70 & I2C_Buff[5])>>4) * 10);   // День місяця
System_date[1] = ((0x0F & I2C_Buff[6]) + ((0x70 & I2C_Buff[6])>>4) * 10);   // Місяць
System_date[2] = ((0x0F & I2C_Buff[7]) + ((0x70 & I2C_Buff[7])>>4) * 10);   // Рік (десятки)
System_date[3] = I2C_Buff[4];                                               // День неділі

if (System_date[2] % 4)     // Перевірка на високосний рік
    {
    Day_in_Mounth[2] = 28;
    }
else
    {
    Day_in_Mounth[2] = 29;
    }
}

void Set_RTC_time (void)
{
I2C_Buff[0] = 0b11010000;
I2C_Buff[1] = 0x00;
TWI_SendData(I2C_Buff,2);
#asm("cli")

I2C_Buff[1] = ((System_time[2] / 10) << 4) | (System_time[2] % 10);
I2C_Buff[2] = ((System_time[1] / 10) << 4) | (System_time[1] % 10);
I2C_Buff[3] = ((System_time[0] / 10) << 4) | (System_time[0] % 10);
I2C_Buff[4] = System_date[3];
I2C_Buff[5] = ((System_date[0] / 10) << 4) | (System_date[0] % 10);
I2C_Buff[6] = ((System_date[1] / 10) << 4) | (System_date[1] % 10);
I2C_Buff[7] = ((System_date[2] / 10) << 4) | (System_date[2] % 10);
#asm("sei")
I2C_Buff[0] = 0b11010000;
TWI_SendData(I2C_Buff,8);
}

void SysTime_incr (void)                        // Системний час
{
if (System_time[2] > 58)                        // Секунди
    {
    System_time[2] = 0x00;
    if (System_time[1] > 58)                    // Хвилини
        {
        System_time[1] = 0x00;
        if (System_time[0] > 22)                // Години
            {
            System_time[0] = 0x00;
            if (System_date[3] > 6)             // День неділі
                {
                System_date[3] = 0x01;
                }
            else
                {
                System_date[3]++;
                }
            if (System_date[0] >= Day_in_Mounth[System_date[1]])           // Доба
                {
                System_date[0] = 0x01;
                if(System_date[2] > 98)         // Рік
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