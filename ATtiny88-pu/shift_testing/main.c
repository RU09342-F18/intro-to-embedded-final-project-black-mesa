#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define F_CPU 1000000L // Clock Speed
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
//PINOUT 14 needed, 22 available 
//PC6	RST
//PD0	RX			//open
//PD1	TX			//open
//PD2				//open
//PD3	Door lock
//PD4	LEDG
//VCC	VCC
//GND	GND
//PB6	6 PCINT6
//PB7	7 PCINT7
//PD5	LEDR
//PD6	Siren
//PD7	LATCH		1
//PB0	0 PCINT0

//top down
//PC5	SCK			2
//PC4	DAT			3
//PC3	# PCINT11
//PC2	* PCINT10
//PC1	9 PCINT9
//PC0	8 PCINT8
//GND	GND
//Aref	VCC
//AVCC	VCC
//PB5	5 PCINT5
//PB4	4 PCINT4
//PB3	3 PCINT3
//PB2	2 PCINT2
//PB1	1 PCINT1
void send(uint8_t input){
	for(uint8_t i=0;i<8;i++){
		if((input & 0x01)== 0x01)
			PORTC |= (1<<PC4);
			else
			PORTC &= ~(1<<PC4);
			input = input>>1;
		PORTC |= (1<<PC5);
		PORTC &= ~(1<<PC5);
	}
}
void latch(void){
	PORTD |= (1<<PD7);
	PORTD &= ~(1<<PD7);
}
uint8_t counter;
ISR(TIMER0_OVF_vect){
	if(counter == 0x80){
		send( 0xCE<<1);//C
	}else if(counter == 0x40){
		send( 0x1C<<1);//U
	}else if(counter == 0x20){
		send( 0x15<<1);//N
	}else if(counter == 0x10){
		send( 0x0F<<1);//T
	}
	send(~counter);
	latch();
	if(counter == 0x80)
		counter = 0x08;

		counter = counter<<1;
}
int main(void){

	DDRB = 0xFF;
	DDRC = 0xFF;
	DDRD = 0xFF;//set all to output except used bits
	counter = 0x10;
	TCCR0A = (1<<CS00);
	TIMSK0 = (1<<TOIE0);

	while(1){
	sei();
	}
	return 0;
}

