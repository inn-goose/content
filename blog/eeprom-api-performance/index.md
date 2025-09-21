---
draft: false
date: 2025-08-30
###
title: EEPROM API Performance with Arduino
linkTitle: eeprom-api-performance
###
# published: true
# description: Read and write operations are dominated by Arduino execution overhead, roughly 120 ns per digital pin operation. Active polling of the EEPROM `READY/BUSY` pin significantly reduces write times, cutting wait from 1400 µs to 600 µs without errors. Sequential write/read tests confirm reliable data transfer, while excessive speed risks corrupting writes. Future work includes testing chip endurance, data retention, and performance across different Arduino clock speeds.
# tags: arduino, eeprom, eeprom28c64
# cover_image: https://direct_url_to_image.jpg
# Use a ratio of 100:42 for best results.
# published_at: 2025-08-30 19:43 +0000
---

## Motivation

In my previous post, I described a basic implementation of the EEPROM API and demonstrated, in a fairly simple example, how to read and write a few data cells. At that point, performance and operation speed were not particularly important, since the goal was only to provide an initial demonstration.

Now I would like to focus on improving performance and, moreover, comparing the values from the EEPROM's datasheet with the actual operation speed. This should not really be considered a precise experiment on measuring the raw speed of the chip itself, because the Arduino platform inevitably introduces certain performance limitations by adding some overhead. Therefore, my goal is to make the read and write API operations as fast as realistically possible, while still taking into account the constraints of the platform.

To achieve this, I will strictly follow the exact pin activation sequence described in the datasheet's waveforms and also respect the timing requirements for operations, such as the Write Cycle Time. I will eliminate unnecessary waiting functions and, in addition, use an oscilloscope to illustrate how the write waveforms actually look in practice.

![Wiring for EEPROM and Oscilloscope](https://dev-to-uploads.s3.amazonaws.com/uploads/articles/8q4j934g1svood44cuu6.jpg)


## Read Waveforms

![EEPROM 28C64 Read Waveforms](https://dev-to-uploads.s3.amazonaws.com/uploads/articles/5q9rp4qqqc8r3tnu6hp5.png)

The sequence of pin activations required for reading data from the EEPROM is, in fact, fairly straightforward.

Read Operation Steps:
1. Set the address on the `Address Bus`
2. Enable the Chip by setting `CE` pin to `LOW`
3. Enable the Output by setting `OE` pin to `LOW`
4. (*) Wait for the `OE` to Output Delay Time
5. Read the cell value on the `Data Bus`
6. Disable the Chip by setting `CE` pin to `HIGH`
7. Disable the Output by setting `OE` pin to `HIGH`

Step 4* is, interestingly, the one to watch, because Arduino platform specifics start to show. The EEPROM Datasheet states that the time between setting the `OE` pin `LOW` and the value appearing on the Data Bus is between 10 and 70 ns for my EEPROM model.

![Oscilloscope: Pin Write Delta](https://dev-to-uploads.s3.amazonaws.com/uploads/articles/t4mzu8qxm4vn8uuhzumx.jpeg)

However, as the oscilloscope measurements indicate, the time between two pin write operations is about 120 ns. I can, reasonably, assume this is execution overhead, since the program is written in C and each function expands into a large set of low-level instructions. This delta between operations is sufficient for the value to appear on the Data Bus.

The Arduino Giga clock is 240 MHz, which is 20 times higher than the Mega’s 16 MHz, so the delta between pin write operations may be 20 times higher. I would like to run measurements of this kind in one of the next articles.


## Write Waveforms

![EEPROM 28C64 Write Waveforms](https://dev-to-uploads.s3.amazonaws.com/uploads/articles/thxi0expikx5l1r4p7lm.png)

The operations required for writing data are slightly more complex and require additional polling of the `READY/BUSY` pin to determine when the write process has completed.

Read Operation Steps:
1. Set the address on the `Address Bus`
2. Enable the Chip by setting `CE` pin to `LOW`
3. Enable the Write by setting `WE` pin to `LOW`
4. Set the value on the Address Bus (~1us, see below)
5. Disable the Write by setting `WE` pin to `HIGH` (initiates the write)
6. Disable the Chip by setting `CE` pin to `HIGH`
7. (*) Poll for the `READY` status

The datasheet specifies two timing intervals important for implementing Chip Status Polling:
* **Time to Device Busy** – the time between the `WE` pin rising edge and the `READY/BUSY` pin transitioning to `BUSY`, maximum 50 ns
* **Write Cycle Time** – the time required to write data into the chip’s memory, maximum 1 ms

I added a 1 µs wait for the **Time to Device Busy**, though this is arguably unnecessary given the platform overhead for each operation.

For the status polling procedure itself, I implemented a simple mechanism:
* If the chip is already in the `BUSY` state at the start of polling, check repeatedly every 200 µs until it transitions to `READY`, with a maximum timeout of 1.4 ms
* Alternatively, simply wait 1.4 ms using a standard `delay()` function

The `READY/BUSY` status pin uses an **Open Drain Output** connection, so it is necessary to set it to `INPUT_PULLUP` mode for reading, or alternatively use an external pull-up resistor.

I will cover the performance details of polling in the next section.

![Oscilloscope: CE and WE waveforms](https://dev-to-uploads.s3.amazonaws.com/uploads/articles/l4guag9x8ighozakrfog.jpeg)

The relationship between the `CE` (yellow) and `WE` (blue) pins, and their behavior during the write operation, is illustrated in the image above. First, the `CE` pin is pulled `LOW` to activate the chip. Then, the `WE` pin is pulled `LOW`, marking the start of placing a value on the Data Bus. The duration between the falling and rising edges of the `WE` pin is about 1200 ns, which corresponds to roughly 8 write operations at 120 ns each, plus the overhead required to set the value on the bus. The rising edge of `WE` initiates the actual data write process. The rising edge of `CE` no longer affects the operation, as can be seen from the waveforms.

![Oscilloscope: WE to BUSY waveforms](https://dev-to-uploads.s3.amazonaws.com/uploads/articles/jvh0zfnyiuwodlqsn62f.jpeg)

It turned out I misread the waveforms last time. `WE` (blue) does indeed trigger the data-write operation on the rising edge, transferring the chip into the `BUSY` state (yellow). This is confirmed by the oscilloscope measurements shown in the image above.

![Oscilloscope: CE to BUSY waveforms](https://dev-to-uploads.s3.amazonaws.com/uploads/articles/qoc3ayx5h72dn5moliwp.jpeg)

Setting the chip to inactive mode by driving the `CE` pin `HIGH` does not affect the data write process, as shown in the image above.


## Polling `BUSY` State

Using active polling of the `READY/BUSY` pin allows the write operation to be accelerated by roughly a factor of two.

Oscilloscope measurements show that a write takes about 500 µs, compared to the specified maximum of 1000 µs. However, even waiting manually for the maximum period can occasionally result in write errors. If a new write cycle begins before the previous one has completed, it can corrupt the data from the previous cycle by placing new values on the Data Bus. For reliability, I therefore set the minimum wait time to 1400 µs.

![Oscilloscope: READY delay](https://dev-to-uploads.s3.amazonaws.com/uploads/articles/falgedtzm6dq0lcmzb4j.jpeg)

![Oscilloscope: READY poll](https://dev-to-uploads.s3.amazonaws.com/uploads/articles/1krvx8dhv70306bv6r4q.jpeg)

Chip status polling, however, reduces the waiting time from 1400 µs to 600 µs without compromising write reliability. Two oscilloscope waveforms illustrate this difference. In the first image, the program simply waits 1400 µs. In the second image, the program actively polls the chip status and returns control from the write function as soon as the chip enters the `READY` state.

This time could be further improved by reducing the `delay()` between pin polls to 50 µs, which theoretically would accelerate the waiting procedure by a factor of three instead of two.


## Performance

[GitHub Project with the Performance API](https://github.com/inn-goose/eeprom_arduino/tree/main/eeprom_performance)

old API implementation:
```
WRITE | TOTAL: 8193044 us | AVG: 8001 us
READ | TOTAL: 15371674 us | AVG: 15011 us
VERIFY: OK
```

performance API implementation:
```
BUSY | TOTAL: 615502 us | AVG: 601 us | MAX: 605 us
WRITE | TOTAL: 622457 us | AVG: 607 us
READ | TOTAL: 4672 us | AVG: 4 us
VERIFY: OK
```

I run a sequential block of write operations followed by a block of read operations. I write 1024 cells one by one with random values, storing the written values in an array. Then I read the same 1024 cells one by one, storing the read values in another array. During each write operation, I also record in an array the waiting time for the chip’s `READY/BUSY` pin status, since this constitutes the main execution time consumption during writing.

The results of an "old API implementation" are not useful for comparison, as the first API version was optimized for visual demonstration rather than speed. I prefer to compare results against the maximum expected values from the datasheet.

I determined that a single digital pin read/write operation takes roughly 120 ns. During the write procedure, I perform 25 digital operations, giving 3000 ns, plus the array conversion procedure, resulting in about 4000 ns total, which aligns with the observed results. This process could be accelerated by caching the mapping between address values and bit representation, potentially speeding up single-cell reads by 5-10%. However, this is largely unnecessary, since the entire 8K cell address space can already be read in 40 ms.

For write operations, the picture is similar regarding execution overhead. The average operation execution time is 607 µs, with an average `READY/BUSY` pin polling wait of 601 µs, leaving about 6 µs for Arduino-side operations. This is slightly higher than for reading, due to data conversion and a 1 µs `delay()` before polling. The polling process itself could be sped up by checking the status every 20 µs, reducing total wait time by roughly 100 µs.

The values achieved for read operations are very good, as the datasheet specifies a maximum wait of 1 ms, whereas I achieve 0.6 ms.


## Data Verification

After completing the write and read blocks, I compare the arrays to match data and check for errors. If writes are performed too quickly, operations overlap and data integrity suffers. For example, reducing the `READY` wait time from 1.4 ms to 0.2 ms results in almost all cells being written incorrectly.

This data verification is unrelated to the data corruption discussed in my previous article, since here I write and read within a single operational cycle. Detecting potential memory degradation of the chip requires performing a power cycle between write and read operations, which will be the topic of one of my upcoming articles.


## Next Steps

(1) Check how many write cycles the new EEPROM chip supports and examine how its write endurance degrades, whether this manifests as slower write speeds or as data corruption.

(2) Write a program to verify the data stored on the EEPROM chip. Additionally, test data retention over a 24-hour period (the datasheet specifies 10 years).

(3) Assemble a breadboard setup with a ZIF socket to allow quick replacement of EEPROM chips for validation and verification.

(4) Compare the overhead of `digitalWrite()` and `digitalRead()` operations on Arduino Mega versus Arduino Giga, taking into account the 20× difference in clock speed.