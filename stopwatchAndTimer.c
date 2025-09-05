unsigned char tH, sH;
unsigned char tM, sM;
unsigned char tS, sS;
unsigned int sD;
bit isTimerRunning, isStopwatchRunning, finishTimer = 0;

void timerDecrement (void);
void stopwatchIncrement (void);

void timerDecrement (void)
    {
    if (tS>0)
        {
        tS--;
        }
    else
        {
        if (tM>0)
            {
            tM--;
            tS=59;
            }
        else
            {
            if (tH>0)
                {
                tH--;
                tM=59;
                tS=59;
                }
            else
                {
                finishTimer = 1;
                isTimerRunning = 0;
                }
            }
        }
    }
    
void stopwatchIncrement (void)
    {
    if (sS<59)
        {
        sS++;
        }
    else
        {
        if (sM<59)
            {
            sM++;
            sS=0;
            }
        else
            {
            if (sH<23)
                {
                sH++;
                sM=0;
                sS=0;
                }
            else
                {
                sD++;
                sS=0;
                sM=0;
                sH=0;
                }
            }
        }
    }