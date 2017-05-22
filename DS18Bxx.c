#pragma used+
unsigned char ROM [9];
unsigned char Sys_Temp[2];

unsigned char Search_ROM ();
void Read_ROM (void);
void Match_ROM (unsigned ROM_number);
void Skip_ROM (void);
void Alarm_Search ();
void Convert_T (void);
unsigned int Write_Scratchpad (void);
void Read_Scratchpad ();
void Copy_Scratchpad ();
void Recall_E ();
unsigned char Read_Power_Supply (void);
unsigned int Get_Temperature (unsigned char dev_numb);

unsigned char Search_ROM ()
{
unsigned char dev_count = 0xDD;
return dev_count;
}

void Read_ROM (void)                                // Читання ROM-коду пристрою (у випадку якщо один на шині)
{
unsigned char x = 0x00;
W1_Tx(0x33);
while(x<8)
    {
    ROM[x] = W1_Rx(8);
    x++;
    }
}

void Match_ROM (unsigned ROM_number)                // Звернення до конктретного пристрою на шині
{
unsigned char x = 0x00;
W1_Tx(0x55);
while(x<8)
    {
    W1_Tx(ROM[x]);
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

unsigned int Write_Scratchpad (void)                // Зчитування 9 бітів EEPROM DS18B20+
{
unsigned char scrp [9];
unsigned char x = 0x00;
unsigned int z;
W1_Tx(0xBE);
while(x<8)
    {
    scrp[x] = W1_Rx(8);
    x++;
    }
z = (0x0F & scrp[0]);
z = z << 8;
x = (scrp[0] >> 4);
x |= (0xF0 & scrp[1] << 4);
z |= 0x00FF & x;
return z;
}

void Read_Scratchpad ()
{

}

void Copy_Scratchpad ()
{

}

void Recall_E ()
{

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
    tC = Write_Scratchpad();    
    }
else
    {

    }
return tC;
}

#pragma used-