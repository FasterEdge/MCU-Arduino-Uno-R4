// FasterEdge 开源项目 - Github: https://github.com/FasterEdge - Gitee: https://gitee.com/FasterEdge
// ability_reg.cpp — RegAbility 实现（Arduino Uno R4 / RA4M1 版，MCU 专有）
// MCU 专有能力：内存映射寄存器读写（8 位，RA4M1 外设空间 0x40000000+）。
#include "fe_ability.h"
namespace fe {
static uint8_t hexVal(char c) {
    if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (uint8_t)(c - 'A' + 10);
    return 0xFF;
}
static bool parseHex(const String &s, uint32_t &val) {
    val = 0; uint8_t d;
    size_t i = 0;
    if (s.startsWith("0x") || s.startsWith("0X")) i = 2;
    for (; i < s.length(); i++) {
        d = hexVal(s.charAt(i));
        if (d == 0xFF) return false;
        val = (val << 4) | d;
    }
    return true;
}
CommandOutput regAbilityDispatch(void *inst, const char *act, const String &args) {
    (void)inst;
    if (strcmp(act, "read") == 0) {
        uint32_t addr;
        if (!parseHex(args, addr))
            return CommandOutput{String(act), String(), String("bad address")};
        uint32_t v = *(volatile uint32_t *)(uintptr_t)addr;
        char buf[64];
        snprintf(buf, sizeof(buf), "{\"addr\":\"0x%08X\",\"value\":%lu,\"hex\":\"0x%08lX\"}",
                 (unsigned)addr, (unsigned long)v, (unsigned long)v);
        return CommandOutput{String(act), String(buf), String()};
    }
    if (strcmp(act, "write") == 0) {
        int comma = args.indexOf(',');
        if (comma <= 0) return CommandOutput{String(act), String(), String("bad format, expect addr,val")};
        uint32_t addr, val;
        if (!parseHex(args.substring(0, comma), addr))
            return CommandOutput{String(act), String(), String("bad address")};
        if (!parseHex(args.substring(comma + 1), val))
            return CommandOutput{String(act), String(), String("bad value")};
        *(volatile uint32_t *)(uintptr_t)addr = val;
        char buf[64];
        snprintf(buf, sizeof(buf), "{\"addr\":\"0x%08X\",\"value\":%lu,\"hex\":\"0x%08lX\"}",
                 (unsigned)addr, (unsigned long)val, (unsigned long)val);
        return CommandOutput{String(act), String(buf), String()};
    }
    if (strcmp(act, "bit_set") == 0) {
        int comma = args.indexOf(',');
        if (comma <= 0) return CommandOutput{String(act), String(), String("bad format, expect addr,bit")};
        uint32_t addr, bit;
        if (!parseHex(args.substring(0, comma), addr))
            return CommandOutput{String(act), String(), String("bad address")};
        bit = args.substring(comma + 1).toInt();
        if (bit > 31) return CommandOutput{String(act), String(), String("bit must be 0..31")};
        uint32_t v = *(volatile uint32_t *)(uintptr_t)addr;
        v |= (1ul << bit);
        *(volatile uint32_t *)(uintptr_t)addr = v;
        char buf[64];
        snprintf(buf, sizeof(buf), "{\"addr\":\"0x%08X\",\"bit\":%lu,\"value\":%lu}",
                 (unsigned)addr, (unsigned long)bit, (unsigned long)v);
        return CommandOutput{String(act), String(buf), String()};
    }
    if (strcmp(act, "bit_clear") == 0) {
        int comma = args.indexOf(',');
        if (comma <= 0) return CommandOutput{String(act), String(), String("bad format, expect addr,bit")};
        uint32_t addr, bit;
        if (!parseHex(args.substring(0, comma), addr))
            return CommandOutput{String(act), String(), String("bad address")};
        bit = args.substring(comma + 1).toInt();
        if (bit > 31) return CommandOutput{String(act), String(), String("bit must be 0..31")};
        uint32_t v = *(volatile uint32_t *)(uintptr_t)addr;
        v &= ~(1ul << bit);
        *(volatile uint32_t *)(uintptr_t)addr = v;
        char buf[64];
        snprintf(buf, sizeof(buf), "{\"addr\":\"0x%08X\",\"bit\":%lu,\"value\":%lu}",
                 (unsigned)addr, (unsigned long)bit, (unsigned long)v);
        return CommandOutput{String(act), String(buf), String()};
    }
    if (strcmp(act, "info") == 0) {
        return CommandOutput{String(act),
            String("{\"ability\":\"RegAbility\",\"desc\":\"RA4M1 32 位寄存器\",\"addr\":\"0x40000000+\"}"), String()};
    }
    return CommandOutput{String(act), String(), String("unsupported command: ") + act};
}
} // namespace fe
