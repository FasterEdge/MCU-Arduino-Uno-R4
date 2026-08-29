<div align="center">
<img src="https://avatars.githubusercontent.com/u/245985800?s=200&v=4" style="width:100px;" width="100"/>
<h2>FasterEdge MCU - Arduino Uno R4 (RA4M1)</h2>
<h3>FasterEdge framework on Arduino Uno R4 (Arduino / PlatformIO editions)</h3>
</div>

### 1. Introduction

This repo implements the **[FasterEdge](https://github.com/FasterEdge/FasterEdge)** framework on the **Arduino Uno R4 (RA4M1)**. The RA4M1 is a 32-bit Cortex-M4F core with 32KB SRAM, 256KB Flash and 8KB (emulated) EEPROM — no network, no OS. Following the [MCU-C51](../MCU-C51) no-network design, the capability set is trimmed and 3 **MCU-specific** modules (registers / GPIO / chip info) are kept.

- ✅ **arduino/ (C++, Arduino framework)** + **platformio_ide/ (pure C, Arduino-API bridge drivers)** dual editions
- ✅ Same names & commands as the main repo — peer programming for edge/cloud
- ✅ HMAC-SHA256 in pure C (zero dependencies)
- ✅ Config/keys persisted to 8KB EEPROM (emulated)
- ✅ platformio_ide edition ships **real drivers via Arduino-core bridge** (Serial / EEPROM / GPIO / millis)

### 2. Implemented Capabilities (no-network subset)

**Abilities (8)**

| Name | Type | Commands |
|------|------|----------|
| `BaseAbility` | Base | `list_data_names` / `list_ability_names` |
| `RoleAbility` | Role | `describe` / `set_role` / `get_role` |
| `TimeAbility` | Time | `sync_manual` / `sync_system` / `get_time` / `configure_run` (no NTP) |
| `OneKeyAbility` | Token | `issue_token` / `verify_token` / `revoke_all` / `list_tokens` / `status` / `rotate` (HMAC-SHA256) |
| `SerialAbility` | Serial | `open` / `close` / `write` / `read` / `is_open` / `set_config` / `get_config` / `list_ports` |
| `ModbusAbility` | Modbus | `set_unit_id` / `get_unit_id` / `read_holding` / `read_input` / `read_coils` / `read_discrete` / `write_holding` / `write_coil` (RTU slave) |
| `RegAbility` | Reg (own) | `read <addr>` / `write <addr>,<value>` / `bit_set <addr>,<bit>` / `bit_clear <addr>,<bit>` / `info` |
| `GpioAbility` | GPIO (own) | `mode <pin>,<input|output|input_pullup>` / `write <pin>,<0|1>` / `read <pin>` / `info` |

**Data (3)**

| Name | Type | Commands |
|------|------|----------|
| `BaseData` | Meta | `logo` / `info` |
| `ConfigData` | KV config (EEPROM) | `get` / `set` / `delete` / `list` / `snapshot` |
| `ChipData` | Chip info (own) | `info` |

### 3. Excluded Capabilities

| Capability | Reason |
|------------|--------|
| MQTTAbility / NetMapData | No network stack on RA4M1 |
| EdgeRoleAbility | Needs network heartbeat |
| ConfigFileAbility | Redundant with ConfigData; no filesystem concept |
| KeyringData | Merged into OneKeyAbility (same EEPROM key) |
| TimeAbility.sync_ntp | No network for SNTP |

### 4. Directory Layout

```
MCU-Arduino-Uno-R4/
├── arduino/                    # Arduino C++ edition (Arduino framework)
│   ├── include/                # fe.h / fe_ability.h / fe_data.h / fe_hmac_sha256.h
│   ├── src/                    # fe.cpp / main.cpp / register.cpp / ability_*.cpp / data_*.cpp
│   └── platformio.ini          # board = uno_r4_minima (renesas-ra + arduino framework)
└── platformio_ide/             # VS Code + PlatformIO IDE project (pure C register-level)
    ├── platformio.ini          # board = uno_r4_minima (renesas-ra + arduino framework)
    ├── .vscode/extensions.json # recommends PlatformIO IDE
    ├── include/                # fe.h / fe_ability.h / fe_data.h / fe_port.h / fe_hmac_sha256.h
    └── src/                    # pure-C logic + fe_port.c / fe_port_arduino.cpp (Arduino API bridge)
```

> `arduino/` and `platformio_ide/` expose identical capabilities & commands; the former is for quick start, the latter demonstrates register-level bare-metal drivers and eases porting to other AVRs (Nano/Mega).

### 5. Usage

1. **arduino edition**: open `arduino/src/main.cpp` in Arduino IDE (board: Arduino Uno), or open `arduino/` in VS Code + PlatformIO; flash and monitor at 115200.
2. **platformio_ide edition**: install the **PlatformIO IDE** VS Code extension, open `platformio_ide/`, use Build / Upload / Serial Monitor from the status bar.
3. No porting needed — `fe_port.c` already ships real AVR register-level drivers.

**Serial command examples:**

```
help
ability_BaseAbility list_ability_names
ability_RoleAbility set_role edge
ability_TimeAbility sync_manual 1700000000
ability_OneKeyAbility issue_token sensor01
ability_ModbusAbility set_unit_id 3
ability_ModbusAbility write_holding 0,42
ability_ModbusAbility read_holding 0,4
ability_SerialAbility set_config 0,9600
ability_SerialAbility write hello
data_ConfigData set wifi.ssid=MyNet
data_ConfigData get wifi.ssid
data_BaseData info
```

### 6. Platform Differences

| Aspect | ESP32/ESP8266 | Uno R4 (RA4M1) |
|--------|---------------|---------------------|
| Architecture | Xtensa 32-bit | **Cortex-M4F 32-bit** |
| RAM / Flash | KB~MB | **32KB / 256KB** |
| Storage | NVS / Flash | **8KB EEPROM (emulated)** |
| Network | Yes | **No** (network items trimmed) |
| Registers | 32-bit MMIO | **32-bit peripheral space 0x40000000+** (RegAbility width 32) |

### 6-b. platformio_ide Notes (AVR register-level)

The `platformio_ide/` edition keeps pure-C business code, with `fe_port_arduino.cpp` bridging (extern "C") to the Arduino core:

| Function | Implementation |
|----------|----------------|
| UART | **Serial** (USB-CDC, UART0) |
| EEPROM | **EEPROM library** (8KB, emulated) |
| Time | **millis()** |
| GPIO | **pinMode / digitalWrite / digitalRead** (pins 0-19) |
| Random | **random()** |

```bash
cd platformio_ide
pio run            # build
pio run -t upload  # flash
pio device monitor # serial monitor (115200)
```

> To change MCU: edit `board` in `platformio.ini` (e.g. `uno_r4_wifi`); `fe_port.c` needs no change (bridged from Arduino API).

### 6-c. MCU-Specific Modules

Beyond main-repo capabilities, 3 **MCU-specific** modules (registers / GPIO / chip info) are provided. The R4 registers are **32-bit RA4M1 peripheral space** (0x40000000+); GPIO uses Arduino pin numbers:

| Module | Type | Commands | Description |
|--------|------|----------|-------------|
| RegAbility | Ability | `read <addr>` / `write <addr>,<value>` / `bit_set <addr>,<bit>` / `bit_clear <addr>,<bit>` / `info` | RA4M1 MMIO registers (32-bit, volatile pointer) |
| GpioAbility | Ability | `mode <pin>,<input|output|input_pullup>` / `write <pin>,<0|1>` / `read <pin>` / `info` | Arduino pin GPIO (pin 0-19) |
| ChipData | Data | `info` | RA4M1 model / RAM / Flash / EEPROM / freq |

**Examples:**

```
ability_RegAbility read 0x40000000      # read peripheral reg
ability_RegAbility write 0x40000000,0x12345678
ability_RegAbility bit_set 0x40000000,7
ability_GpioAbility mode 13,output    # onboard LED
ability_GpioAbility write 13,1
ability_GpioAbility read 2
data_ChipData info
```

> ⚠️ Register access touches hardware directly; a wrong write may crash the system. Debug/low-level use only.

### 7. Correspondence with the Main Repo

- Commands match the main repo exactly, and the implementation is isomorphic with MCU-C51 / MCU-ESP32.
- `Atom` model: singleton global Atom, `data_` / `ability_` prefix routing.
- Tokens via HMAC-SHA256 (pure C, no mbedTLS), key persisted in EEPROM.
- Modbus register tables live in RAM; RTU entry `modbus_slave_service()` is reserved.

### 8. Sibling Projects

- **[FasterEdge MCU - ESP32](https://github.com/FasterEdge/MCU-ESP32)**: dual-core, WiFi/BLE, more peripherals
- **[FasterEdge MCU - ESP8266](https://github.com/FasterEdge/MCU-ESP8266)**: WiFi, low power
- **[FasterEdge MCU - C51](https://github.com/FasterEdge/MCU-C51)**: 8-bit 8051, most minimal
- **[FasterEdge MCU - Arduino Uno R3](https://github.com/FasterEdge/MCU-Arduino-Uno-R3)**: 8-bit AVR (ATmega328P)
- **[FasterEdge](https://github.com/FasterEdge/FasterEdge)**: framework main repo
