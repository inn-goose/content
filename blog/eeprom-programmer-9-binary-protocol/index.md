---
date: 2026-04-06
###
title: "EEPROM Programmer: Migrating from JSON-RPC to Binary Protocol"
###
description: "Replacing JSON-RPC + ArduinoJson with a binary serial protocol: 2.7x faster full write cycle, 4.4x faster reads, 31% less flash."
summary: "The EEPROM Programmer's serial protocol was migrated from JSON-RPC (with ArduinoJson dependency) to a lightweight binary protocol. On Arduino DUE + AT28C64: reads are 4.4x faster (6.96s to 1.58s), full write cycles are 2.7x faster (33.74s to 12.70s), and flash usage dropped 31% (16KB saved). The protocol uses length-prefixed frames with CRC-16/CCITT integrity checking, a state machine receiver on the firmware side, and blocking serial reads on the Python side. ArduinoJson — the only external firmware dependency — was eliminated entirely."
###
tags: [eeprom-programmer, performance, arduino]
series: ["EEPROM Programmer"]
series_order: 9
---


## TLDR

The EEPROM Programmer's serial protocol was migrated from JSON-RPC (with ArduinoJson dependency) to a lightweight binary protocol. On Arduino DUE + AT28C64: reads are **4.4x faster** (6.96s → 1.58s), full write cycles are **2.7x faster** (33.74s → 12.70s), and flash usage dropped **31%** (16KB saved). ArduinoJson — the only external firmware dependency — was eliminated entirely. The binary protocol uses length-prefixed frames with CRC-16/CCITT, a state machine receiver on the firmware side, and blocking serial reads on the Python side that eliminate the 50ms poll sleep from the JSON-RPC implementation.

{{< alert "fire" >}}
During read operations with the EEPROM Programmer, the chip's `!WE` pin **MUST** be connected to `VCC` using a jumper wire to disable the write mode. Otherwise, invoking the CLI may corrupt data on the chip due to Arduino's internal behavior. [Details](/blog/eeprom-programmer-5-data-corruption/)
{{< /alert >}}


## Motivation

The [EEPROM Programmer](https://github.com/inn-goose/eeprom-programmer) uses a serial connection between the Arduino board and a Python CLI to read and write EEPROM chips. The original implementation used [JSON-RPC 2.0](/blog/eeprom-programmer-4-serial-json-rpc-api/) over serial, backed by the [ArduinoJson](https://arduinojson.org/) library.

### What JSON-RPC Gave Us

- **Serial Monitor debugging**: type raw JSON in Arduino Serial Monitor to test wiring and diagnose issues — no tooling needed. Critical during early hardware development when the CLI didn't exist yet.
- **Self-describing errors**: `"Failed to init AT28C256 chip with error: 12"` vs an opcode. When debugging new chip support, readable error messages saved significant time.
- **Minimal client code**: the Python client was ~130 lines of `json.dumps`/`json.loads`. No protocol spec, no byte-level framing.
- **Development velocity**: adding a new RPC method = one string comparison in the handler + one `send_request` call in Python. The [blog series](/tags/eeprom-programmer/) used JSON-RPC in Serial Monitor as a teaching tool.

### Where It Hurts

Per-page cost breakdown (AT28C256 read, 64-byte pages):

| Cost | MEGA (164ms/page) | DUE (109ms/page) |
|---|---|---|
| ArduinoJson parse+serialize+heap | ~102ms (62%) | ~53ms (48%) |
| Serial wire (329 bytes at 115200) | ~29ms (17%) | ~29ms (26%) |
| Python 50ms poll sleep (avg) | ~25ms (15%) | ~25ms (23%) |
| GPIO (64 × digitalWrite/Read) | ~8ms (5%) | ~3ms (3%) |

The dominant cost is **ArduinoJson on the microcontroller**, not the serial wire. Each page requires heap-allocating `DynamicJsonDocument`s, building a `JsonArray` of 64 elements, serializing to decimal-string JSON, then `clear()` + `garbageCollect()`. On MEGA's 16MHz AVR with software heap, this is ~100ms per page. Increasing baud rate alone barely helps (921600 baud + 5ms poll: MEGA drops from 84s → 61s, only 1.4x).

Additional concerns:
- ArduinoJson v6 → v7 breaking change (`DynamicJsonDocument` removed entirely)
- ~6KB flash + ~1KB RAM consumed by ArduinoJson alone
- 350-byte receive buffer needed for JSON parsing
- ArduinoJson was the only external firmware dependency


## Measurements: Before

**Setup**: Arduino DUE + AT28C64 (8KB), ArduinoJson v7.4.2, 115200 baud

| Operation | Time |
|---|---|
| Read 8KB | 6.96s |
| Erase 8KB | 13.85s |
| Write only 8KB | 12.91s |
| Write + in-session verify | 19.88s (12.87 + 7.01) |
| **Full (erase + write + verify)** | **33.74s** (13.86 + 12.91 + 6.97) |

Firmware flash: 50,936 bytes (9% of DUE capacity).

Wire bytes per 64-byte page:
- READ_PAGE: request ~55B, response ~323B (total ~378B)
- WRITE_PAGE: request ~320B, response ~80B (total ~400B)

Per-byte timings (`--collect-performance`):
- AVG read: 31.34 μs/byte
- AVG write: 539.56 μs/byte (erase), 546.18 μs/byte (write)

> Note: measurements use in-session write+verify (no serial reconnect between operations) to avoid [data corruption from Arduino's reset behavior](/blog/eeprom-programmer-5-data-corruption/).

```
$ ./eeprom_programmer_cli/cli.py /dev/cu.usbmodem2101 -p AT28C64 --write test_bin/64_the_red_migration.bin
connect programmer: /dev/cu.usbmodem2101
programmer_settings: {'board_wiring_type': 28, 'max_page_size': 64}
connect programmer: DONE, 0.24 sec
init device: AT28C64
chip settings: {'memory_size': 8192}
init device: DONE
write operation: test_bin/64_the_red_migration.bin
erase operation
erase pattern: 0xFF
set_write_mode: WRITE mode is ON for 64 bytes pages
erase operation: DONE, 13.86 sec
write operation: started
set_write_mode: WRITE mode is ON for 64 bytes pages
write operation: DONE, 12.91 sec
verify operation: started
set_read_mode: READ mode is ON for 64 bytes pages
verify operation: DONE, 6.97 sec
```


## The Decision

### Options Considered

1. **Optimize JSON-RPC**: increase baud rate, reduce poll sleep — projections showed only ~1.4x improvement (MEGA: 84s → 61s). ArduinoJson CPU cost dominates, not wire time.
2. **Hybrid approach**: keep JSON-RPC for control commands (init_chip, set_mode — called once), binary only for bulk transfer (read_page, write_page — called hundreds of times). Reasonable, but still requires ArduinoJson for the control path.
3. **Full binary protocol**: eliminate ArduinoJson entirely. More work upfront, but removes the only external dependency and gives maximum performance.

### Why Full Binary

ArduinoJson is too heavy for a microcontroller that just needs to shuttle 64-byte pages between serial and GPIO. The library was designed for general-purpose JSON manipulation — heap allocation, tree traversal, polymorphic types — none of which this protocol needs. The v6 → v7 breaking change (`DynamicJsonDocument` removed, `JsonDocument` redesigned) forced a migration decision: update to v7 or replace entirely. Given that ArduinoJson accounted for 62% of per-page time and was the sole external dependency, replacement made more sense than update.


## Protocol Design

### Frame Format

```
[0xAA] [0x55] [LEN_L] [LEN_H] [BODY...] [CRC_L] [CRC_H]
```

- **Sync word**: `0xAA 0x55` — classic alternating-bit pattern, same approach as STK500v2 and AVR bootloaders
- **LEN**: uint16 LE — byte count of BODY only
- **BODY**: `CMD(1) | SEQ(1) | PAYLOAD(0..N)`
- **CRC-16/CCITT**: poly `0x1021`, init `0xFFFF`, computed over BODY bytes
- All multi-byte integers: **little-endian** (native on both AVR and ARM)

### Design Decisions

| Decision | Rationale |
|---|---|
| Length-prefix over COBS/SLIP | Zero encoding overhead on the 64-byte hot path |
| CRC-16/CCITT | Industry standard (HDLC, X.25, Bluetooth), catches all single/double-bit errors and burst errors ≤16 bits |
| Bit-by-bit CRC (no lookup table) | Saves 256 bytes flash, negligible speed difference at 115200 baud |
| SEQ byte | Detects stale responses after timeout/retry — prevents silent data corruption |
| Little-endian | Native on both AVR and ARM — zero-cost serialization |
| Single Serial.write() per frame | Full frame built in contiguous buffer, no flush() — avoids per-call overhead and separate USB packets |

### Commands

| CMD | Name | Direction | Payload |
|-----|------|-----------|---------|
| 0x01 | INIT_CHIP | req | chip_name (null-terminated) |
| 0x02 | SET_READ_MODE | req | page_size: uint16 |
| 0x03 | READ_PAGE | req | page_no: uint16 |
| 0x04 | SET_WRITE_MODE | req | page_size: uint16 |
| 0x05 | WRITE_PAGE | req | page_no: uint16 + data: uint8[N] |
| 0x06 | GET_READ_PERF | req | (none) |
| 0x07 | GET_WRITE_PERF | req | (none) |
| 0x81 | INIT_CHIP | resp | memory_size: uint32 |
| 0x82 | SET_READ_MODE | resp | (ACK, no payload) |
| 0x83 | READ_PAGE | resp | data: uint8[N] |
| 0x84 | SET_WRITE_MODE | resp | (ACK, no payload) |
| 0x85 | WRITE_PAGE | resp | (ACK, no payload) |
| 0x86 | GET_READ_PERF | resp | timings: uint16[N] |
| 0x87 | GET_WRITE_PERF | resp | timings: uint16[N] |
| 0xFE | BOOT | special | version: uint8, wiring_type: uint8, max_page_size: uint8 |
| 0xFF | ERROR | special | original_cmd: uint8, error_code: uint16, message: null-terminated |

### Wire Bytes Comparison

| Message | Binary | JSON-RPC | Reduction |
|---|---|---|---|
| READ_PAGE request | 10B | ~55B | 5.5x |
| READ_PAGE response (64B) | 72B | ~323B | 4.5x |
| WRITE_PAGE request (64B) | 74B | ~320B | 4.3x |
| WRITE_PAGE response | 8B | ~80B | 10x |


## Migration Strategy

The migration was done in incremental commits to keep diffs small and reviewable:

1. **Add firmware protocol handler** (`binary_protocol.h`): new file, included in `.ino` but not wired — purely additive, existing behavior unchanged
2. **Add Python protocol client** (`binary_protocol/client.py`): new file, not wired — purely additive
3. **Switchover**: rewire both firmware `.ino` and Python `eeprom_programmer_client.py` to use binary protocol — the only commit that changes behavior
4. **Delete old JSON-RPC files**: dead code removal

Commits 1-2 added all new code without changing any behavior. Commit 3 was the only risky one, but small because all new code was already in place and tested. Each step was compiled, uploaded, and validated on hardware before committing.


## Measurements: After

**Setup**: Arduino DUE + AT28C64 (8KB), binary protocol, 115200 baud

### Without `--collect-performance`

| Operation | Before | After | Speedup |
|---|---|---|---|
| Read 8KB | 6.96s | 1.58s | **4.4x** |
| Erase 8KB | 13.85s | 5.77s | **2.4x** |
| Write only 8KB | 12.91s | 5.35s | **2.4x** |
| **Full (erase + write + verify)** | **33.74s** | **12.70s** | **2.7x** |

### With `--collect-performance`

| Operation | Before | After | Speedup |
|---|---|---|---|
| Erase 8KB | 20.74s | 7.87s | **2.6x** |
| Write 8KB | 19.35s | 7.32s | **2.6x** |
| Verify (read) 8KB | 13.86s | 3.68s | **3.8x** |
| **Full cycle** | **53.95s** | **18.87s** | **2.9x** |

Per-byte timings are identical (same EEPROM, same GPIO):
- AVG read: 31.34 → 30.00 μs/byte
- AVG write: 539.56 → 539.03 μs/byte

Firmware flash: 50,936 → 34,936 bytes (**-31%**, 16KB saved). ArduinoJson dependency eliminated entirely.

### Why Not 10x?

The original 10x projection was for MEGA (ArduinoJson ~100ms/page). On DUE it was ~53ms/page — less overhead to remove. Write operations have a hardware floor: EEPROM write polling takes ~400-540μs/byte regardless of protocol. For 8KB that's ~3.3s minimum. The protocol improvement is ~4x on serial overhead, but the hardware floor masks it:

- **Read**: near-zero hardware time → total speedup shows full protocol improvement (4.4x)
- **Write**: ~3.3s hardware floor out of 5.35s → protocol improvement visible but diluted (2.4x)

```
$ ./eeprom_programmer_cli/cli.py /dev/cu.usbmodem2101 -p AT28C64 --write test_bin/64_the_red_migration.bin
connect programmer: /dev/cu.usbmodem2101
programmer_settings: {'board_wiring_type': 28, 'max_page_size': 64}
connect programmer: DONE, 0.22 sec
init device: AT28C64
chip settings: {'memory_size': 8192}
init device: DONE
write operation: test_bin/64_the_red_migration.bin
erase operation
erase pattern: 0xFF
set_write_mode: WRITE mode is ON for 64 bytes pages
erase operation: DONE, 5.77 sec
write operation: started
set_write_mode: WRITE mode is ON for 64 bytes pages
write operation: DONE, 5.35 sec
verify operation: started
set_read_mode: READ mode is ON for 64 bytes pages
verify operation: DONE, 1.58 sec
```


## Summary

The migration from JSON-RPC to a binary protocol delivered a 2.7x speedup on full write cycles (4.4x on reads), a 31% reduction in flash usage, and eliminated the only external firmware dependency. The protocol is 223 lines of C++ and 203 lines of Python. The incremental migration strategy — additive commits first, single behavioral switchover last — kept risk contained while allowing each component to be reviewed and tested in isolation.


## Appendix: Implementation Details

### Firmware: State Machine Receiver

The firmware reads serial data byte-by-byte and advances through a state machine. No heap allocation, no parsing — just state transitions and buffer fills:

```cpp
void loop() {
  while (Serial.available()) {
    uint8_t c = (uint8_t)Serial.read();

    switch (_state) {
      case WAIT_SYNC1:
        if (c == SYNC_BYTE_1) _state = WAIT_SYNC2;
        break;

      case WAIT_SYNC2:
        if (c == SYNC_BYTE_2) {
          _state = WAIT_LEN_L;
        } else if (c == SYNC_BYTE_1) {
          _state = WAIT_SYNC2;  // 0xAA again — could be start of new sync
        } else {
          _state = WAIT_SYNC1;
        }
        break;

      // ... LEN_L → LEN_H → BODY → CRC_L → CRC_H → dispatch
    }
  }
}
```

After CRC validation, the handler is called with `(cmd, seq, payload, payload_len)` — raw bytes, no deserialization.

### Firmware: Command Handler

Compare the binary handler to the [JSON-RPC version](/blog/eeprom-programmer-4-serial-json-rpc-api/). No string parsing, no JSON document allocation, no type conversion:

```cpp
void command_handler(uint8_t cmd, uint8_t seq,
                     const uint8_t* payload, uint16_t payload_len) {
  if (cmd == CMD_READ_PAGE) {
    uint16_t page_no = payload[0] | ((uint16_t)payload[1] << 8);

    uint8_t buffer[page_size];
    ErrorCode code = eeprom_programmer.read_page(page_no, buffer);

    binary_board.send_response(cmd, seq, buffer, page_size);
  }
  // ...
}
```

The entire frame — sync, length, body, CRC — is built in a single contiguous buffer and sent with one `Serial.write()` call:

```cpp
void _send_frame(uint8_t cmd, uint8_t seq,
                 const uint8_t* payload, uint16_t payload_len) {
  // header: sync + len
  _send_buf[0] = SYNC_BYTE_1;
  _send_buf[1] = SYNC_BYTE_2;
  _send_buf[2] = (uint8_t)(body_len & 0xFF);
  _send_buf[3] = (uint8_t)(body_len >> 8);

  // body: cmd + seq + payload (memcpy)
  // trailer: crc over body

  Serial.write(_send_buf, frame_len);  // single call, no flush()
}
```

### Python: Frame Parsing

The Python client uses blocking `serial.read(n)` with timeout — eliminating the 50ms poll sleep from the JSON-RPC implementation. The sync detection mirrors the firmware's state machine:

```python
def _read_frame(self, timeout):
    # sync — state machine matching firmware
    state = 'SYNC1'
    while True:
        b = self.serial.read(1)
        if len(b) == 0:
            return None, None, None  # timeout
        if state == 'SYNC1':
            if b[0] == SYNC_BYTE_1:
                state = 'SYNC2'
        elif state == 'SYNC2':
            if b[0] == SYNC_BYTE_2:
                break  # synced
            elif b[0] != SYNC_BYTE_1:
                state = 'SYNC1'
            # else: another 0xAA, stay in SYNC2

    # length, body, crc — all blocking reads
    body = self.serial.read(body_len)
    # ...CRC check...
    return cmd, seq, payload
```

### CRC-16/CCITT

Both sides use identical CRC-16/CCITT implementations (poly `0x1021`, init `0xFFFF`), bit-by-bit without a lookup table. The standard test vector `"123456789"` produces `0x29B1`.

```cpp
// Firmware (C++)
static uint16_t crc16(const uint8_t* data, uint16_t len) {
  uint16_t crc = 0xFFFF;
  for (uint16_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t bit = 0; bit < 8; bit++) {
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
    }
  }
  return crc;
}
```

```python
# Python
def crc16(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) if (crc & 0x8000) else (crc << 1)
            crc &= 0xFFFF
    return crc
```
