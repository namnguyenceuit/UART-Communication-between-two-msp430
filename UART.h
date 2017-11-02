#ifndef UART_H_
#define UART_H_

void CLOCK_Configure();
void UART_Init();
void UART_Write_Char(unsigned char data);
void UART_Write_String(char* string);
void UART_Write_Int(unsigned long n);
void UART_Write_Float(float x,unsigned char coma);

//1Mhz clock configure
void CLOCK_Configure()
{
	if(CALBC1_1MHZ == 0xFF)
	        while(1);
	DCOCTL = 0;
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;
	BCSCTL2 = SELM_0 | DIVM_0;
}

//Init UART
void UART_Init()
{
	CLOCK_Configure();
	//Setup RXD, TXD (P1SEL & P2SEL Table 16, Page 43 MSP430G2553 Datasheet)
	//Positive PxSEL for every PORT
	P1SEL = BIT1 + BIT2;	//P1.1 = RXD, P1.2 = TXD
	P1SEL2 = BIT1 + BIT2;	//P1.1 = RXD, P1.2 = TXD

	UCA0CTL1 |= UCSWRST; //USCI Software reset, place UCA0 in reset state to be configure

	//Configure
	UCA0CTL1 |= UCSSEL_2; //Use SMCLK

	//clockConfigure 1MHZ selected
	//select baudrate 9600, datasheet MSP430x2xx Family, table 15.5
	//UCBRx = 6, UCBRSx = 0, UCBRFx = 8, UCOS16 = 1
	//UCBRx include two 8bit registers: UCAxBR0(low) & UCAxBR1(high)
	UCA0BR0 = 6; //6dec in 8bit-range bin
	UCA0BR1 = 0;

	//Modulation control respectively with the table
	UCA0MCTL = UCBRS_0 + UCBRF_8 + UCOS16;

	UCA0CTL1 &= ~UCSWRST; //Take UCA0 out of reset state

	IE2 |= UCA0RXIE;	//Enable interrupt when receive(Rx)
	_BIS_SR(GIE);	//Global enable interrupt
}

void UART_Write_Char(unsigned char data)
{
	// Send a single byte using the hardware UART 0
	// Example usage: UARTSendChar('a');

	while (!(IFG2 & UCA0TXIFG)); // Wait for TX buffer to be ready for new data
	UCA0TXBUF = data; //Write the character at the location specified by the pointer and increment pointer
}

void UART_Write_String(char* string)
{
	// Send number of bytes Specified in ArrayLength in the array at using the hardware UART 0
	// Example usage: UARTSendArray("Hello");
	// int data[2]={1023, 235};
	// UARTSendArray(data, 4); // Note because the UART transmits bytes it is necessary to send two bytes for each integer hence the data length is twice the array length
	while(*string) // Het chuoi ky tu thi thoat
		{
		while (!(IFG2&UCA0TXIFG)); // Wait for TX buffer to be ready for new data
		UCA0TXBUF= *string; //Write the character at the location specified by the pointer and increment pointer
		string ++; // get the next character
		}
	//UART_Write_Char(0);
}

void UART_Write_Int(unsigned long n)
{
     unsigned char buffer[16]; //(16) unsigned long ccs
     unsigned char i,j;

     if(n == 0) {
    	 UART_Write_Char('0');
          return;
     }

     for (i = 15; i > 0 && n > 0; i--) {
          buffer[i] = (n % 10) + '0'; // to turn n%10 to %c
          n /= 10;
     }

     for(j = i+1; j <= 15; j++) {
    	 UART_Write_Char(buffer[j]);
     }
}

void UART_Write_Float(float number, unsigned char coma)
{
	unsigned long temp;

	if(number < 0)
	{
		UART_Write_Char('-');			//print negative number
		number *= -1;
	}
	temp = (unsigned long) number;		//convert to integer

	UART_Write_Int(temp);

	number = ((float)number - temp);

	if(coma != 0)
		UART_Write_Char('.');	// print "."
	while(coma > 0)
	{
		number *= 10;
		coma--;
	}
	//temp = (unsigned long)(number + 0.5);	//round
	UART_Write_Int(number);
}

// Echo back RXed character, confirm TX buffer is ready first
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_IRS (void)
{
    while(!(IFG2 & UCA0RXIFG));
    if (UCA0RXBUF == 'A')			//got "ACTIVE" string
        TA1CTL = TASSEL_1 + ID_3 + MC_1;
    if (UCA0RXBUF == 'S')			//got "STOP" string
        TA1CTL = MC_0;
}

#endif	/* UART_H_ */
