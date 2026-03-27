#ifndef MAX7219_REGMAX7219_H_
#define MAX7219_REGMAX7219_H_

/* ROWS */
#define REG_ROW0      0x01
#define REG_ROW1      0x02
#define REG_ROW2      0x03
#define REG_ROW3      0x04
#define REG_ROW4      0x05
#define REG_ROW5      0x06
#define REG_ROW6      0x07
#define REG_ROW7      0x08

/* CONTROL REGISTERS */
#define REG_DECODE_MODE 0x09    // BCD decode mode control
#define REG_INTENSITY   0x0A    // Brightness control (0x00 - 0x0F)
#define REG_SCAN_LIMIT  0x0B    // Number of digits to scan (0-7)
#define REG_SHUTDOWN    0x0C    // Shutdown register (0 = shutdown, 1 = normal)
#define REG_DISPLAYTEST 0x0F    // Display test (1 = test mode, all LEDs on)

#endif
