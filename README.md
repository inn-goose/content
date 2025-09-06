# Arduino Performance Measurement with Oscilloscope

## TLDR

When I was working on improving the performance of my [EEPROM API](https://github.com/inn-goose/eeprom-arduino), I noticed that different microcontroller models execute read operations at different speeds, say the difference between the GIGA and the MEGA can be as much as 20x.

So I decided to measure the performance of different Arduino microcontroller models using an oscilloscope. I'm trying to determine the minimum execution time of a simple digital pin toggle operation. And try to estimate the number of CPU instructions based on the CPU clock speed and the operation’s execution time.

The source code: [sketch.ino](sketch/sketch.ino)


## Oscilloscopes

### FNIRSI DSO152

![FNIRSI DSO152](https://github.com/inn-goose/arduino-performance/blob/main/images/FNIRSI%20DSO152.jpeg?raw=true)

| Bandwidth | Sampling Rate | Min Time Div |
| -- | :-: | :-: |
| 200 KHz | 2.5 MSa/s | 10 μs |

[Manufacturer description](https://www.fnirsi.com/products/dso-152?variant=46705204134106)

A small single-channel oscilloscope that introduced me to the world of waveforms. It's well suited for measuring simple processes such as capacitor discharge or a signal from a 555 timer. But the limitations of a single channel become apparent quickly.

Its bandwidth is sufficient to measure the fastest operations on the UNO R3. Beyond that, limitations start to appear.

### OWON HDS242

![OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/OWON%20HDS242.jpeg?raw=true)

| Bandwidth | Sampling Rate | Min Time Div |
| -- | :-: | :-: |
| 40 MHz | 250 MSa/s | 5 ns |

[Product datasheet](https://files.owon.com.cn/specifications/HDS200.pdf)

A very convenient two-channel oscilloscope with all the necessary features for advanced measurements. For example, it allows measuring the CPU clock speed on the `XTAL2` pin of an ATmega328P and correlating it with the duration of a `digitalWrite` operation. With some skill, it can even serve as a replacement for a four-channel oscilloscope.

Its bandwidth is sufficient to measure the fastest digital operations on the GIGA.


## UNO R3

The most common Arduino platform. Serves as the baseline for my calculations.

| CPU Clock | Cycle Length | Operation Length | Cycles per Operation |
| -- | :--: | :--: | :--: |
| 16 MHz | 62.5 ns | 10,600 ns | 170 |

$\frac{1 \text{ sec}}{16,000,000 \text{ Hz}} = 62.5  \text{ ns}$

### CPU Clock Measurement

Connecting to `XTAL2` (pin `10`, [pinout](https://github.com/inn-goose/arduino-performance/blob/main/images/ATmega328P%20Pinout.png?raw=true)) provides the CPU clock signal, as described in the [ATmega328P Datasheet](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf) and visible on the oscilloscope screen as a stable sine wave as well.

![UNO R3 Clock](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Clock.jpeg?raw=true)

The UNO R3 CPU frequency (16 MHz) and the OWON oscilloscope bandwidth (40 MHz) allow observing and measuring the cycle duration and board's crystal oscillator frequency with reasonable accuracy. The voltage measurement (600 mV) cannot be fully trusted, as it approaches the instrument’s measurement limit.

![UNO R3 Clock OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Clock%20OWON%20HDS242.jpeg?raw=true)

### `Pin Toggle` Operation Measurement

I defined the simplest operation measurable by the oscilloscope as a unit. The operation consists of two functions: one sets the pin high, the other sets it low. This sequence is looped, generating a "clean" square waveform on the oscilloscope. I refer to this operation as a `Pin Toggle` for simplicity.

(*) it can be observed that the transition from 0 to 1 takes slightly longer, with a delta within a few percent. I assume this is due to boilerplate around the `loop()` function, as the `Pin Toggle` operation also includes all the routine code hidden outside the `loop()` callback.

![UNO R3 Pin Toggle Operation](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Operation.jpeg?raw=true)

The fastest execution time of the `Pin Toggle` operation on the UNO R3 is approximately 10.6 µs, or 100 kHz. As my measurements below show, one operation cycle (blue) corresponds to roughly 170 CPU cycles (yellow).

![UNO R3 Pin Toggle Operation OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Operation%20OWON%20HDS242.jpeg?raw=true)


### "Slow" `5` Pin

An interesting platform feature that divides pins into "fast" and "slow". My measurements show a 1.5x regression in frequency between the "fast" `12` pin and the "slow" `5` pin.

The frequency of a "slow" pin toggle operation is approximately 65 kHz.

$\frac{1 \text{ sec}}{65,000 \text{ Hz}} = 15,384  \text{ ns}$

The speed and frequency of the pin toggle operation are measured. CPU frequency was measured only for the UNO R3, other platforms operate at frequencies beyond the measurement capabilities of my oscilloscopes. The measurement is performed with two oscilloscopes simultaneously to compare results.

![UNO R3 Slow Pin Oscilloscopes](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Slow%20Pin%20Oscilloscopes.jpeg?raw=true)

**FNIRSI DSO152**: The operation frequency approaches the oscilloscope’s measurement limit, but the square wave is still clearly distinguishable, as is the voltage level.

![UNO R3 Slow Pin FNIRSI DSO152](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Slow%20Pin%20FNIRSI%20DSO152.jpeg?raw=true)

**OWON HDS242**: A clear square waveform. This instrument is designed for such frequencies.

![UNO R3 Slow Pin OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Slow%20Pin%20OWON%20HDS242.jpeg?raw=true)

### "Fast" `12` Pin

The frequency of the "fast" pin toggle operation is approximately 100 kHz.

$\frac{1 \text{ sec}}{100,000 \text{ Hz}} = 10,000  \text{ ns}$

![UNO R3 Fast Pin Oscilloscopes](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Fast%20Pin%20Oscilloscopes.jpeg?raw=true)

**FNIRSI DSO152**: same as the "slow" pin measurement

![UNO R3 Fast Pin FNIRSI DSO152](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Fast%20Pin%20FNIRSI%20DSO152.jpeg?raw=true)

**OWON HDS242**: same as the "slow" pin measurement

![UNO R3 Fast Pin OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Fast%20Pin%20OWON%20HDS242.jpeg?raw=true)


## UNO R4

UNO R4 replaces the classic UNO with an ATmega328P, increasing the CPU clock speed, as well as the basic pin toggle operation speed. This can be useful when the microcontroller is used for data processing or data transmission. Measurements show an almost linear correlation between the CPU speed and the operation length. Both oscilloscopes handle pin toggle operation timing surprisingly well, even considering hardware limitations.

| CPU Clock | Cycle Length | Operation Length | Cycles per Operation |
| :--: | :--: | :--: | :--: |
| 48 MHz | 20.8 ns | 4,420 ns | 213 |

$\frac{1 \text{ sec}}{48,000,000 \text{ Hz}} = 20.83  \text{ ns}$

The frequency of the pin toggle operation is approximately 226 kHz. Which lays within the measurement range of the FNIRSI oscilloscope and well below the limits of the OWON model.

$\frac{1 \text{ sec}}{226,000 \text{ Hz}} = 4,425  \text{ ns}$

![UNO R4 Oscilloscopes](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R4%20Oscilloscopes.jpeg?raw=true)

Connecting to pins `5` and `12` yields the same frequency value, suggesting that the distinction between "fast" and "slow" pins is specific to the UNO R3.

![UNO R4 Oscilloscopes 2](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R4%20Oscilloscopes%202.jpeg?raw=true)

**FNIRSI DSO152**: The CPU clock speed is far beyond the measurement capabilities of the oscilloscope, so the instrument cannot display a clear square waveform. But it can estimate the cycle duration and fairly accurately show the voltage value vertically.

![UNO R4 FNIRSI DSO152](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R4%20FNIRSI%20DSO152.jpeg?raw=true)

**OWON HDS242**: The oscilloscope’s bandwidth allows measuring operations at these speeds without issues, producing a clear waveform. The 40 MHz bandwidth theoretically allows measuring the CPU clock, if measure the right pin of RA4M1 CPU.

![UNO R4 OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R4%20OWON%20HDS242.jpeg?raw=true)


## NANO ESP32

A very fast platform with extensive wireless capabilities but few pins. My measurements show the largest CPU-cycle overhead per pin toggle. I would attribute this to the processor architecture and the compiler internals.

| CPU Clock | Cycle Length | Operation Length | Cycles per Operation |
| :--: | :--: | :--: | :--: |
| 240 MHz | 4.2 ns | 1,376 ns | 328 |

$\frac{1 \text{ sec}}{240,000,000 \text{ Hz}} = 4.16  \text{ ns}$

The frequency of the pin toggle operation is approximately 726 kHz. Which exceeds the FNIRSI measurement limit by nearly four times, but still well below the limits of the OWON model.

$\frac{1 \text{ sec}}{726,000 \text{ Hz}} = 1,377  \text{ ns}$

![NANO ESP32 Oscilloscopes](https://github.com/inn-goose/arduino-performance/blob/main/images/NANO%20ESP32%20Oscilloscopes.jpeg?raw=true)

**FNIRSI DSO152**: Measuring the exact operation time is beyond the capabilities of the oscilloscope’s horizontal scale. However, a careful count of the pulses shows 6 operations per horizontal division, which roughly corresponds to the measurements of the higher-grade instrument.

![NANO ESP32 FNIRSI DSO152](https://github.com/inn-goose/arduino-performance/blob/main/images/NANO%20ESP32%20FNIRSI%20DSO152.jpeg?raw=true)

**OWON HDS242**: Measuring the CPU frequency is not possible, as it exceeds the capabilities of the current oscilloscope. However, measuring the time of a single operation produces a very clear waveform without bounce. The NANO ESP32 operates at 3.3V, which is clearly visible on the screen. The instrument is well suited for measurements at these speeds.

![NANO ESP32 OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/NANO%20ESP32%20OWON%20HDS242.jpeg?raw=true)


## GIGA R1

The fastest platform, which also allows connecting many pins and performing logic analysis of complex circuits. Only the OWON oscilloscope was capable of measuring the pin toggle operation time. CPU clock measurements are beyond the capabilities of both instruments.

| CPU Clock | Cycle Length | Operation Length | Cycles per Operation |
| :--: | :--: | :--: | :--: |
| 480 MHz | 2.1 ns | 466 ns | 222 |

$\frac{1 \text{ sec}}{480,000,000 \text{ Hz}} = 2.08  \text{ ns}$

The frequency of the pin toggle operation is approximately 2.15 MHz. Making the FNIRSI oscilloscope unusable and creating some challenges for the OWON model.

$\frac{1 \text{ sec}}{2,150,000 \text{ Hz}} = 465  \text{ ns}$

![GIGA R1 Oscilloscopes](https://github.com/inn-goose/arduino-performance/blob/main/images/GIGA%20R1%20Oscilloscopes.jpeg?raw=true)

**FNIRSI DSO152**: Measuring the exact operation time is beyond the capabilities of the oscilloscope’s horizontal scale. An approximate count gives 3 operations per 10 µs scale, which differs from the actual value by a factor of 7.

![GIGA R1 FNIRSI DSO152](https://github.com/inn-goose/arduino-performance/blob/main/images/GIGA%20R1%20FNIRSI%20DSO152.jpeg?raw=true)

**OWON HDS242**: Measuring the execution time of a single operation approaches the limit of the horizontal display, but shows the reasonable waveform. The voltage difference is less than 5 volts due to signal bounce.

![GIGA R1 OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/GIGA%20R1%20OWON%20HDS242.jpeg?raw=true)


## Conclusion

The ratio between the execution time of the pin toggle operation and the CPU clock speed depends on the hardware and compilers but remains fairly predictable. Several repeated measurements were made to confirm data consistency.

| Microcontroller | CPU Clock (MHz) | Operation Frequency (kHz) | Operation Length (ns) | Cycles per Operation |
| -- | :--: | :--: | :--: | :--: |
| Arduino UNO R3 | 16 | 100 | 10,600 | 170 |
| Arduino UNO R4 | 48 | 226 | 4,420 | 213 |
| Arduino NANO ESP32 | 240 | 726 | 1,376 | 328 |
| Arduino GIGA R3 | 480 | 2,150 | 466 | 222 |

The most cycles are required on the ESP32 platform, and the fewest on the ATmega328P. However the CPU clock directly affects the operation speed despite instruction overhead: the higher the processor speed, the faster the commands execute.

This is confirmed by measurements of the 28C64 EEPROM read operation. On the slowest MEGA with an ATmega328P, reading the entire memory takes approximately 1.4 sec, whereas on the GIGA with an STM32H7 it takes only 38 ms.