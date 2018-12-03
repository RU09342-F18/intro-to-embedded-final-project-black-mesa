#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define F_CPU 1000000L // Clock Speed
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1


void USART_Init( unsigned int ubrr){
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;//setup usart bits
	UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);//tx rx enable, RX interrupt enable
}
void Timer_Init(void){
	TCCR0A = (1<<COM0A1)|(1<<WGM01)|(1<<WGM00);//fast pwm on timer0
	TCCR0B = (1<<CS01);//3.9 KHz overflow on timer 0
	OCR0A = 0;//Timer0 PWM value init

	TCCR1B = (1<<CS11)|(1<<WGM12); //timer 1 clockdiv 
	OCR1AH = (1<<6)|(1<<1);//timer 1 overflow rate high byte
	OCR1AL = 0;//low byte
	TIMSK1 = (1<<OCIE1A);//timer 1 interrupt enable
	
}
void ADC_init(void){
	ADMUX = (1<<MUX0)|(1<<ADLAR);//ADC pin 24
	ADCSRA = (1<<ADEN)|(1<<ADPS1)|(1<<ADPS0)|(1<<ADATE);//125 KHz ADC
	ADCSRB = (1<<ADTS2)|(1<<ADTS1);//manual interrupt
}

ISR(TIMER1_COMPA_vect){

}

void USART_Transmit( uint8_t data ){
	while ( !( UCSR0A & (1<<UDRE0)) );//wait until buffer is empty
	UDR0 = data;
}
void sendint(uint8_t temp){//converts binary to ascii
	uint8_t tempbytes[] = {0,0,0};
	while(temp >= 100){
		tempbytes[2]++;
		temp-=100;
	}while(temp >= 10){
		tempbytes[1]++;
		temp-=10;
	}while(temp != 0){
		tempbytes[0]++;
		temp-=1;
	}
	USART_Transmit(tempbytes[2]+48);//transmit the result over UART
	USART_Transmit(tempbytes[1]+48);
	USART_Transmit(tempbytes[0]+48);

}
ISR(USART_RX_vect){
	
}
int main(void){

	DDRB = 0xFF;
	DDRC = 0xFD;
	DDRD = 0xFF;//set all to output except used bits
	USART_Init(MYUBRR);//init uart
	Timer_Init();//init timer
	ADC_init();//init ADC
	while(1){
		ADCSRA |= (1<<ADSC);//enable conversion
		asm("SEI");//set up interrupts
		SMCR = (1<<SE);//sleep enable
		asm("sleep");//sleep
		//stuff
	}
	return 0;
}

