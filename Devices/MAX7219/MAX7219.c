#include "MAX7219.h"
#include "regMAX7219.h"

static uint8_t matrix[MATRIX_SIZE];				 				// byte = row, bits = columns

static uint8_t matrixMAX[DAISY_CHAIN_NUMBER][MATRIX_SIZE];	// 4 module & 8 rows

static uint8_t matrix_counter_bits[MATRIX_SIZE];

static uint8_t num[10][3] = {{0xF8, 0x88, 0xF8},
							 {0x08, 0xF8, 0x48},
							 {0xC8, 0xA8, 0x98},
							 {0xF8, 0xA8, 0x88},
							 {0x20, 0x38, 0xE0},
							 {0x90, 0xA8, 0xE8},
							 {0xB8, 0xA8, 0xF8},
							 {0xE0, 0x90, 0x88},
							 {0xF8, 0xA8, 0xF8},
							 {0xF8, 0xA0, 0xE0}};

void initMAX7219(void)
{
	initSPI();

	uint8_t data[DAISY_CHAIN_NUMBER];

	for(int i = 0; i < DAISY_CHAIN_NUMBER; i++) data[i] = 0x00;
	writeSPIDaisy(REG_DISPLAYTEST, data);


	for(int i = 0; i < DAISY_CHAIN_NUMBER; i++) data[i] = 0x01;
	writeSPIDaisy(REG_SHUTDOWN, data);


	for(int i = 0; i < DAISY_CHAIN_NUMBER; i++) data[i] = 0x07;
	writeSPIDaisy(REG_SCAN_LIMIT, data);

	for(int i = 0; i < DAISY_CHAIN_NUMBER; i++) data[i] = 0x01;
	writeSPIDaisy(REG_INTENSITY, data);

	for(int i = 0; i < DAISY_CHAIN_NUMBER; i++) data[i] = 0x00;
	writeSPIDaisy(REG_DECODE_MODE, data);

	clearMAX7219();

	return;
}

void clearMAX7219()
{
	uint8_t data[DAISY_CHAIN_NUMBER];
	for(int i = 0; i < DAISY_CHAIN_NUMBER; i++) data[i] = 0x00;

	for(uint8_t row = 0; row < 8; row++)
	{
		writeSPIDaisy_Interrupt(row + 1, data); //INTERRUPT HERE
	}

	return;
}

void renderMAX7219(void)
{
	uint8_t column[DAISY_CHAIN_NUMBER];

	for(uint8_t row = 0; row < DAISY_MATRIX_SIZE_Y; row++)
	{
		for(uint8_t module = 0; module < DAISY_CHAIN_NUMBER; module++)
		{
			column[module] = matrixMAX[module][row];
		}
		writeSPIDaisy_Interrupt(row + 1, column);
	}

	return;
}

void clearMatrix(void)
{
	for(uint8_t row = 0; row < DAISY_MATRIX_SIZE_Y; row++)
	{
		for(uint8_t module = 0; module < DAISY_CHAIN_NUMBER; module++)
		{
			matrixMAX[module][row] = 0;
		}
	}
	return;
}

void setPixel(uint8_t x, uint8_t y, uint8_t state)
{
	if(x >= DAISY_MATRIX_SIZE_X || y >= DAISY_MATRIX_SIZE_Y) return;

	uint8_t module = x / 8;
	uint8_t column    = x % 8;

	if(state)
	{
		matrixMAX[module][y] |= (1 << column);
	}
	else
	{
		matrixMAX[module][y] &= ~(1 << column);
	}

	return;
}

void counter()
{
	//clearFlag = 0;
	static int ones = 0;
	static int tenth = 0;
	static int s = 0, n = 0;

	for(int  i = 0; i < 3; i++)
	{
		matrix[i] = num[ones][i];
		matrix[i + 5] = num[tenth][i];
	}

	ones++;
	if(ones > 9)
	{
		ones = 0;

		tenth++;
		for(int  i = 0; i < 3; i++)
		{
			if(tenth > 9) tenth = 0;
		}
	}

	setPixel(n, s, 1);

	s++;
	if(s >= 8)
	{
		s = 0;
		n++;
		if(n >= 3)
		{
			n= 0;
			//clearFlag = 1;

			for(uint8_t row = 0; row < MATRIX_SIZE; row++)
			{
				matrix_counter_bits[row] = 0;
				matrix[row] &= ~matrix_counter_bits[row];
			}
			matrix[4] = 0;
			matrix[3] = 0;
		}

	}
	return;
}

void testAllOn()
{
    uint8_t data[DAISY_CHAIN_NUMBER];

    for(int i = 0; i < DAISY_CHAIN_NUMBER; i++) data[i] = 0xFF;

    for(uint8_t row = 0; row < 8; row++)
    {
        writeSPIDaisy(row + 1, data);
    }
}

void testLine()
{
	clearMAX7219();

	for(int x = 0; x < 32; x++)
	{
		if(x > 7 && x < 16)
		{
			setPixel(x, 2, 1);
		}
	}
	setPixel(15, 3, 1);

	renderMAX7219();


	return;
}

