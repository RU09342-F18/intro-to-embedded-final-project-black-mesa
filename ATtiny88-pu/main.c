#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

//PINOUT
 
//PC6	RST
//PD0				//open
//PD1				//open
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



uint8_t receivedchars[] = {0x10,0x10,0x10,0x10};//four understores, it will store this code as it is entered and display it.
uint8_t keypadctr = 0,sregctr = 0x10, attempts=0x66,icounter = 0;
uint8_t code[] = {0xB6,0x60,0xF6,0x66};//door code
					//5194

void send(uint8_t input){//transmit byte to the shift register
	for(uint8_t i=0;i<8;i++){//run 8 times
		if((input & 0x01)== 0x01)//if 1, set output to 1
			PORTC |= (1<<PC4);
		else
			PORTC &= ~(1<<PC4);//else 0
		input = input>>1;//shift over 1
		PORTC |= (1<<PC5);//toggle clock
		PORTC &= ~(1<<PC5);
	}
}
void latch(void){//function used in miltiplexing the 7seg display through the shift register
	PORTD |= (1<<PD7);
	PORTD &= ~(1<<PD7);
}
uint8_t sregctr;
ISR(TIMER0_OVF_vect){//overflow vector
	
	if(attempts == 0xFC){//if attempts = 0, sound alarm
		icounter++;
		if((icounter & 0x10) == 0)
			PORTD ^= (1<<PD6);
	
	}
		
	if(sregctr == 0x80){//fancy thing done here, sregctr is the second thing sent. It acts as the cathode on the 7seg, and is also used for deciding which digit is sent
		send( receivedchars[0]);
	}else if(sregctr == 0x40){
		send( receivedchars[1]);
	}else if(sregctr == 0x20){
		send( receivedchars[2]);
	}else if(sregctr == 0x10){
		send( receivedchars[3]);
	}
	send(~sregctr);//send the inversino of sregctr
	latch();//latch the shift register, displaying the values.
	if(sregctr == 0x80)//if it is on the 4th digit, set it as just before the 1st digit
		sregctr = 0x08;

		sregctr = sregctr<<1;//shift over sregctr
}
//0 = 0xFC	5 = 0xB6	
//1 = 0x60	6 = 0xBE
//2 = 0xDA	7 = 0xE0
//3 = 0xF2	8 = 0xFE
//4 = 0x66	9 = 0xF6
void debounce(void){//(very) basic debounce
	for(uint8_t j=0;j<2;j++)
		for(uint8_t i=0;i<255;i++)
			asm("nop");
}
void tone(void){//single beep function. This function still works through the alarm.
	for(uint8_t i=0;i<10;i++){
		PORTD |= (1<<PD6);
			for(uint8_t j=0;j<1;j++)
				asm("nop");
		PORTD &= ~(1<<PD6);
			for(uint8_t j=0;j<10;j++)
				asm("nop");
	}
}
void twotone(uint8_t num1, uint8_t delay1,uint8_t num2, uint8_t delay2){//this function plays two tones based on the inputs given as delays and number of cycles, giving the frequency and frequency length. This tone still works through the alarm, which is nice
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
	PORTD = (1<<PD5);//set a defined value for all outputs and pullups for the inputs

	
	TCCR0A = (1<<CS00);//timer0 init
	TIMSK0 = (1<<TOIE0);//timer0 OVF interrupt init, approx. 3.9 KHz
	sei();//set up interrupts
	while(1){
		if(attempts != 0xFC)
			PORTD &= ~(1<<PD6);//turn off the pin just in case it gets stuck on and can potentially burn out the speaker
		if((PINB & (1<<PB0)) == (0x00) ){//key pressed 0
			tone();//play a chort beep
			debounce();//delay for a few ms. In a future design, timer1 will be used for debouncing.
			receivedchars[keypadctr] = 0xFC;//0, set the 7seg value for 0 to the array
			while((PINB & (1<<PB0)) != (1<<PB0) );
			keypadctr++;//increment the number of digits received
		}
		else if((PINB & (1<<PB1)) == (0x00) ){//key pressed 1
			tone();
			debounce();
			receivedchars[keypadctr] = 0x60;//1
			while((PINB & (1<<PB1)) != (1<<PB1));
			keypadctr++;
		}
		else if((PINB & (1<<PB2)) == (0x00) ){//key pressed 2
			tone();
			debounce();
			receivedchars[keypadctr] = 0xDA;//2
			while((PINB & (1<<PB2)) != (1<<PB2));
			keypadctr++;
		}
		else if((PINB & (1<<PB3)) == (0x00) ){//key pressed 3
			tone();
			debounce();
			receivedchars[keypadctr] = 0xF2;//3
			while((PINB & (1<<PB3)) != (1<<PB3));
			keypadctr++;
		}
		else if((PINB & (1<<PB4)) == (0x00) ){//key pressed 4
			tone();
			debounce();
			receivedchars[keypadctr] = 0x66;//4
			while((PINB & (1<<PB4)) != (1<<PB4));
			keypadctr++;
		}
		else if((PINB & (1<<PB5)) == (0x00) ){//key pressed 5
			tone();
			debounce();
			receivedchars[keypadctr] = 0xB6;//5
			while((PINB & (1<<PB5)) != (1<<PB5));
			keypadctr++;
		}
		else if((PINB & (1<<PB6)) == (0x00) ){//key pressed 6
			tone();
			debounce();
			receivedchars[keypadctr] = 0xBE;//6
			while((PINB & (1<<PB6)) != (1<<PB6));
			keypadctr++;
		}
		else if((PINB & (1<<PB7)) == (0x00) ){//key pressed 7
			tone();
			debounce();
			receivedchars[keypadctr] = 0xE0;//7
			while((PINB & (1<<PB7)) != (1<<PB7));
			keypadctr++;
		}
		else if((PINC & (1<<PINC0)) == (0x00)){//key pressed 8
			tone();
			debounce();
			receivedchars[keypadctr] = 0xFE;//8
			while((PINC & (1<<PC0)) != (1<<PC0));
			keypadctr++;
		}
		else if((PINC & (1<<PC1)) == (0x00)){//key pressed 9
			tone();
			debounce();
			receivedchars[keypadctr] = 0xF6;//9
			while((PINC & (1<<PC1)) != (1<<PC1));
			keypadctr++;
		}
		else if((PINC & (1<<PC2)) == (0x00)){//key pressed *
			tone();
			debounce();
			receivedchars[keypadctr] = 0xEE;//* (A)
			while((PINC & (1<<PC2)) != (1<<PC2));
			keypadctr++;
		}
		else if((PINC & (1<<PC3)) == (0x00)){//key pressed # currently these two keys have no use, in a future implementation, this will be used to set the password.
			tone();
			debounce();
			receivedchars[keypadctr] = 0x3E;//# (B)
			while((PINC & (1<<PC3)) != (1<<PC3));
			keypadctr++;
		}
		
			if(keypadctr == 4){//4 being the number of digits in a code
				for(uint8_t j=0;j<10;j++)
					for(uint8_t i=0;i<255;i++);//short delay to make the keypad seem more fluid
				keypadctr = 0;//reset counter
				if((receivedchars[0] == code[0]) & (receivedchars[1] == code[1])& (receivedchars[2] == code[2])& (receivedchars[3] == code[3])){
				//check that the code equals
					
					//right code
					PORTD |= (1<<PD4);//set green LED
					PORTD |= (1<<PD3);//set door open
					PORTD &= ~(1<<PD5);//turn off red LED
					icounter = 0;//this is used to divide the timer0 OVF rate by 2, so it is reset when the code is correct
					attempts = 0x66;//0x66 = 7seg(0) so this resets the number of attempts
					receivedchars[0] = 0x9E;//e
					receivedchars[1] = 0x1C;//l
					receivedchars[2] = 0x1C;//l
					receivedchars[3] = 0x3A;//o I couldn't do "ack" "correct" or "welcome" on a 7seg display, so this was tne next bast thing
					twotone(50,12,60,5);
				for(uint8_t j=0;j<250;j++)
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
				
				
					receivedchars[0] = 0x2A;//n
					receivedchars[1] = 0x3A;//o
					receivedchars[2] = 0xCE;//p
					receivedchars[3] = 0x9E;//e
					twotone(20,30,20,30);
					for(uint8_t j=0;j<30;j++)
						for(uint8_t i=0;i<255;i++);//short delay so "nope" is readable
					if(attempts != 0xFC){
					receivedchars[0] = 0xEE;// A
					receivedchars[1] = 0x12;// =
					receivedchars[2] = 0x00;//' '
					receivedchars[3] = attempts;//attempts in 7seg format
					}
					for(uint8_t j=0;j<50;j++)
						for(uint8_t i=0;i<255;i++);//short delay so "A= 3" "A= 2" "A= 1" is visible. At A= 0, a siren sounds 
					
				
				}
			
					PORTD &= ~(1<<PD4);//clear green LED
					PORTD &= ~(1<<PD3);//turn off door lock
					PORTD |= (1<<PD5);
					if(keypadctr == 0){//had to do this for some reason, or the above 20 lines would not execute. strange.
						receivedchars[0] = 0x10;//_
						receivedchars[1] = 0x10;//_
						receivedchars[2] = 0x10;//_
						receivedchars[3] = 0x10;//_ i did underscores (bottom segment) so you know the device is active.
					}
				
			
			}
	}
	return 0;
}

