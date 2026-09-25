#include "ssd1306.h"
#include "i2c1.h"
#include "font5x7.h"

static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
static uint8_t cursorX = 0, cursorY = 0;

static void SSD1306_WriteCmd(uint8_t cmd) {
	uint8_t buf[2] = {0x00, cmd};
	I2C1_WriteBytes(SSD1306_I2C_ADDR, buf, 2);
}

static void SSD1306_WriteData(uint8_t *data, uint16_t len) {
	uint8_t buf[17];
	uint16_t sent = 0;

	while(sent < len) {
		uint16_t chunk = (len-sent > 16)? 16 : (len-sent);
		buf[0] = 0x40;
		for(uint16_t i = 0; i < chunk; i++) buf[1+i] = data[sent+i];
		I2C1_WriteBytes(SSD1306_I2C_ADDR, buf, chunk + 1);
		sent += chunk;
	}
}

void SSD1306_Init(void) {
	 SSD1306_WriteCmd(0xAE);             // Display OFF
	 SSD1306_WriteCmd(0xD5); SSD1306_WriteCmd(0x80); // Clock divide / oscillator freq
	 SSD1306_WriteCmd(0xA8); SSD1306_WriteCmd(0x3F); // Multiplex ratio = 63 (64 dòng)
	 SSD1306_WriteCmd(0xD3); SSD1306_WriteCmd(0x00); // Display offset = 0
	 SSD1306_WriteCmd(0x40);             // Start line = 0
	 SSD1306_WriteCmd(0x8D); SSD1306_WriteCmd(0x14); // Bật charge pump (bắt buộc nếu không cấp VCC ngoài)
	 SSD1306_WriteCmd(0x20); SSD1306_WriteCmd(0x00); // Memory addressing mode = horizontal
	 SSD1306_WriteCmd(0xA1);             // Segment remap (lật ngang cho đúng chiều)
	 SSD1306_WriteCmd(0xC8);             // COM scan direction (lật dọc)
	 SSD1306_WriteCmd(0xDA); SSD1306_WriteCmd(0x12); // COM pins hardware config
	 SSD1306_WriteCmd(0x81); SSD1306_WriteCmd(0xCF); // Contrast
	 SSD1306_WriteCmd(0xD9); SSD1306_WriteCmd(0xF1); // Pre-charge period
	 SSD1306_WriteCmd(0xDB); SSD1306_WriteCmd(0x40); // VCOMH deselect level
	 SSD1306_WriteCmd(0xA4);             // Resume nội dung từ RAM (không phải test pattern)
	 SSD1306_WriteCmd(0xA6);             // Normal display (0xA7 để đảo màu)
	 SSD1306_WriteCmd(0xAF);             // Display ON
	 SSD1306_Clear();
	 SSD1306_UpdateScreen();
}

void SSD1306_Clear(void) {
    for (uint16_t i = 0; i < sizeof(SSD1306_Buffer); i++) SSD1306_Buffer[i] = 0x00;
    cursorX = 0;
    cursorY = 0;
}

void SSD1306_UpdateScreen(void) {
    for (uint8_t page = 0; page < SSD1306_HEIGHT / 8; page++) {
        SSD1306_WriteCmd(0xB0 + page); // Chọn page
        SSD1306_WriteCmd(0x00);        // Column start, nibble thấp
        SSD1306_WriteCmd(0x10);        // Column start, nibble cao
        SSD1306_WriteData(&SSD1306_Buffer[SSD1306_WIDTH * page], SSD1306_WIDTH);
    }
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, uint8_t color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;

    uint16_t idx = x + (y / 8) * SSD1306_WIDTH;
    if (color) SSD1306_Buffer[idx] |=  (uint8_t)(1U << (y % 8));
    else       SSD1306_Buffer[idx] &= (uint8_t)~(1U << (y % 8));
}

void SSD1306_SetCursor(uint8_t x, uint8_t y) {
    cursorX = x;
    cursorY = y;
}

void SSD1306_WriteChar(char ch) {
    if (ch < 32 || ch > 126) ch = ' ';
    const uint8_t *glyph = Font5x7[(uint8_t)ch - 32];

    for (uint8_t col = 0; col < 5; col++) {
        uint8_t line = glyph[col];
        for (uint8_t row = 0; row < 7; row++) {
            SSD1306_DrawPixel((uint8_t)(cursorX + col), (uint8_t)(cursorY + row),
                               (line >> row) & 0x01);
        }
    }

    cursorX = (uint8_t)(cursorX + 6); // 5 cột glyph + 1 cột trắng ngăn cách
    if (cursorX + 6 > SSD1306_WIDTH) {
        cursorX = 0;
        cursorY = (uint8_t)(cursorY + 8);
    }
}

void SSD1306_WriteString(const char *str) {
    while (*str) SSD1306_WriteChar(*str++);
}
