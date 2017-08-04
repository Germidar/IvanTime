#pragma used+
unsigned char cfg_pwm = 0x81;   // регістр конфігурації режиму таймера bit 0: =1 PWM | =0 SQW
unsigned char adctmp;
unsigned int beep = 0x00; // 33 Час для звуку.
void beep_sound (unsigned int Time_in_ms, unsigned long frequency);

void set_brightness (unsigned char brightness)
{
if (cfg_pwm & 0x01)
    {
    TCCR2 = 0b11101010;     //(1<<PWM2) | (0<<COM21) | (1<<COM20) | (1<<CTC2) | (0<<CS22) | (1<<CS21) | (0<<CS20);
    OCR2 = brightness;
    PORTB.5 = 0xFF;
    }
}

void auto_brightness (void)
{
if (cfg_pwm & 0x80)
    {
    ADMUX = 0b00100111;
    ADCSRA = 0b10100101;
    delay_ms(1);
    ADCSRA = 0b11000101;
    delay_ms(1);
    adctmp = ADCH;
    set_brightness(adctmp);
    }
else
    {
    if (cfg_pwm & 0x01)
        {
        TCCR2 = 0x00;
        PORTB.5 = 0xFF;
        }
    }
}

void beep_sound (unsigned int Time_in_ms, unsigned long frequency)
{
beep = Time_in_ms / 2;      // Визначення часу звучання біпера
cfg_pwm &= 0xFE;                // bit0 = 0 "Sound_mode"
TCCR2=(0<<PWM2) | (0<<COM21) | (1<<COM20) | (1<<CTC2) | (0<<CS22) | (1<<CS21) | (0<<CS20);
OCR2 = (1000000 / (frequency * 2)) - 1;               //
PORTB.5 = 0x00;
}

#pragma used-