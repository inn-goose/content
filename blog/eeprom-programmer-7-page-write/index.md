---
date: 2025-12-22
###
title: "EEPROM Programmer: AT28C256's Page Write Operation"
###
description: "AT28C256 page writes are timing-critical: they fail on Arduino Mega but work reliably on faster platforms like Arduino Due."
summary: "Page-write support in the AT28C256 can significantly speed up programming, but it depends on tight timing. The Arduino Mega is too slow to use this feature reliably, while faster platforms like the Arduino Due make it practical and stable."
###
tags: [eeprom-programmer, datasheet, performance, arduino]
---

## TLDR

[EEPROM Programmer on GitHub](https://github.com/inn-goose/eeprom-programmer)

The **AT28C256** chip has a feature I had not encountered in other devices from this family: it supports a **Page Write mode**, which can potentially reduce the time required to program the entire memory by an order of magnitude.

Based on my reading of the [AT28C256 datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/doc0006.pdf), the chip includes an internal high-speed volatile buffer. Writes to this buffer do not require `DATA` polling, which makes it possible to accumulate 64 bits of data before committing them to non-volatile storage in a single operation.

In general, write speed is the primary bottleneck of EEPROM devices. When programming the AT28C256 byte by byte, each write—followed by polling for completion—can take up to 6 ms per byte. Part of this can be attributed to limitations of the EEPROM programmer platform itself, but with a total capacity of 256K, these delays quickly become excessive. In practice, programming the entire chip using an Arduino MEGA takes more than five minutes.

In this article, I describe the constraints I ran into while implementing this feature in an EEPROM programmer, and explain why this approach does not work on the Arduino MEGA.


## DATA Polling

I covered the details of how it works in a separate [DATA Polling](/blog/eeprom-programmer-6-data-polling-vs-rdy-busy/#data-polling) post; here I will only note that each polling cycle requires 10 `digitalWrite` operations, which are relatively expensive in terms of execution time. Multiple polling cycles are typically needed for a single write—on average, around 5 cycles per byte.

As a result, writing each byte individually—and then verifying the integrity of each byte individually—consumes a significant amount of resources and, in practice, a great deal of time. The page-write approach changes this dynamic: it allows integrity checking to be performed only for the last byte written out of a 64-byte page. This reduces the number of `digitalWrite` operations by roughly 64 * 10 * 5 per 64B page.


## How Page Write Works

![Page Mode Write Waveforms](images/page-mode-write-waveforms.png)

I will allow myself to quote the AT28C256 datasheet directly:

> The page write operation of the AT28C256 allows 1 to 64 bytes of data to be written into the device during a single internal programming period. A page write operation is initiated in the same manner as a byte write; the first byte written can then be followed by 1 to 63 additional bytes. Each successive byte must be written within 150 μs (tBLC) of the previous byte. If the tBLC limit is exceeded the AT28C256 will cease accepting data and commence the internal program- ming operation. All bytes during a page write operation must reside on the same page as defined by the state of the A6 - A14 inputs.

This excerpt contains several important conditions that must be satisfied:

(1) The size of a single page should be limited to 64 bytes. This constraint is already met, as it is imposed by the `Serial JSON-RPC` protocol and by the memory limitations of the Arduino MEGA. I address the limitations of the JSON-RPC protocol in a separate article here: [JSON-RPC Protocol Limitations](/blog/project-2-serial-json-rpc-arduino/#json-rpc-protocol-limitations)

(2) All bytes must be written within the same page. A page is defined by constant values on the address bus lines A6–A14. This condition is also satisfied: the EEPROM Programmer emulates page-based writes by default, even though it internally performs byte-wise writes. As a result, no changes are required here either.

(3) Each byte within a page must be written within a 150 µs window. This requirement is the one I will focus on in the next section.

(4) The datasheet additionally notes that per-byte integrity checking is not required. Successful completion of a page write can be determined by verifying the correctness of the last byte written.

As a result, the core logic of the EEPROM Programmer requires only minimal changes. Instead of verifying every byte within a page, it is sufficient to verify only the last byte. All other aspects of the write process, including data writes and DATA polling, remain unchanged.


## 150 µs and Arduino MEGA

I examined the performance characteristics of various Arduino platforms in the [Oscilloscope Analysis of Arduino Pin Toggle Performance](/blog/experiments-1-arduino-performance/) post. This was the first time I encountered a situation where the low clock frequency of the microcontroller prevented me from implementing a hardware feature of the chip.

The execution speed of `digitalRead` and `digitalWrite` operations is directly dependent on the microcontroller’s clock frequency. Writing a single byte to EEPROM is a fairly complex operation and requires several dozen low-level `digital*` calls.

In practice, writing a single data byte on the Arduino MEGA takes on the order of 400 µs, even without accounting for DATA polling. As a result, this exceeds the 150 µs timing window specified in the datasheet, making page-write operations effectively impossible on this platform. Only switching to the Arduino DUE, with a clock frequency of 85 MHz, makes it possible to reduce the write time of a single data byte to approximately 110 µs and stay within the required 150 µs window.

file: `./eeprom_programmer/eeprom_programmer_lib.h`
```
    case ChipType::AT28C256:
      // doesn't work on Arduino MEGA
      // enable if use Arduino DUE only
      _can_write_pages = true;
      break;
```

Enabling page-write support is controlled by the `_can_write_pages` flag. If the programmer is compiled with page mode enabled and then run on the Arduino MEGA, the resulting contents of the EEPROM will be corrupted. Every 64th byte will be correct—since it is explicitly verified—while all other bytes will contain effectively random data.

On the Arduino DUE, this mode operates reliably. I tested it under different conditions on multiple AT28C256 chips. That said, full data verification is still recommended.


## Performance of the Page Write Mode

In byte-write mode, the write speed increases by only about 20% when moving from the Arduino MEGA to the Arduino DUE. I attribute this to inherent limitations of the chip itself, which appears to be optimized for page-write operation at the expense of byte-wise writes.

In contrast, page-write performance improves by a factor of approximately 3.5 between the two Arduino platforms. This makes it possible to program the full 256 Kbit of memory in about one and a half minutes, which is comparable to read time—typically an order of magnitude faster than write operations.

Read Operation

| Arduino | Page Write Mode | Full Memory Read (sec) | Full Memory Write (sec) |
| -- | :--: | :--: | :--: |
| MEGA, 16 MHz | – | 84 | 305 |
| DUE, 84 MHz | no | 56 | 255 |
| DUE, 84 MHz | yes | 56 | 85 |


## Conclusions

Implementing this mode turned out to be an interesting exercise. On the one hand, the logical structure of the EEPROM Programmer required virtually no changes. On the other hand, it took a significant amount of time to understand why this mode did not work in the first place on the Arduino MEGA. I ran a large number of experiments, switching between different `!WE` and `!CE` activation strategies and optimizing both the write and read procedures, before realizing that I had simply hit the performance limits of the platform.

Unfortunately, I was not able to run the EEPROM Programmer on the Arduino GIGA and measure write performance on that platform. Given the roughly 30x increase in microcontroller clock speed, it would be very interesting to see whether the total chip programming time scales linearly with frequency.

The GIGA does not behave well with the Serial interface and exhibits unusual behavior. All other platforms reset when a Serial connection is opened, but the GIGA does not. Additionally, I was unable to get any output to the console during the `setup()` function.

In the next article, I plan to summarize the work on this project, describe all supported chip types, and compare read and write performance between the commercial XGecu hardware programmer and the Arduino-based solution.