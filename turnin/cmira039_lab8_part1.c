/*	Author: Carlos Miranda cmira039@ucr.edu
 *  Partner(s) Name: n/a
 *	Lab Section: 23
 *	Assignment: Lab #8  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: https://youtu.be/4SSCa8F7w1k
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char song[3][3] = {
	"C4", "D4", "E4"
	};


unsigned char IPINA()
{
	return ~PINA;
}

unsigned char isOnlyA0()
{
	return IPINA() == 0x01;
}

unsigned char isOnlyA1()
{
	return IPINA() == 0x02;
}

unsigned char isOnlyA2()
{
	return IPINA() == 0x04;
}

unsigned char isA0()
{
	return IPINA() & 0x01;
}

unsigned char isA1()
{
	return IPINA() & 0x02;
}

unsigned char isA2()
{
	return IPINA() & 0x04;
}

enum SONG_STATES {INIT, WAIT, PLAY_NOTE_C, PLAY_NOTE_D, PLAY_NOTE_E} SONG_STATE;

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
	char octave = song[index][1];
	
	if(note == '!')
		if(octave == '!')
				return 0;
	if(note == 'C')
		if(octave == '4')
				return 261.63;
	if(note == 'D')
		if(octave == '4')
				return 293.66;
	if(note == 'E')
		if(octave == '4')
				return 329.63;
	return 3520.00; 
}

void tickNoteSM()
{
	switch(SONG_STATE)
	{
		case INIT:
		SONG_STATE = WAIT;
		break;
		
		case WAIT:
		if(isOnlyA0())
		{
			SONG_STATE = PLAY_NOTE_C;
		}
		else if(isOnlyA1())
		{
			SONG_STATE = PLAY_NOTE_D;
		}
		else if(isOnlyA2())
		{
			SONG_STATE = PLAY_NOTE_E;
		}
		else 
		{
			SONG_STATE = WAIT;
		}
		break;
		
		case PLAY_NOTE_C:
		if(isOnlyA0())
		{
			SONG_STATE = PLAY_NOTE_C;
		}
		else 
		{
			SONG_STATE = WAIT;
		}
		break;
		
		case PLAY_NOTE_D:
		if(isOnlyA1())
		{
			SONG_STATE = PLAY_NOTE_D;
		}
		else 
		{
			SONG_STATE = WAIT;
		}
		break;
		
		case PLAY_NOTE_E:
		if(isOnlyA2())
		{
			SONG_STATE = PLAY_NOTE_E;
		}
		else 
		{
			SONG_STATE = WAIT;
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
		
		case WAIT:
		set_PWM(0);
		break;
		
		case PLAY_NOTE_C:
		set_PWM(getFrequency(0));
		break;
		
		case PLAY_NOTE_D:
		set_PWM(getFrequency(1));
		break;
		
		case PLAY_NOTE_E:
		set_PWM(getFrequency(2));
		break;
		
		default:
		SONG_STATE = INIT;
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

	PWM_on();
	
	while(1) 
	{
		tickNoteSM();
	}
	
	PWM_off();
	return 1;
}
	



