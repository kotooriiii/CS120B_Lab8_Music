/*	Author: Carlos Miranda cmira039@ucr.edu
 *  Partner(s) Name: n/a
 *	Lab Section: 23
 *	Assignment: Lab #8  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: https://youtu.be/8gvkyuXJ8-4
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned char song[8][3] = {
	"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"
				  };

unsigned char noteIndex = 0x00;
unsigned char noteSize = 0x08;

double currFrequency = 261.63;

unsigned char isPoweredOn = 0x00;


enum NOTE_POWER_ENUM {NP_INIT, NP_WAIT, NP_TOGGLE, NP_END} NOTE_POWER;
enum NOTE_STATES_ENUM {NS_INIT, NS_WAIT, NS_INC, NS_DEC, NS_HOLD} NOTE_STATE;

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
	if(note == 'F')
		if(octave == '4')
				return 349.23;
	if(note == 'G')
		if(octave == '4')
				return 392.00;
	if(note == 'A')
		if(octave == '4')
				return 440.00;
	if(note == 'B')
		if(octave == '4')
				return 493.88;
	if(note == 'C')
		if(octave == '5')
				return 523.25;
	return currFrequency; 
}

void tickPowerSM()
{
	switch(NOTE_POWER)
	{
		case NP_INIT:
		NOTE_POWER = NP_WAIT;
		break;
		
		case NP_WAIT:
		if(isOnlyA2())
		{
			NOTE_POWER = NP_TOGGLE;
		}
		else
		{
			NOTE_POWER = NP_WAIT;
		}
		break;
		
		case NP_TOGGLE:
		NOTE_POWER = NP_END;
		break;

		case NP_END:
		if(isA2())
		{
			NOTE_POWER = NP_END;
		}
		else 
		{
			NOTE_POWER = NP_WAIT;
		}
		break;
		

		
		default:
		NOTE_POWER = NP_INIT;
		break; 
	}
	switch(NOTE_POWER)
	{
		case NP_INIT:
		break;
		
		case NP_WAIT:
		break;
		
		case NP_TOGGLE:
		
		//Toggle
		if(isPoweredOn)
		{
			isPoweredOn = 0x00;
		}
		else
		{
			isPoweredOn = 0x01;
		}
		
		if(isPoweredOn)
		{
			set_PWM(currFrequency);
		}
		else
		{
			set_PWM(0);
		}
		break;

		case NP_END:
		break;

		default:
		break; 
	}
}

void tickNoteSM()
{
	if(!isPoweredOn)
	{
		NOTE_STATE = NS_WAIT;
		return;
	}
	
	switch(NOTE_STATE)
	{
		case NS_INIT:
		NOTE_STATE = NS_WAIT;
		break;
		
		case NS_WAIT:
		if(isOnlyA0())
		{
			NOTE_STATE = NS_INC;
		}
		else if(isOnlyA1())
		{
			NOTE_STATE = NS_DEC;
		}
		else
		{
			NOTE_STATE = NS_WAIT;
		}
		break;
		
		case NS_INC:
		case NS_DEC:
		NOTE_STATE = NS_HOLD;
		break;
		
		case NS_HOLD:
		if(isA0() || isA1())
		{
			NOTE_STATE = NS_HOLD;
		}
		else
		{
			NOTE_STATE = NS_WAIT;
		}
		break;
		
		default:
		NOTE_STATE = NS_INIT;
		break; 
	}
	
	switch(NOTE_STATE)
	{
		case NS_INIT:
		break;
		
		case NS_WAIT:
		break;
		
		case NS_INC:
		
	        currFrequency = getFrequency(++noteIndex);
		set_PWM(currFrequency);
		
		if(noteIndex == noteSize)
			noteIndex = noteSize - 1;
		break;
	
		case NS_DEC:
	
		if(noteIndex == 0)
			break;

		currFrequency = getFrequency(--noteIndex);
		set_PWM(currFrequency);
		
		;		
		break;
		
		case NS_HOLD:
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
		
	NOTE_POWER = NP_INIT;
	NOTE_STATE = NS_INIT;

	PWM_on();
	
	while(1) 
	{
		tickPowerSM();
		tickNoteSM();
	}
	
	PWM_off();
	return 1;
}
	


