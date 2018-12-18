#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

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


/*void Timer_Init(void){
	TIMSK0 = (1<<OCIE0A);
	TCCR0A = (1<<WGM01)|(1<<WGM00);
	TCCR0B = (1<<WGM02);	
	OCR0A = 255;
	//TIMSK1 = (1<<TOIE1);//timer 1 OVF interrupt enable
}*/


uint8_t receivedchars[] = {0x10,0x10,0x10,0x10};
uint8_t keypadctr = 0,sregctr = 0x10, attempts=0x66,icounter = 0;
uint8_t code[] = {0xB6,0x60,0xF6,0x66};
					//5194
/*ISR(TIMER1_COMPA_vect){
	PORTD &= ~(1<<PD3);
	TCCR1B &= ~(1<<CS11); //timer 1 clockdiv disable 

}*/
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
uint8_t sregctr;
ISR(TIMER0_OVF_vect){
	
	if(attempts == 0xFC){
		icounter++;
		if((icounter & 0x10) == 0)
			PORTD ^= (1<<PD6);
	
	}
		
	if(sregctr == 0x80){//9C 38 2A 1E
		send( receivedchars[0]);
	}else if(sregctr == 0x40){
		send( receivedchars[1]);
	}else if(sregctr == 0x20){
		send( receivedchars[2]);
	}else if(sregctr == 0x10){
		send( receivedchars[3]);
	}
	send(~sregctr);
	latch();
	if(sregctr == 0x80)
		sregctr = 0x08;

		sregctr = sregctr<<1;
}
//0 = 0xFC	5 = 0xB6	
//1 = 0x60	6 = 0xBE
//2 = 0xDA	7 = 0xE0
//3 = 0xF2	8 = 0xFE
//4 = 0x66	9 = 0xF6
void debounce(void){
	for(uint8_t j=0;j<2;j++)
	for(uint8_t i=0;i<255;i++)
	asm("nop");
}
void tone(void){
	for(uint8_t i=0;i<10;i++){
		PORTD |= (1<<PD6);
			for(uint8_t j=0;j<1;j++)
				asm("nop");
		PORTD &= ~(1<<PD6);
			for(uint8_t j=0;j<10;j++)
				asm("nop");
	}
}
void twotone(uint8_t num1, uint8_t delay1,uint8_t num2, uint8_t delay2){
	for(uint8_t i=0;i<num1;i++){
		PORTD |= (1<<PD6);
			for(uint8_t j=0;j<1;j++)
				asm("nop");
		PORTD &= ~(1<<PD6);
			for(uint8_t j=0;j<delay1;j++)
				asm("nop");
	}
			
	for(uint8_t j=0;j<3;j++)
		for(uint8_t i=0;i<255;i++);
	for(uint8_t i=0;i<num2;i++){
		PORTD |= (1<<PD6);
			for(uint8_t j=0;j<1;j++)
				asm("nop");
		PORTD &= ~(1<<PD6);
			for(uint8_t j=0;j<delay2;j++)
				asm("nop");
	}
}
int main(void){

	DDRB = 0x00;
	DDRC = 0xF0;
	DDRD = 0xFF;//set all to output except used bits
	PORTB = 0xFF;
	PORTC = 0x0F;
	PORTD = (1<<PD5);
		//button_int init
	//timer_init

	TCCR0A = (1<<CS00);
	TIMSK0 = (1<<TOIE0);
	sei();
	while(1){
	
	if(attempts != 0xFC)
		PORTD &= ~(1<<PD6);
	if((PINB & (1<<PB0)) == (0x00) ){
		tone();
		debounce();
		receivedchars[keypadctr] = 0xFC;//0
		PORTD ^= (1<<PD2);
		while((PINB & (1<<PB0)) != (1<<PB0) );
		keypadctr++;
	}
	else if((PINB & (1<<PB1)) == (0x00) ){
		tone();
		debounce();
		receivedchars[keypadctr] = 0x60;//1
		while((PINB & (1<<PB1)) != (1<<PB1));
		keypadctr++;
	}
	else if((PINB & (1<<PB2)) == (0x00) ){
		tone();
		debounce();
		receivedchars[keypadctr] = 0xDA;//2
		while((PINB & (1<<PB2)) != (1<<PB2));
		keypadctr++;
	}
	else if((PINB & (1<<PB3)) == (0x00) ){
		tone();
		debounce();
		receivedchars[keypadctr] = 0xF2;//3
		while((PINB & (1<<PB3)) != (1<<PB3));
		keypadctr++;
	}
	else if((PINB & (1<<PB4)) == (0x00) ){
		tone();
		debounce();
		receivedchars[keypadctr] = 0x66;//4
		while((PINB & (1<<PB4)) != (1<<PB4));
		keypadctr++;
	}
	else if((PINB & (1<<PB5)) == (0x00) ){
		tone();
		debounce();
		receivedchars[keypadctr] = 0xB6;//5
		while((PINB & (1<<PB5)) != (1<<PB5));
		keypadctr++;
	}
	else if((PINB & (1<<PB6)) == (0x00) ){
		tone();
		debounce();
		receivedchars[keypadctr] = 0xBE;//6
		while((PINB & (1<<PB6)) != (1<<PB6));
		keypadctr++;
	}
	else if((PINB & (1<<PB7)) == (0x00) ){
		tone();
		debounce();
		receivedchars[keypadctr] = 0xE0;//7
		while((PINB & (1<<PB7)) != (1<<PB7));
		keypadctr++;
	}
	else if((PINC & (1<<PINC0)) == (0x00)){
		tone();
		debounce();
		receivedchars[keypadctr] = 0xFE;//8
		while((PINC & (1<<PC0)) != (1<<PC0));
		keypadctr++;
	}
	else if((PINC & (1<<PC1)) == (0x00)){
		tone();
		debounce();
		receivedchars[keypadctr] = 0xF6;//9
		while((PINC & (1<<PC1)) != (1<<PC1));
		keypadctr++;
	}
	else if((PINC & (1<<PC2)) == (0x00)){
		tone();
		debounce();
		receivedchars[keypadctr] = 0xEE;//* (A)
		while((PINC & (1<<PC2)) != (1<<PC2));
		keypadctr++;
	}
	else if((PINC & (1<<PC3)) == (0x00)){
		tone();
		debounce();
		receivedchars[keypadctr] = 0x3E;//# (B)
		while((PINC & (1<<PC3)) != (1<<PC3));
		keypadctr++;
	}
		
		if(keypadctr == 4){
			for(uint8_t j=0;j<10;j++)
				for(uint8_t i=0;i<255;i++);
			keypadctr = 0;
			if((receivedchars[0] == code[0]) & (receivedchars[1] == code[1])& (receivedchars[2] == code[2])& (receivedchars[3] == code[3])){
				//right code
				PORTD |= (1<<PD4);
				PORTD |= (1<<PD3);
				PORTD &= ~(1<<PD5);
				icounter = 0;
				attempts = 0x66;
				receivedchars[0] = 0x9E;
				receivedchars[1] = 0x1C;
				receivedchars[2] = 0x1C;
				receivedchars[3] = 0x3A;
				twotone(50,12,60,5);
			for(uint8_t j=0;j<50;j++)
				for(uint8_t i=0;i<255;i++);
			}else{
				//wrong code
				if(attempts == 0x66)
				attempts = 0xF2;//3
				else if(attempts == 0xF2)
				attempts = 0xDA;//2
				else if(attempts == 0xDA)
				attempts = 0x60;//1
				else if(attempts == 0x60)
				attempts = 0xFC;
				
				
				receivedchars[0] = 0x2A;
				receivedchars[1] = 0x3A;
				receivedchars[2] = 0xCE;
				receivedchars[3] = 0x9E;
				twotone(20,30,20,30);
				for(uint8_t j=0;j<30;j++)
					for(uint8_t i=0;i<255;i++);
				if(attempts != 0xFC){
				receivedchars[0] = 0xEE;
				receivedchars[1] = 0x12;
				receivedchars[2] = 0x00;
				receivedchars[3] = attempts;
				}
				for(uint8_t j=0;j<50;j++)
					for(uint8_t i=0;i<255;i++);
					
				
			}
			
				PORTD &= ~(1<<PD4);
				PORTD &= ~(1<<PD3);
				PORTD |= (1<<PD5);
				if(keypadctr == 0){//had to do this for some reason, or the above 20 lines would not execute. strange.
					receivedchars[0] = 0x10;
					receivedchars[1] = 0x10;
					receivedchars[2] = 0x10;
					receivedchars[3] = 0x10;
				}
				
			
		}
	}
	return 0;
}

