/*
 * CogBadge THT
 *
 * Author : Benjamin
 */ 

#define F_CPU 1000000

#include <avr/io.h>
#include <util/delay.h>



int patternStageTime = 20; // Time between pattern steps in multiples of x10ms

//=================================//
//   Patterns                      //
//=================================//

// Pattern struct
typedef struct {
	uint8_t size; // Number of stages to the pattern to cycle through
	uint8_t stages[16]; // Pattens may have a maximum of 16 stages
} Pattern;

// Create patterns
#define patternsSIZE 9
Pattern patterns[patternsSIZE] = {
	{
		.size = 4,
		.stages = {
			0b01111111,
			0b10111111,
			0b11011111,
			0b11101111
		}
	},
	{
		.size = 4,
		.stages = {
			0b01111111,
			0b11101111,
			0b11011111,
			0b10111111
		}
	},
	{
		.size = 4,
		.stages = {
			0b00111111,
			0b10011111,
			0b11001111,
			0b01101111
		}
	},
	{
		.size = 4,
		.stages = {
			0b00111111,
			0b01101111,
			0b11001111,
			0b10011111
		}
	},
	{
		.size = 2,
		.stages = {
			0b01011111,
			0b10101111
		}
	},
	{
		.size = 2,
		.stages = {
			0b00001111,
			0b11111111
		}
	},
	{
		.size = 1,
		.stages = {
			0b00001111
		}
	},
	{
		.size = 16,
		.stages = {
			0b11100001,
			0b11010001,
			0b10110001,
			0b01110001,
			0b11100010,
			0b11010010,
			0b10110010,
			0b01110010,
			0b11100100,
			0b11010100,
			0b10110100,
			0b01110100,
			0b11101000,
			0b11011000,
			0b10111000,
			0b01111000
		}
	},
	{
		.size = 16,
		.stages = {
			0b11100001,
			0b01111000,
			0b10111000,
			0b11011000,
			0b11101000,
			0b01110100,
			0b10110100,
			0b11010100,
			0b11100100,
			0b01110010,
			0b10110010,
			0b11010010,
			0b11100010,
			0b01110001,
			0b10110001,
			0b11010001
		}
	}
};


//=================================//
//   IMPL                          //
//=================================//

char bitChangeLow(uint8_t new, uint8_t old, uint8_t bit) {
	bit = 1<<bit;
	return ((new & bit) != (old & bit)) && (new & bit) == 0;
}

// Debounce buttons with four samples
uint8_t pinb[4];
uint8_t debouncedPINB() {
	for (int i = 3; i > 0; i--) {
		pinb[i] = pinb[i-1];
	}
	pinb[0] = PINB;
	
	return pinb[0] & pinb[1] & pinb[2] & pinb[3];
}

// Update and step through patterns
uint8_t currentPattern = 0;
uint8_t lastPattern = 0;
uint8_t nextStage = 0;
void stepPattern() {
	if (currentPattern != lastPattern) {
		lastPattern = currentPattern;
		nextStage = 0;
	}
	
	PORTA = patterns[currentPattern].stages[nextStage++];
	
	if (nextStage >= patterns[currentPattern].size) {
		nextStage = 0;
	}
}


int main(void) {
	DDRA = 0xFF;
	
	uint8_t currentPatternTime = 0;
	
	uint8_t previousPINB = debouncedPINB(); // Used for detecting button state change
    while (1) {

		// Check buttons
		uint8_t currentPINB = debouncedPINB();
		if (bitChangeLow(currentPINB, previousPINB, PINB1)) {
			// Button SW3 changes pattern step time
			patternStageTime *= 2; // Double step time each time
			if (patternStageTime > 160) {
				// Cycle back to a step time of 50ms after 1600ms
				patternStageTime = 5;
			}
		}
		else if (bitChangeLow(currentPINB, previousPINB, PINB2)) {
			// Button SW2 changes pattern
			currentPattern++;
			currentPatternTime = patternStageTime; // Fast forward time for immediate pattern change
			if (currentPattern >= patternsSIZE) {
				currentPattern = 0;
			}
		}
		previousPINB = currentPINB; // Update memory of button state

		// Increment pattern time
		if (++currentPatternTime >= patternStageTime) {
			stepPattern();
			currentPatternTime = 0;
		}
		
		// Sleep for 10ms
		_delay_ms(10);
    }
}