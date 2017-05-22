#pragma used+
unsigned char W1_Reset(void);
void W1_Tx_bit (unsigned char Tx_Bit);
unsigned char W1_Rx (unsigned char Nbit);
void W1_Tx (unsigned char Tx_Byte);

unsigned char W1_Reset (void)    // Reset 1-Wire BUS
{
unsigned char BUS_State;
DDRB.0 = 0xFF;
delay_us(480);
DDRB.0 = 0x00;
delay_us(60);
BUS_State =! PINB.0;
delay_us(420);
return BUS_State;
}

void W1_Tx_bit (unsigned char Tx_Bit)   // Write one BIT on 1-wire BUS
{
DDRB.0 = 0xFF;
delay_us(2);
if (Tx_Bit > 0)
    {
    DDRB.0 = 0x00;  // Transmit "1"
    delay_us(58);
    }
else
    {
    delay_us(58);   // Transmit "0"
    DDRB.0 = 0x00;
    }
delay_us(1);
}

unsigned char W1_Rx (unsigned char Nbit)     // Read 1-wire BUS data
{
unsigned char Rx_Bit, Rx_Byte = 0x00, i;
for (i=0x01;i<=Nbit;i++)
    {
    DDRB.0 = 0xFF;
    delay_us(2);
    DDRB.0 = 0x00;
    delay_us(12);   // CHANGE to uS
    Rx_Bit = PINB.0;
    Rx_Byte |= (Rx_Bit << (i-1));
    delay_us(47);
    }
return Rx_Byte;
}

void W1_Tx (unsigned char Tx_Byte) // Write Byte on 1-wire BUS
{
unsigned char w, WTMP;
for (w=0;w<=7;w++)
    {
    WTMP = (0x01 & Tx_Byte);
    W1_Tx_bit(WTMP);
    Tx_Byte = Tx_Byte >> 1;
    }
}
#pragma used-