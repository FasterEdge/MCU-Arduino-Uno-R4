// ─────────────────────────────────────────────────────────────
// FasterEdge 开源项目
// Github: https://github.com/FasterEdge
// Gitee:  https://gitee.com/FasterEdge
// ─────────────────────────────────────────────────────────────
// fe_port_arduino.cpp — Arduino 核心库桥接（Arduino Uno R4 / RA4M1 版）
// 以 extern "C" 导出 C 可调用的平台原语，供 fe_port.c 使用。
#include <Arduino.h>
#include <EEPROM.h>
#include <stdint.h>

extern "C" {

void fe_arduino_uart_init(void) { Serial.begin(115200); }

uint16_t fe_arduino_uart_write(const uint8_t *data, uint16_t len) {
    Serial.write(data, len);
    return len;
}

uint8_t fe_arduino_uart_available(void) { return Serial.available() ? 1 : 0; }

int fe_arduino_uart_read(void) { return Serial.read(); }

void fe_arduino_uart_close(void) { Serial.end(); }

uint8_t fe_arduino_eeprom_read(uint16_t addr) { return EEPROM.read(addr); }

void fe_arduino_eeprom_write(uint16_t addr, uint8_t val) { EEPROM.write(addr, val); }

void fe_arduino_eeprom_begin(uint16_t sz) { EEPROM.begin(sz); }

uint32_t fe_arduino_millis(void) { return millis(); }

uint32_t fe_arduino_random(void) { return (uint32_t)random(0x7FFFFFFF); }

int fe_arduino_gpio_set_mode(uint8_t pin, const char *mode) {
    if (strcmp(mode, "input") == 0) pinMode(pin, INPUT);
    else if (strcmp(mode, "input_pullup") == 0) pinMode(pin, INPUT_PULLUP);
    else if (strcmp(mode, "output") == 0) pinMode(pin, OUTPUT);
    else return -1;
    return 0;
}

int fe_arduino_gpio_write(uint8_t pin, uint8_t level) { digitalWrite(pin, level); return 0; }

int fe_arduino_gpio_read(uint8_t pin) { return digitalRead(pin); }

void fe_arduino_delay_ms(uint32_t ms) { delay(ms); }

} // extern "C"
