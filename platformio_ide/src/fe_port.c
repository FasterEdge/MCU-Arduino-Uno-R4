// fe_port.c — FasterEdge MCU 平台移植层实现（Arduino Uno R4 / RA4M1 版）
// 真实实现：通过 Arduino 核心库桥接（fe_port_arduino.cpp 提供 extern "C" API）。
//   UART0  : Serial（USB-CDC）
//   EEPROM : EEPROM 库（模拟 8KB）
//   GPIO   : pinMode/digitalWrite/digitalRead（Arduino API）
//   time   : millis()
#include "fe_port.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================
// 格式化输出（委托标准 vsnprintf）
// ============================================================
int fe_snprintf(char *buf, u16 size, const char *fmt, ...) {
    va_list ap;
    int n;
    va_start(ap, fmt);
    n = vsnprintf(buf, size, fmt, ap);
    va_end(ap);
    if (n < 0) { buf[0] = 0; return 0; }
    if ((u16)n >= size) buf[size - 1] = 0;
    return n;
}

// ============================================================
// 串口（桥接到 Arduino Serial）
// ============================================================
// 桥接函数声明（在 fe_port_arduino.cpp 中实现）
extern void fe_arduino_uart_init(void);
extern u16 fe_arduino_uart_write(const u8 *data, u16 len);
extern u8  fe_arduino_uart_available(void);
extern int fe_arduino_uart_read(void);
extern void fe_arduino_uart_close(void);

void fe_port_uart_init(u8 port, u32 baud, fe_port_uart_rx_cb_t rx_cb, void *user) {
    (void)port; (void)baud; (void)rx_cb; (void)user;
    fe_arduino_uart_init();
}

u16 fe_port_uart_write(u8 port, const u8 *data, u16 len) {
    (void)port;
    return fe_arduino_uart_write(data, len);
}

u8 fe_port_uart_available(u8 port) {
    (void)port;
    return fe_arduino_uart_available();
}

int fe_port_uart_read(u8 port) {
    (void)port;
    return fe_arduino_uart_read();
}

void fe_port_uart_close(u8 port) {
    (void)port;
    fe_arduino_uart_close();
}

// ============================================================
// EEPROM（桥接到 Arduino EEPROM 库）
// ============================================================
extern u8 fe_arduino_eeprom_read(u16 addr);
extern void fe_arduino_eeprom_write(u16 addr, u8 val);
extern void fe_arduino_eeprom_begin(u16 sz);
#define EEPROM_SIZE 8192

u8 fe_port_eeprom_get_str(u16 addr, char *out, u16 outlen) {
    u16 i;
    if (addr + 1 > EEPROM_SIZE || outlen == 0) return FALSE;
    for (i = 0; i + 1 < outlen; i++) {
        u8 c = fe_arduino_eeprom_read(addr + i);
        out[i] = (char)c;
        if (c == 0) break;
    }
    out[i] = 0;
    return TRUE;
}

u8 fe_port_eeprom_set_str(u16 addr, const char *value) {
    u16 i;
    u16 n = (u16)strlen(value) + 1;
    if (addr + n > EEPROM_SIZE) return FALSE;
    for (i = 0; i < n; i++) fe_arduino_eeprom_write(addr + i, (u8)value[i]);
    return TRUE;
}

u8 fe_port_eeprom_get_u32(u16 addr, u32 *out) {
    if (addr + 4 > EEPROM_SIZE) return FALSE;
    *out = (u32)fe_arduino_eeprom_read(addr)
         | ((u32)fe_arduino_eeprom_read(addr+1) << 8)
         | ((u32)fe_arduino_eeprom_read(addr+2) << 16)
         | ((u32)fe_arduino_eeprom_read(addr+3) << 24);
    return TRUE;
}

u8 fe_port_eeprom_set_u32(u16 addr, u32 value) {
    u8 i;
    if (addr + 4 > EEPROM_SIZE) return FALSE;
    for (i = 0; i < 4; i++) fe_arduino_eeprom_write(addr + i, (u8)(value >> (i * 8)));
    return TRUE;
}

// ============================================================
// 系统时间（桥接到 Arduino millis）
// ============================================================
extern u32 fe_arduino_millis(void);
static u32 s_epoch = 0;

u32 fe_port_time_now(void) {
    return s_epoch + fe_arduino_millis() / 1000UL;
}

void fe_port_time_set(u32 epoch) {
    s_epoch = epoch;
}

// ============================================================
// 随机数（桥接到 Arduino random）
// ============================================================
extern u32 fe_arduino_random(void);

void fe_port_random_fill(u8 *buf, u16 len) {
    u16 i;
    for (i = 0; i < len; i++) buf[i] = (u8)(fe_arduino_random() >> 24);
}

// ============================================================
// GPIO（桥接到 Arduino pinMode/digitalWrite/digitalRead）
// ============================================================
extern int fe_arduino_gpio_set_mode(u8 pin, const char *mode);
extern int fe_arduino_gpio_write(u8 pin, u8 level);
extern int fe_arduino_gpio_read(u8 pin);

int fe_port_gpio_set_mode(u8 pin, const char *mode) {
    return fe_arduino_gpio_set_mode(pin, mode);
}

int fe_port_gpio_write(u8 pin, u8 level) {
    return fe_arduino_gpio_write(pin, level);
}

int fe_port_gpio_read(u8 pin) {
    return fe_arduino_gpio_read(pin);
}

// ============================================================
// 芯片信息
// ============================================================
void fe_port_chip_info(char *out, u16 outlen) {
    fe_snprintf(out, outlen,
                "{\"chip\":\"RA4M1\",\"arch\":\"Cortex-M4F\","
                "\"ramBytes\":32768,\"flashBytes\":262144,\"eepromBytes\":8192,"
                "\"freqMHz\":48}");
}

// ============================================================
// 延时（桥接到 Arduino delay）
// ============================================================
extern void fe_arduino_delay_ms(u32 ms);

void fe_port_delay_ms(u32 ms) {
    fe_arduino_delay_ms(ms);
}
