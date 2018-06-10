#pragma used+
unsigned char ROM [3][8];
signed int Sys_tmp;
unsigned char Sys_Temp[4] = {28,28,18,28};
float temperature;

unsigned char Search_ROM ();
void Read_ROM (void);
void Match_ROM (unsigned ROM_number);
void Skip_ROM (void);
void Alarm_Search ();
void Convert_T (void);
signed int Pidarastiya (void);
void Write_Scratchpad ();
void Read_Scratchpad ();
void Copy_Scratchpad ();
void Recall_E ();
unsigned char Read_Power_Supply (void);
unsigned int Get_Temperature (unsigned char dev_numb);

void clear_ROM (void)
{
unsigned char a, b, c;

for (a = 0x00; a < 3; a++)
    {
    for (b = 0x00; b < 8; b++)
        {
        ROM[a][b] = 0x00;
        }
    }
}

unsigned char Search_ROM ()
{
unsigned char rBit, count_bit = 0x00, count_byte = 0x00, dualbit = 0x00;
unsigned char dev_count = 0x00;
clear_ROM();
newpoisk:
if (W1_Reset())
    {
    W1_Tx(0xF0);
    for (count_byte = 0x00; count_byte < 8; count_byte++)
        {
        for (count_bit = 0x00; count_bit < 8; count_bit++)
            {
            rBit = W1_Rx(2);

            switch (rBit)
                {
                case 0x00:
                dualbit = (count_byte * 8) + count_bit;
                if (dev_count == 0x00)
                    {
                    ROM[dev_count][count_byte] |= 0x00 << count_bit;
                    W1_Tx_bit(0x00);
                    }
                else
                    {
                    ROM[dev_count][count_byte] |= 0x01 << count_bit;
                    W1_Tx_bit(0x01);
                    }
                break;

                case 0x01:
                ROM[dev_count][count_byte] |= 0x01 << count_bit;
                W1_Tx_bit(0x01);
                break;

                case 0x02:
                ROM[dev_count][count_byte] |= 0x00 << count_bit;
                W1_Tx_bit(0x00);
                break;

                case 0x03:
                // Помилка на шині (жодний пристрій не відповів)
                dev_count = 0xFF;
                count_byte = 0x00;
                count_bit = 0x00;
                break;

                default:

                }
            }
        }
    if (dev_count != 0xFF)
        {
        dev_count++;
        }

    if ((dualbit > 0x00) & (dev_count == 0x01))
        {
        goto newpoisk;
        }
    }
else
    {
    dev_count = 0x00;   // No Device
    }
return dev_count;
}

void Read_ROM (void)                                // Читання ROM-коду пристрою (у випадку якщо один на шині)
{
unsigned char x = 0x00;
W1_Tx(0x33);
while(x<8)
    {
    ROM[2][x] = W1_Rx(8);   // temp in debug
    x++;
    }
}

void Match_ROM (unsigned ROM_number)                // Звернення до конктретного пристрою на шині
{
unsigned char x = 0x00;
W1_Tx(0x55);
while(x<8)
    {
    W1_Tx(ROM[ROM_number][x]);
    x++;
    }
}

void Skip_ROM (void)                                // Пропустити ROM-код звернення до всіх датчиків що присутні на шині
{
W1_Tx(0xCC);
}

void Alarm_Search ()
{

}

void Convert_T (void)
{
W1_Tx(0x44);
}

signed int Pidarastiya (void)                // Зчитування 9 бітів EEPROM DS18B20+
{
unsigned char scrp [9];
unsigned char x = 0x00;
signed int z;
W1_Tx(0xBE);
while(x<8)
    {
    scrp[x] = W1_Rx(8);
    x++;
    }
z = scrp[1] << 8;
z |= 0x00FF & scrp[0];

return z;
}

void Write_Scratchpad ()
{
W1_Tx(0x4E);
}

void Read_Scratchpad ()
{
W1_Tx(0xBE);
}

void Copy_Scratchpad ()
{
W1_Tx(0x48);
}

void Recall_E ()
{
W1_Tx(0xB8);
}

unsigned char Read_Power_Supply (void)
{
unsigned char x;
W1_Tx(0xB4);
x = W1_Rx(0x01);
return x;
}

unsigned int Get_Temperature (unsigned char dev_numb)
{
unsigned int tC;
if (W1_Reset() == 0x01)
    {
    Skip_ROM();
    Convert_T();
    delay_ms(810);
    W1_Reset();
    Skip_ROM();
    tC = Pidarastiya();
    }
else
    {

    }
return tC;
}

void Gov_No (void)
{
signed int i_tmp;
Sys_tmp = Get_Temperature(1);
i_tmp = Sys_tmp >> 4;
temperature = i_tmp + (Sys_tmp & 0x000F) * 0.0625;

if (temperature < 0)
    {
    Sys_tmp ^= 0xFFFF;
    Sys_tmp++;
    i_tmp = Sys_tmp >> 4;
    temperature = i_tmp + (Sys_tmp & 0x000F) * 0.0625;
    Sys_Temp[0] = 27;
    goto plus;
    }
else
    {
    if (i_tmp > 99)
        {
        Sys_Temp[0] = i_tmp / 100;
        Sys_Temp[1] = (i_tmp - 100) / 10;
        Sys_Temp[2] = (i_tmp - 100) % 10;
        }
    else
        {
        Sys_Temp[0] = 28;
        plus:
        Sys_Temp[1] = i_tmp / 10;
        if (Sys_Temp[1] < 1)
            {
            Sys_Temp[1] = Sys_Temp[0];
            Sys_Temp[0] = 28;
            }
        Sys_Temp[2] = i_tmp % 10;
        }
    Sys_Temp[3] = (temperature - i_tmp) * 10;
    }
}

#pragma used-
