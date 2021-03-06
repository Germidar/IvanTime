#pragma used+
unsigned char ROM [3][8];   //  Address find device
unsigned char Sys_Temp[3][4] = {{28,28,18,28},
                                {28,28,18,28},
                                {28,28,18,28}};

unsigned char dev_count;

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
void Update_Temperature_data();
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
    for (i=0;i<8;i++)           // ������� ����� � �����
        {
        fb =crc  ^ data;
        fb&=1;
        crc >>= 1;
        data >>= 1;
        if (fb == 1)crc^=0x8c;  // �������, ���
        }
    st_byt++;
    }
while (st_byt != 9);            // ������� ������ � �������
return crc;
}

void clear_ROM (void)
{
unsigned char a, b;

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
dev_count = 0x00;
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
                // ������� �� ���� (������ ������� �� ������)
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

void Read_ROM (void)                                // ������� ROM-���� �������� (� ������� ���� ���� �� ����)
{
unsigned char x = 0x00;
W1_Tx(0x33);
while(x<8)
    {
    ROM[2][x] = W1_Rx(8);   // temp in debug
    x++;
    }
}

void Match_ROM (unsigned ROM_number)                // ��������� �� ������������ �������� �� ����
{
unsigned char rom_byte;
W1_Tx(0x55);
for (rom_byte=0x00; rom_byte < 8; rom_byte++)
	{
	W1_Tx(ROM[ROM_number][rom_byte]);
	}
}

void Skip_ROM (void)                                // ���������� ROM-��� ��������� �� ��� ������� �� �������� �� ����
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
unsigned char scrp [9];
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

void Update_Temperature_data(void)
{
unsigned char dev_num;                                  // dev_count - ʳ������ ��������� ��������
signed int vremtemp;
unsigned char T_integer, T_fraction;
unsigned char znak = 0x1C;

for (dev_num = 0x00; dev_num < dev_count; dev_num++)
    {
    if (W1_Reset())
        {
        Match_ROM(dev_num);                             // W1_Tx(0x55);
		Read_Scratchpad(); 		                        // W1_Tx(0xBE);
		vremtemp = Get_temperature(dev_num);	        // ������� 9 ����� � ������� �� ���������� �RC
        if (vremtemp == -32768)
            {

            }
        else
            {
            if(vremtemp < 0)
                {
                vremtemp = (vremtemp ^= 0xFFFF);
                vremtemp++;
                znak = 0x1B;
                }
            else
                {
                znak = 0x1C;
                }

            T_integer = vremtemp >> 4;
            T_fraction = 0x000F & vremtemp;

            if (T_integer > 99)
                {
                Sys_Temp[dev_num][0] = T_integer / 100;
                Sys_Temp[dev_num][1] = (T_integer - 100) / 10;
                Sys_Temp[dev_num][2] = (T_integer - 100) % 10;
                }
            else
                {
                if (T_integer > 9)
                    {
                    Sys_Temp[dev_num][0] = znak;
                    Sys_Temp[dev_num][1] = T_integer / 10;
                    Sys_Temp[dev_num][2] = T_integer % 10;
                    }
                else
                    {
                    Sys_Temp[dev_num][0] = 0x1C;
                    Sys_Temp[dev_num][1] = znak;
                    Sys_Temp[dev_num][2] = T_integer;
                    }
                }
            Sys_Temp[dev_num][3] = (T_fraction * 0.625);    // ��������� ������� �����������
            }
        }
    else
        {
        Sys_Temp[dev_num][0] = 0x1C;
        Sys_Temp[dev_num][1] = 0x1C;
        Sys_Temp[dev_num][2] = 0x12;
        Sys_Temp[dev_num][3] = 0x1C;
        }

    }

}

#pragma used-
