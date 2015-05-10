#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 7372800UL
#define BAUD 9600
#include <util/setbaud.h>
#include <util/delay.h>
#define BYTES 38
#define LBYTES 4 
#define LINES 15
#define COLS 20
#define DREFRESHMS 0
#define DNEXTMS 300
#define NEACH 2
#define FMSIZE 12
unsigned char cur[BYTES] = {0};
volatile unsigned char new[BYTES] = {0};
volatile int idx = 0;
unsigned char frame[FMSIZE] = {0};
static unsigned char usart_getc(void){
	//loop_until_bit_is_set(UCSR0A,RXC0);
	unsigned char c = UDR0;
	return c;
}
static void usart_init(void){
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	#if USE_2X
	UCSR0A |= (1 << U2X0);
	#else
	UCSR0A &= ~(1 << U2X0);
	#endif
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
	UCSR0C=(1<<UCSZ01)|(1<<UCSZ00);
}
static void io_init(void){
	DDRA = 0xFF;
	DDRB = 0xFF;
	DDRC = 0xFF;
	DDRF = 0xFF;
	DDRG = 0xFF;
}
static void set_cols(unsigned char *porta,unsigned char *portb,unsigned char *portc,unsigned char *portf,unsigned char *portg){
	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	PORTF = 0;
	PORTG = 0;
	PORTA = *porta;
	PORTB = *portb;
	PORTC = *portc;
	PORTF = *portf;
	PORTG = *portg;
	_delay_ms(DNEXTMS);
	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	PORTF = 0;
	PORTG = 0;
	*portf = 0;
	*portc = 0;
	*portg = 0;
}
static void calc_diff(unsigned char diff[],volatile unsigned char new[],unsigned char cur[],int line){
	int lsbit = line*COLS;
	int index = lsbit/8;
	int shift = lsbit%8;
	int j;
	unsigned char mask = 0xFF << shift;
	int commited = 8-shift;
	for(j=0;j<LBYTES;j++){
		diff[j] = new[index+j] ^ cur[index+j];
		unsigned char cur_tmp = cur[index+j];
		cur[index+j] &= ~mask;
		cur[index+j] |= mask & (cur_tmp ^ diff[j]);
		if(COLS-commited>=8){
			mask = 0xFF;
			commited += 8;
		}else if(COLS>commited){
			mask = 0xFF >> (8-(COLS-commited));
			commited = COLS;
		}else{
			mask = 0x00;
		}
	}
	for(j=0;j<LBYTES-1;j++)
		diff[j] = (unsigned char)((*(unsigned short*)(&diff[j]))>>shift);

}
int main(void){
	usart_init();
	io_init();
	sei();
	PORTA = 0;
	PORTB = 0;
	PORTC = 0;
	PORTF = 0;
	PORTG = 0;
	while(1){
		_delay_ms(DREFRESHMS);
		PORTE = ~PORTE;
		int i;
		for(i=0;i<LINES;i++){
			unsigned char porta = 0;
			unsigned char portb = 0;
			unsigned char portc = 0;
			unsigned char portf = 0;
			unsigned char portg = 0;
			//diff
			unsigned char diff[LBYTES] = {0};
			calc_diff(diff,new,cur,i);
			//io
			//line
			if(i<8){
				porta = 1<<i;
				portb = 0;
			}else{
				porta = 0;
				portb = 1<<(i-8);
			}
			//col
			unsigned char *col_port[3] = { &portc,&portf,&portg };
			unsigned char mask_end[3] = { 0,0,0x10 };
			int j,count=0;
			for(j=0;j<3;j++){
				unsigned char mask;
				for(mask=1;mask!=mask_end[j];mask<<=1){
					if(diff[j]&mask){
						*col_port[j] |= mask;
						count++;
						if(count==NEACH){
							set_cols(&porta,&portb,&portc,&portf,&portg);
							count = 0;
						}
					}
				}
			}
			if(count){
				set_cols(&porta,&portb,&portc,&portf,&portg);
				count = 0;
			}
		}
	}
}
int check_and_get(int *index,unsigned char *data){
	//"sta",[index],[index],[index],[data],[data],[data],"end"
	int good = 1;
	unsigned char *fptr = frame;
	//head
	good &= *fptr++ == (unsigned char)'s';
	good &= *fptr++ == (unsigned char)'t';
	good &= *fptr++ == (unsigned char)'a';
	//index
	unsigned char i;
	i = *fptr++;
	good &= *fptr++ == i;
	good &= *fptr++ == i;
	//data
	unsigned char d;
	d = *fptr++;
	good &= *fptr++ == d;
	good &= *fptr++ == d;
	//tail
	good &= *fptr++ == 'e';
	good &= *fptr++ == 'n';
	good &= *fptr++ == 'd';
	*index = (int)i;
	*data = d;
	return good;
}
ISR(USART0_RX_vect){
	unsigned char c = usart_getc();
	begin:
	if(idx==0&&c!='s')
		goto end;
	if(idx==1&&c!='t'){
		idx=0;
		goto begin;
	}
	if(idx==2&&c!='a'){
		idx=0;
		goto begin;
	}
	frame[idx] = c;
	if(++idx==FMSIZE){
		idx=0;
		int index;
		unsigned char data;
		if(check_and_get(&index,&data))
			new[index] = data;
	}
	end:;
}
