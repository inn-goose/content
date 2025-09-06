# Arduino Performance Measurement with Oscilloscope

## TLDR

When I was working on improving the performance of my [EEPROM API](https://github.com/inn-goose/eeprom-arduino), I noticed that different microcontroller models execute read operations at different speeds, say the difference between the GIGA and the MEGA can be as much as 20x.

So I decided to measure the performance of different Arduino microcontroller models using an oscilloscope. I'm trying to determine the minimum execution time of a simple digital pin toggle operation. And try to estimate the number of CPU instructions based on the clock speed and the operation’s execution time.

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

A very convenient two-channel oscilloscope with all the necessary features for advanced measurements. For example, it allows measuring the clock speed on the `XTAL2` pin of an ATmega328P and correlating it with the duration of a `digitalWrite` operation. With some skill, it can even serve as a replacement for a four-channel oscilloscope.

Its bandwidth is sufficient to measure the fastest digital operations on the GIGA.


## UNO R3

### "Slow" 5 Pin

![UNO R3 Slow Pin Oscilloscopes](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Slow%20Pin%20Oscilloscopes.jpeg?raw=true)

![UNO R3 Slow Pin FNIRSI DSO152](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Slow%20Pin%20FNIRSI%20DSO152.jpeg?raw=true)

![UNO R3 Slow Pin OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Slow%20Pin%20OWON%20HDS242.jpeg?raw=true)

### "Fast" 12 Pin

![UNO R3 Fast Pin Oscilloscopes](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Fast%20Pin%20Oscilloscopes.jpeg?raw=true)

![UNO R3 Fast Pin FNIRSI DSO152](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Fast%20Pin%20FNIRSI%20DSO152.jpeg?raw=true)

![UNO R3 Fast Pin OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R3%20Fast%20Pin%20OWON%20HDS242.jpeg?raw=true)


## UNO R4

![UNO R4 Oscilloscopes](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R4%20Oscilloscopes.jpeg?raw=true)

![UNO R4 Oscilloscopes 2](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R4%20Oscilloscopes%202.jpeg?raw=true)

![UNO R4 OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R4%20OWON%20HDS242.jpeg?raw=true)

![UNO R4 FNIRSI DSO152](https://github.com/inn-goose/arduino-performance/blob/main/images/UNO%20R4%20FNIRSI%20DSO152.jpeg?raw=true)


## GIGA R1

![GIGA R1 Oscilloscopes](https://github.com/inn-goose/arduino-performance/blob/main/images/GIGA%20R1%20Oscilloscopes.jpeg?raw=true)

![GIGA R1 FNIRSI DSO152](https://github.com/inn-goose/arduino-performance/blob/main/images/GIGA%20R1%20FNIRSI%20DSO152.jpeg?raw=true)

![GIGA R1 OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/GIGA%20R1%20OWON%20HDS242.jpeg?raw=true)


## NANO ESP32

![NANO ESP32 Oscilloscopes](https://github.com/inn-goose/arduino-performance/blob/main/images/NANO%20ESP32%20Oscilloscopes.jpeg?raw=true)

![NANO ESP32 FNIRSI DSO152](https://github.com/inn-goose/arduino-performance/blob/main/images/NANO%20ESP32%20FNIRSI%20DSO152.jpeg?raw=true)

![NANO ESP32 OWON HDS242](https://github.com/inn-goose/arduino-performance/blob/main/images/NANO%20ESP32%20OWON%20HDS242.jpeg?raw=true)
