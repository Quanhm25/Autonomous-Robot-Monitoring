#include "i2c1.h"

// Giao thức I2C, với hai dây SDA (dữ liệu) và SCL (clock do Master tạo) kết nối giữa ngoại vi làm slave và STM32 làm Master

#define I2C1_TIMEOUT 20000

// Hàm khởi tạo driver giao tiếp I2C1 trực tiếp mức thanh ghi
// Khi gọi làm thì STM32 thực hiện:
// - Bật CLK GPIOB
// - Bật CLK I2C1
// - Cấu hình PB6/PB7
// - Chọn chế độ thay thế (Alternate Function)
// - Cấu hình tốc độ I2C
// - Bật I2C1
void I2C1_Init(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN; // Cấp clock cho GPIOB (RCC = Reset and Clock Control)
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // Cấp clock cho I2C1 nằm trên APB1

	GPIOB->MODER &= ~((3<<12) | (3<<14)); // Chỉnh 2 bit chế độ của GPIOB 6 và GPIOB 7 về mức 00 (reset state)
	GPIOB->MODER |= ((2<<12) | (2<<14)); // Chỉnh 2 bit chế độ của GPIOB 6 và GPIO 7 về mức 10 (alternate function)
	GPIOB->OTYPER |= (1<<6) | (1<<7); // Cấu hình open-drain
	GPIOB->OSPEEDR |= (3<<12) | (3<<14); // Cấu hình tốc độ GPIO ở mức 11 - very high (cấu hình khả năng chuyển mức logic của GPIO)

	GPIOB->AFR[0] &= ~((0xF<<24) | (0xF<<28));  // GPIO 0-7 nằm trên AFR[0], mà mỗi pin Alternate Function có 4 bit. Ta có 0xF = 1111, vậy nên dòng này có thể hiểu là dòng xóa cấu hình AF của PB6 và PB7
	GPIOB->AFR[0] |= ((0x4<<24) | (0x4<<28)); // Theo datasheet, 0x4 là AF4 mà STM32F411RET6 ánh xạ I2C1 vào AF4. Sau khi code dòng này, PB6 trở thành I2C1_SCL và PB7 trở thành I2C1_SDA

	I2C1->CR1 |= I2C_CR1_SWRST; // Control Register của I2C1 thực hiện tắt I2C1 trước khi cấu hình (~Peripheral Enable)
	I2C1->CR1 &= ~I2C_CR1_SWRST;
	I2C1->CR2 = 42; // Cấu hình Control Register 2, dòng này liên quan đến tần số clock APB1 (CR2.FREQ)
	I2C1->CCR = 210; // CCR = Clock Control Register, dòng này sử dụng để thiết lập thời gian của SCL. (Đang để ở mức 100kHz)
	I2C1->TRISE = 43; // TRISE = FREQ(MHz) + 1
	I2C1->CR1 |= I2C_CR1_PE; // Thực hiện bật I2C
}

// Đây là hàm chờ một flag xuất hiện và chỉ sử dụng được trong file hiện tại
// uint8_t trả về 1 khi thành công và 0 khi timeout
// volatile uint32_t *reg là con trỏ đến thanh ghi
static uint8_t I2C1_WaitFlag(volatile uint32_t *reg, uint32_t flag) {
	uint32_t timeout = 20000; // Giới hạn số vòng lặp
	while(!(*reg & flag)) { // Trong khi flag chưa được set thì tiếp tục chờ
		if(--timeout == 0) return 0; // Nếu hàm đợi quá lâu thì return 0
	}
	return 1; // Không thỏa mãn các điều kiện trên thì trả về 1
}

static uint8_t I2C1_Start(uint8_t addr, uint8_t readDir) {
	I2C1->CR1|= I2C_CR1_START;
	if(!I2C1_WaitFlag(&I2C1->SR1, I2C_SR1_SB)) return 0;
	(void)I2C1->SR1;

	I2C1->DR = (uint8_t)((addr << 1) | readDir);
	if(!I2C1_WaitFlag(&I2C1->SR1, I2C_SR1_ADDR)) return 0;
	(void) I2C1->SR1;
	(void) I2C1->SR2;
	return 1;
}

static void I2C1_Stop(void) {
	I2C1->CR1 |= I2C_CR1_STOP;
}

uint8_t I2C1_WriteBytes(uint8_t addr, uint8_t *data, uint8_t len) { // addr: địa chỉ slave; *data: con trỏ đến dữ liệu cần gửi; len: số byte cần gửi
    if(!I2C1_Start(addr, 0)) {
    	I2C1_Stop();
    	return 0;
    }

    for(uint16_t i = 0; i < len; i++) {
    	if(!I2C1_WaitFlag(&I2C1->SR1, I2C_SR1_TXE)) {
    		I2C1_Stop();
    		return 0;
    	}
    	I2C1->DR = data[i];
    }

    if(!I2C1_WaitFlag(&I2C1->SR1, I2C_SR1_BTF)) {
    	I2C1_Stop();
    	return 0;
    }

    I2C1_Stop();
    return 1;
}

uint8_t I2C1_WriteReg(uint8_t addr, uint8_t reg, uint8_t data) {
	uint8_t buf[2] = {reg, data};
	return I2C1_WriteBytes(addr, buf, 2);
}

uint8_t I2C1_ReadBytes(uint8_t addr, uint8_t *buf, uint8_t len) {
	if(len == 0) return 0;

	I2C1->CR1 |= I2C_CR1_ACK;
	if(!I2C1_Start(addr, 1)) {
		I2C1_Stop();
		return 0;
	}

	for(uint16_t i = 0; i < len; i++) {
		if(i == len - 1) {
			I2C1->CR1 &= ~I2C_CR1_ACK;
			I2C1_Stop();
		}
		if(!I2C1_WaitFlag(&I2C1->SR1, I2C_SR1_RXNE)) return 0;
		buf[i] = (uint8_t)I2C1->DR;
	}
	return 1;
}
