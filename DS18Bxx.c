#pragma used+
#define MAX_TEMP_DEVICES 3
unsigned char ROM [MAX_TEMP_DEVICES][8];   // Addresses of found devices
unsigned char Sys_Temp[MAX_TEMP_DEVICES][4] = {{28,28,18,28},
                                {28,28,18,28},
                                {28,28,18,28}};

unsigned char dev_count;

unsigned char searchROM(unsigned char searchCmd, unsigned char maxDevices);
unsigned char Search_ROM ();
void Read_ROM (void);
void Match_ROM (unsigned ROM_number);
void Skip_ROM (void);
void Alarm_Search ();
void Convert_T (void);
void Write_Scratchpad ();
void Read_Scratchpad ();
void Copy_Scratchpad ();
void Recall_E ();

void Convert_Temperature();
void updateTemperatureData(void);
unsigned char Read_Power_Supply (void);
signed int Get_temperature (unsigned char index_device);

unsigned char calc_crc (unsigned char *mas)
{
unsigned char i,data,crc,fb,st_byt;
st_byt = 0x00;
crc = 0x00;
do
    {
    data = mas[st_byt];
    for (i=0;i<8;i++)           // счетчик битов в байте
        {
        fb =crc  ^ data;
        fb&=1;
        crc >>= 1;
        data >>= 1;
        if (fb == 1)crc^=0x8c;  // полином, мля
        }
    st_byt++;
    }
while (st_byt != 9);            // счетчик байтов в массиве
return crc;
}

void clearROM (void)
{
unsigned char a, b;

for (a = 0x00; a < MAX_TEMP_DEVICES; a++)
    {
    for (b = 0x00; b < 8; b++)
        {
        ROM[a][b] = 0x00;
        }
    }
}

unsigned char searchROM(unsigned char searchCmd, unsigned char maxDevices)
    {
    unsigned char foundDevices = 0;
    unsigned char rom[8];
    unsigned char lastDiscrepancy = 0;
    unsigned char lastDeviceFlag = 0;
    unsigned char discrepancyMarker, bitNumber, byteNumber, bitMask;
    unsigned char bitA, bitB, bitToWrite;
    unsigned char i;
    
    clearROM();

    while (!lastDeviceFlag && foundDevices < maxDevices)
        {
        if (!W1_Reset())
            {
            // Якщо шина пуста — завершити пошук
            break;
            }

        W1_Tx(searchCmd);  // Команда пошуку: 0xF0 або 0xEC

        discrepancyMarker = 0;
        bitNumber = 1;
        byteNumber = 0;
        bitMask = 1;

        for (i = 0; i < 8; i++)
            {
            rom[i] = 0;
            }

        // Перебираємо 64 біти ROM коду
        while (bitNumber <= 64)
            {
            bitA = W1_Rx(1);
            bitB = W1_Rx(1);

            if (bitA == 1 && bitB == 1)
                {
                // Немає відповіді на шині
                return foundDevices;
                }

            if (bitA == 0 && bitB == 0)
                {
                // Конфлікт — обидва біти 0
                if (bitNumber < lastDiscrepancy)
                    {
                    bitToWrite = (ROM[foundDevices - 1][byteNumber] & bitMask) ? 1 : 0;
                    }
                else 
                    {
                    if (bitNumber == lastDiscrepancy)
                        {
                        bitToWrite = 1;
                        } 
                    else
                        {
                        bitToWrite = 0;
                        }
                    }

                if (bitToWrite == 0)
                    {
                    discrepancyMarker = bitNumber;
                    }
                }
            else
                {
                bitToWrite = bitA;
                }

            // Записуємо обраний біт у ROM
            if (bitToWrite)
                {
                rom[byteNumber] |= bitMask;
                }
            else
                {
                rom[byteNumber] &= ~bitMask;
                }

            W1_Tx_bit(bitToWrite);  // Надсилаємо біт на шину

            // Переходимо до наступного біту
            bitNumber++;
            bitMask <<= 1;
            if (bitMask == 0)
                {
                bitMask = 1;
                byteNumber++;
                }
            }

        // Копіюємо ROM у глобальний масив
        for (i = 0; i < 8; i++)
            {
            ROM[foundDevices][i] = rom[i];
            }
        foundDevices++;
        lastDiscrepancy = discrepancyMarker;

        if (lastDiscrepancy == 0)
            {
            lastDeviceFlag = 1;
            }
        }

    return foundDevices;
    }

unsigned char Search_ROM ()
{
unsigned char rBit, count_bit = 0x00, count_byte = 0x00, dualbit = 0x00;
dev_count = 0x00;
clearROM();
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
unsigned char rom_byte;
W1_Tx(0x55);
for (rom_byte=0x00; rom_byte < 8; rom_byte++)
	{
	W1_Tx(ROM[ROM_number][rom_byte]);
	}
}

void Skip_ROM (void)                                // Пропустити ROM-код звернення до всіх датчиків що присутні на шині
{
W1_Tx(0xCC);
}

void Alarm_Search ()
{
//int *p;
W1_Tx(0xEC);

}

void Convert_T (void)
{
W1_Tx(0x44);
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

signed int Get_temperature (unsigned char index_device)
{
signed int temperature;
unsigned char scrp[9];
unsigned char scrp_num = 0x00;
unsigned char chekbyte = 0x00;

while(scrp_num < 9)
    {
    scrp[scrp_num] = W1_Rx(8);
    scrp_num++;
    }
temperature = scrp[1] << 8;
temperature |= 0x00FF & scrp[0];

chekbyte = calc_crc(scrp);
if (chekbyte)
    {
    temperature = -32768;
    }
return temperature;
}

void Convert_Temperature(void)
{
if (W1_Reset())
	{
	Skip_ROM();		// W1_Tx(0xCC);
	Convert_T();	// W1_Tx(0x44);
	}
}

void updateTemperatureData(void)
    {
    unsigned char nDevice, nByte;
    unsigned char scratchpadBuffer[9];
    signed int temperature;
    for (nDevice=0;nDevice<dev_count;nDevice++)
        {
        if (W1_Reset())
            {
            Match_ROM(nDevice);                             // W1_Tx(0x55);
		    Read_Scratchpad(); 		                        // W1_Tx(0xBE);
            for (nByte=0;nByte<9;nByte++)
                {
                scratchpadBuffer[nByte]=W1_Rx(8);           // Read Scratchpad from device
                }

            if(calc_crc(scratchpadBuffer) == 0)
                {
                temperature = scratchpadBuffer[0];
                temperature |= (signed int)scratchpadBuffer[1] << 8;
                
                if (temperature >= 0)
                    {
                    scratchpadBuffer[0] = 0x1C;
                    nByte = 0x000F & temperature;
                    temperature = temperature >> 4; 
                    }
                else
                    {
                    scratchpadBuffer[0] = 0x1B; // variable for hundreds (or sign)
                    temperature ^= 0xFFFF;
                    temperature++;
                    nByte = 0x000F & temperature;
                    temperature = temperature >> 4;
                    }
                
                if (temperature > 99)
                    {
                    Sys_Temp[nDevice][0] = temperature / 100;
                    Sys_Temp[nDevice][1] = (temperature - 100) / 10;
                    Sys_Temp[nDevice][2] = (temperature - 100) % 10;
                    }
                else
                    {
                    if (temperature > 9)
                        {
                        Sys_Temp[nDevice][0] = scratchpadBuffer[0];
                        Sys_Temp[nDevice][1] = temperature / 10;
                        Sys_Temp[nDevice][2] = temperature % 10;
                        }
                    else
                        {
                        Sys_Temp[nDevice][0] = 0x1C;
                        Sys_Temp[nDevice][1] = scratchpadBuffer[0];
                        Sys_Temp[nDevice][2] = temperature;
                        }
                    }
                Sys_Temp[nDevice][3] = (625 * nByte) / 1000;
                }
            else
                {
                // Crc Error
                }
            }
        else
            {
            Sys_Temp[nDevice][0] = 0x1C;
            Sys_Temp[nDevice][1] = 0x1C;
            Sys_Temp[nDevice][2] = 0x12;
            Sys_Temp[nDevice][3] = 0x1C;
            }
        }
    }

#pragma used-
