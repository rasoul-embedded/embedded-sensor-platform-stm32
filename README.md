# STM32F407 + LSM6DSO32 IMU Bring-Up (Month 1)

## Overview

This project implements a basic embedded system on the STM32F407 Discovery board to interface with the LSM6DSO32 IMU.
The system periodically reads accelerometer and gyroscope data over I2C and streams it via UART.

The focus of this stage is:

* Low-level peripheral configuration
* Interrupt-driven sampling
* Sensor communication

---

## Hardware

* STM32F407 Discovery
* LSM6DSO32 IMU

### Pin Configuration

| Function  | Pin |
| --------- | --- |
| I2C1_SCL  | PB6 |
| I2C1_SDA  | PB7 |
| USART2_TX | PA2 |
| USART2_RX | PA3 |

---

## Features

* Custom GPIO, I2C, USART, and TIM drivers
* I2C communication with LSM6DSO32
* Timer-based periodic sampling
* Interrupt-driven design using `sample_flag`
* UART data streaming (115200 baud)

---

## System Architecture

```
TIM2 Interrupt
      ↓
sample_flag = 1
      ↓
Main Loop
      ↓
Read IMU (I2C)
      ↓
Format Data
      ↓
Send via UART
```

---

## Example Output

```
AX=12 AY=-8 AZ=998 | GX=15 GY=-2 GZ=6
AX=10 AY=-9 AZ=1001 | GX=14 GY=-3 GZ=5
```

---

## Code Structure

```
/drivers
    gpio
    i2c
    usart
    tim

/sensor
    lsm6dso32

main.c
```

---

## Key Implementation Details

### Timer (TIM2)

* Generates periodic update interrupt
* ISR sets `sample_flag`
* No heavy processing inside ISR

### I2C

* Standard mode
* Used for sensor register read/write

### USART2

* 115200 baud
* Used for debugging and data streaming

### Sampling Strategy

* Timer controls sampling timing
* Main loop performs sensor read and transmission
* Prevents blocking inside interrupts

---

## Limitations

* No calibration
* No filtering
* No sensor fusion
* UART is blocking
* Sampling timing affected by `sprintf` and transmission

---

## Next Steps (Month 2)

* Deterministic sampling validation
* Sensor calibration (bias removal)
* Digital filtering
* Complementary filter (orientation estimation)

---
