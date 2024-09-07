#include "msp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ADCInit(void);
float tempRead(void);
void UARTInit(void);
void TX(char text[]);
int RX(void);
void m(void);
void blink(int comb, int tt);
void rgb(void);
void button(void);
void temperature(void);
void t(void);

void ADCInit(void)
{
	
	REF_A ->CTL0 &= ~0x8; 
	REF_A ->CTL0 |= 0x30; 
	REF_A ->CTL0 &= ~0x01; 
	
	ADC14 ->CTL0 |= 0x10;
	ADC14 ->CTL0 &= ~0x02; 
	ADC14 ->CTL0 |=0x4180700; 
	ADC14 ->CTL1 &= ~0x1F0000; 
	ADC14 ->CTL1 |= 0x800000; 
	ADC14 ->MCTL[0] |= 0x100; 
	ADC14 ->MCTL[0] |= 22; 
	ADC14 ->CTL0 |=0x02; 
	return;
}

float tempRead(void)
{
	float temp; 
	uint32_t cal30 = TLV ->ADC14_REF2P5V_TS30C; 
	uint32_t cal85 = TLV ->ADC14_REF2P5V_TS85C; 
	float calDiff = cal85 - cal30; 
	ADC14 ->CTL0 |= 0x01; 
	while((ADC14 ->IFGR0) ==0)
	{
		
	}
	temp = ADC14 ->MEM[0]; 
	temp = (temp - cal30) * 55; 
	temp = (temp/calDiff) + 30; 
	return temp; 
}

void UARTInit(void)
{
	EUSCI_A0->CTLW0 |= 1;       
  EUSCI_A0->MCTLW = 0;        
  EUSCI_A0->CTLW0 = 0x0081;
	EUSCI_A0->BRW = 26;      
	P1->SEL0 |= 0x0C;           
  P1->SEL1 &= ~0x0C;	
  EUSCI_A0->CTLW0 &= ~1;  
	return;
}

void TX(char text[])
{
	int i =0;
	while(text[i] != '\0')
	{
		EUSCI_A0 ->TXBUF = text[i];
		while((EUSCI_A0 ->IFG & 0x02) == 0)
		{
	
		}
		i++;
	}
	return;
}

int RX(void)
{
	int i = 0;
	char command[2];
	char x;
	while(1)
	{
		if((EUSCI_A0 ->IFG & 0x01) !=0)
		{
			command[i] = EUSCI_A0 ->RXBUF;
			EUSCI_A0 ->TXBUF = command[i];
			while((EUSCI_A0 ->IFG & 0x02)==0);
			if(command[i] == '\r')
			{
				command[i] = '\0';
				break;
			}
			else
			{
			i++;
			}
		}
	}
	x = atoi(command);
	TX("\n\r");
	return x;
}

void m(void)
{
	TX("MSP432 Menu\n\r\n\r");
	TX("1. RGB Control\n\r");
	TX("2. Digital Input\n\r");
	TX("3. Temperature Reading\n\r");
	return;
}

//Option 1
void rgb(void){
	TX("Enter combination of RGB (1 - 7):");
	int comb = RX();
	if(comb > 7 || comb < 1)
	{
		comb = 7;
	}
	TX("Enter Toggle Time:");
	int tt = RX();
	TX("Enter Number of blinks:");
	int blinking = RX();
	TX("Blinking LED...\n\r");
	for(int i=0;i<blinking;i++)
	{
		blink(comb,tt);
	}
	TX("Done\n\r\0");
	return;
}

//Option 2
void button(void)
{
	int B1, B2;
	B1 = P1->IN & 0x02;
	B2 = P1->IN & 0x10;
	if(B1==0)
	{
		TX("Button 1 pressed.\n\r");
	}
	if(B2==0)
	{
		TX("Button 2 pressed.\n\r");
	}
	if(B1!=0 && B2!=0)
	{
		TX("No button pressed.\n\r");
	}
	TX("\n\r\0");
	return;
}

//Option 3
void temperature(void)
{
	TX("Enter Number of Temperature Reading (1-5):");
	int TReadings = RX();
	if(TReadings < 1 || TReadings > 5)
	{
		TReadings = 5;
	}
	char x[50];
	for(int i = 0; i<TReadings; i++)
	{
		float Celsius = tempRead();
		float Fahr = (Celsius*(9.0/5.0))+32; 
		sprintf(x,"Reading %d: %.2f C & %.2f F\n\r",i+1,Celsius,Fahr);
		TX(x);
		t();
	}
	TX("\n\r");
	ADCInit();
	return;
}

void t(void)
{
	TIMER32_1->CONTROL |= 0x43;
	TIMER32_1->LOAD = 3000000-1;
	TIMER32_1->CONTROL |= 0x80;
	while((TIMER32_1->RIS & 1) == 0);
	TIMER32_1->INTCLR = 0;
}

void blink(int comb, int tt)
{
	SysTick->LOAD = (tt*3000000)-1; 
	SysTick->CTRL |= 0x4;
	P2->OUT &= comb;
	SysTick->CTRL |=0x1;
	P2->OUT |= comb;
	while((SysTick->CTRL & 0x10000) == 0);
	SysTick->LOAD = (tt*3000000)-1;
	SysTick->CTRL |= 0x4;
	SysTick->CTRL |=0x1;
	P2->OUT &= ~comb;
	while((SysTick->CTRL & 0x10000) == 0);
	return;
}

int main(void)
{
	UARTInit();
	ADCInit();
	P2->SEL0 &= ~0x07; 
	P2->SEL1 &= ~0x07; 
	P2->DIR |= 0x07; 
	P1->DIR &= ~0x12; 
	P1->REN |=0x12;
	P1->OUT |=0x12; 
	while(1)
	{
		m();
		int option = RX();
		switch(option)
		{
			case 1:
				rgb();
			break;
			
			case 2:
				button();
			break;
			
			case 3:
				temperature();
			break;
			
			default:
				TX("Invalid Menu Option\n\r\n\r");
			break;
		}
	}
	while(1);
}
