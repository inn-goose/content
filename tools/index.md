---
###
title: Tools
###
---

## Oscilloscopes

### FNIRSI DSO152

![FNIRSI DSO152](images/fnirsi-dso152.jpeg)

| Bandwidth | Sampling Rate | Min Time Div |
| :--: | :-: | :-: |
| 200 KHz | 2.5 MSa/s | 10 μs |

[Manufacturer description](https://www.fnirsi.com/products/dso-152?variant=46705204134106)

A small single-channel oscilloscope that introduced me to the world of waveforms. It's well suited for measuring simple processes such as capacitor discharge or a signal from a 555 timer. But the limitations of a single channel become apparent quickly.

Its bandwidth is sufficient to measure the fastest operations on the UNO R3. Beyond that, limitations start to appear.

### OWON HDS242

![OWON HDS242](images/owon-hds242.jpeg)

| Bandwidth | Sampling Rate | Min Time Div |
| :--: | :-: | :-: |
| 40 MHz | 250 MSa/s | 5 ns |

[Product datasheet](https://files.owon.com.cn/specifications/HDS200.pdf)

A very convenient two-channel oscilloscope with all the necessary features for advanced measurements. For example, it allows measuring the CPU clock speed on the `XTAL2` pin of an ATmega328P and correlating it with the duration of a `digitalWrite` operation. With some skill, it can even serve as a replacement for a four-channel oscilloscope.

Its bandwidth is sufficient to measure the fastest digital operations on the GIGA.