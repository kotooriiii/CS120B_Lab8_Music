/*	Author: Carlos Miranda cmira039@ucr.edu
 *  Partner(s) Name: n/a
 *	Lab Section: 23
 *	Assignment: Lab #8  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: 
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char song[64][4] = {
	"G#3", "E_4", "E_4", "F#4", "D#4", "D#4", "E_4", "E_4",
	"C#4", "C#4", "D#4", "D#4", "A_3", "A_3", "G#3", "G#3",
	"G#3", "E_4", "E_4", "F#4", "D#4", "D#4", "E_4", "E_4",
	"C#4", "C#4", "B_4", "B_4", "C#5", "C#5", "!!!", "!!!"
	,			  
	"!!!", "E_4", "E_4", "F#4", "D#4", "D#4", "E_4", "E_4",
	"C#4", "C#4", "D#4", "D#4", "A_3", "A_3", "G#3", "G#3",
	"G#3", "C#4", "C#4", "C#4", "C#4", "C#4", "C#4", "C#4",
	"C#4", "C#4", "!!!", "!!!", "!!!", "A#3", "A#3", "C_4"
				  };

unsigned char songIndex = 0x00;
unsigned char songSize = 0x40;

enum SONG_STATES {INIT, READY, PLAYING, END} SONG_STATE;

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

unsigned char IPINA()
{
	return ~PINA;
}

unsigned char isOnlyA0()
{
	return IPINA() & 0x01;
}


/*
	Timer start
*/

void TimerISR()
{
	TimerFlag = 1;
}

void TimerOff()
{
	TCCR1B = 0x00;
}

void TimerOn()
{
	TCCR1B = 0x0B;

	OCR1A = 125;

	TIMSK1 = 0x02;

	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;

	SREG |= 0x80;

}

ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0)
	{
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

/*
	Timer end
*/



/*
	PWM Start
*/

void set_PWM(double frequency) {
    static double current_frequency;

    if (frequency != current_frequency) {
        if(!frequency)
            TCCR3B &= 0x08;
        else
            TCCR3B |= 0x03;

        if(frequency < 0.954)
            OCR3A = 0xFFFF;
        else if (frequency > 31250)
            OCR3A = 0x0000;
        else
            OCR3A = (short) (8000000 / (128 * frequency)) - 1;
        
        TCNT3 = 0;
        current_frequency = frequency;
    }
}

void PWM_on() {
    TCCR3A = (1 << COM3A0);
    TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
    set_PWM(0);
}

void PWM_off() {
    TCCR3A = 0x00;
    TCCR3B = 0x00;
}

/*
	PWM End
*/


double getFrequency(unsigned char index)
{
	char note = song[index][0];
	char format = song[index][1];
	char octave = song[index][2];
	if(note == '!')
		if(octave == '!')
			if(format == '!')
				return 0;
	if(note == 'E')
		if(octave == '4')
			if(format == '_')
				return 329.63;
	if(note == 'G')
		if(octave == '3')
			if(format == '#')
				return 207.652;
	if(note == 'F')
		if(octave == '4')
			if(format == '#')
				return 369.99;
	if(note == 'D')
		if(octave == '4')
			if(format == '#')
				return 311.127;
	if(note == 'C')
		if(octave == '4')
			if(format == '#')
				return 277.18;
	if(note == 'C')
		if(octave == '4')
			if(format == '_')
				return 261.63;
	if(note == 'C')
		if(octave == '5')
			if(format == '#')
				return 554.37;
	if(note == 'A')
		if(octave == '3')
			if(format == '_')
				return 216.00;
	if(note == 'A')
		if(octave == '3')
			if(format == '#')
				return 233.08;
	if(note == 'B')
		if(octave == '4')
			if(format == '_')
				return 493.88;
	return 3520.00; 
}

void tickSongSM()
{
	switch(SONG_STATE)
	{
		case INIT:
		SONG_STATE = READY;
		break;
		
		case READY:
		if(isOnlyA0())
		{
			SONG_STATE = PLAYING;
		}
		else 
		{
		SONG_STATE = READY;
		}
		break;
		
		case PLAYING:
		if(songIndex == songSize)
		{
			SONG_STATE = END;
		} 
		break;
		
		case END:
		if(!isOnlyA0())
		{
			SONG_STATE = READY;
		}
		break;
		
		default:
		SONG_STATE = INIT;
		break; 
	}
	
	switch(SONG_STATE)
	{
		case INIT:
		break;
		
		case READY:
		songIndex = 0x00;
		break;
		
		case PLAYING:
		set_PWM(getFrequency(songIndex));
		songIndex++;
		break;
		
		case END:
		set_PWM(0.0);
		break;
		
		default:
		break; 
	}
}

int main(void) 
{
	
	//Inputs
	DDRA = 0x00; PORTA = 0xFF; 
	
	//Outputs
	DDRB = 0xFF; PORTB = 0x00; 
		
	SONG_STATE = INIT;
	
	double period = 60000.0 / 144 / 2;
	TimerSet(period);
	TimerOn();
	
	PWM_on();
	
	while(1) 
	{
		tickSongSM();
		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 1;
}
	



