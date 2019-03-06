#define F_CPU 16000000UL
#include<avr/io.h>
#include<util/delay.h>
#include<math.h>
#include <stdlib.h>
#include <stdio.h>


#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
 
float thea_1;
float thea_2;
float r,r_1,r_2;
float v_r_new,v_l_new,v_orig;
float v_l_p,v_r_p;

 void adc_init()
{
    ADMUX = (1<<REFS0);
 

    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}
 

uint16_t adc_read(uint8_t ch)
{

    ch &= 0b00000111;  
    ADMUX = (ADMUX & 0xF8)|ch;     
 

    ADCSRA |= (1<<ADSC);
 
    
    while(ADCSRA & (1<<ADSC));
 
    return (ADC);
}
 

 void pwn_init()
 {
	TCCR1A|=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11); 
	TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS11)|(1<<CS10); 
    
	ICR1=4999; 
 
	DDRD|=(1<<PD5)|(1<<PD4); 
}





void UART_init(long USART_BAUDRATE)
{
	UCSRB |= (1 << RXEN) | (1 << TXEN);
	UCSRC |= (1 << URSEL) | (1 << UCSZ0) | (1 << UCSZ1);//8 bits
	UBRRL = BAUD_PRESCALE;		
	UBRRH = (BAUD_PRESCALE >> 8);
}

unsigned char UART_RxChar()
{
	while ((UCSRA & (1 << RXC)) == 0);
	return(UDR);			
}

void UART_TxChar(char ch)
{
	while (! (UCSRA & (1<<UDRE)));	
	UDR = ch ;
}

void UART_SendString(char *str)
{
	unsigned char j=0;
	
	while (str[j]!=0)		
	{
		UART_TxChar(str[j]);	
		j++;
	}
}

void data(float t)
{
	if(t < 0)
	{
		t *=-1;
	}
	int d=t*100;
	int l4 = d%10;
	int l3 = (d /10)%10;
	int l2 = (d /100)%10;
	int l1 = (d /1000)%10;
	UART_TxChar(l1+48);
	UART_TxChar(l2+48);
	UART_TxChar(46);
	UART_TxChar(l3+48);
	UART_TxChar(l4+48);
	//UART_SendString(l2)
	UART_TxChar('\n');
}	

void new_data(float t)
{
	if(t<0)
	{
		t *=-1;
	}
	
	int d=t*100;
	int l5 = d%10;
	int l4 = (d /10)%10;
	int l3 = (d /100)%10;
	int l2 = (d /1000)%10;
	int l1 = (d/10000)%10;
	UART_TxChar(l1+48);
	UART_TxChar(l2+48);
	UART_TxChar(l3+48);
	UART_TxChar(46);
	UART_TxChar(l4+48);
	UART_TxChar(l5+48);
	//UART_SendString(l2)
	UART_TxChar('\n');
}
void disp_thea(float thea_1,float thea_2)
{
	UART_SendString("t1\n");
	data(fabs(thea_1));
	UART_SendString("t2\n");
	data(fabs(thea_2));
	UART_SendString("r1\n");
	data(fabs(r_1));
	UART_SendString("r2\n");
	data(fabs(r_2));
	UART_SendString("r\n");
	data(fabs(r));
	_delay_ms(100);
}
void g(float thea_1,float thea_2)
{
	UART_SendString("t1\n");
	data(thea_1);
	UART_SendString("t2\n");
	data(thea_2);
}

void disp_velocity()
{
	UART_SendString("v l: ");
	data(fabs(v_l_new));
	UART_SendString("v r: ");
	data(fabs(v_r_new));
	UART_SendString("v  : ");
	data(fabs(v_orig));
}
void disp_velocity_percentage()
{
	//UART_SendString("v l: ");
	new_data(fabs(v_l_p));
	//UART_SendString("v r: ");
	//new_data(fabs(v_r_p));
	
}

void main()
{
    float deg;
    UART_init(9600);
	adc_init();
	pwn_init();
    while(1)
	{
	
	uint16_t a = adc_read(0);
	
    if(a<=512)
    {
        deg = 1.5707 - (0.003079*a);
        deg *=-1;
    }
	
    else if(a>512 && a<=1023)
    {
        deg = (0.003079*a) - 1.5707;
        
    }
	
	float b = (2.5/tan(deg));
    float c_1 = 2.5/(b+1.25);
	float c_2 = 2.5/(b-1.25);
	thea_1 = ((180/3.14)*atan(c_1));// angle in degree
	thea_2 = ((180/3.14)*atan(c_2));
	//g(thea_1,thea_2);
	OCR1A = (345+((445/180)*thea_1)); //90 - 0o // 90 --> 349
	OCR1B = (345+((445/180)*thea_2));
	r_1 = 2.5/c_1;
	r_2 = 2.5/c_2;
	r = r_2+1.25;
	float t_1 = fabs(thea_1);
	float t_2 = fabs(thea_2);
	disp_thea(t_1,t_2);
	//disp_radius();
	float w_spec = 10;//rad/sec
	float v_left = w_spec*r_2;
	float v_right = w_spec*r_1;
	v_orig = w_spec*r;
	float dif = (v_right-v_left);
	v_l_new = v_orig + dif/2;
	v_r_new = v_orig - dif/2;
	v_l_p = 100*v_l_new/v_orig;
	v_r_p = 100*v_r_new/v_orig;
	//disp_velocity();
	//disp_velocity_percentage();
	}
}