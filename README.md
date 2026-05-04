## 📌 Overview

This project implements a real-time IMU acquisition and processing system on the **STM32F407 Discovery** using the **LSM6DSO32** inertial sensor.

At the end of Month 2, the system supports:

* Timer-driven deterministic sampling at 100 Hz
* Accelerometer and gyroscope acquisition via I2C
* Bias calibration for accelerometer and gyroscope
* Real-time low-pass filtering
* Roll and pitch estimation using a complementary filter
* UART data logging in CSV format
* UART transmission using DMA
* Timing analysis using GPIO and a logic analyzer
* Linux-compatible serial data logging
* Offline CSV validation and analysis

The system is now stable, measurable, and ready for further orientation estimation and control-oriented applications.

---

## 🎯 Objectives of Month 2

* Replace uncontrolled sampling with deterministic timer-based sampling
* Correct raw sensor outputs using bias calibration
* Reduce noise using real-time filtering
* Implement basic roll and pitch estimation
* Reduce UART logging overhead using DMA
* Validate timing behavior using a logic analyzer
* Validate logged data quality using offline analysis

---

## 🧭 System Architecture

```text
TIM2 Interrupt (100 Hz)
        ↓
sample_flag = 1
        ↓
Main Loop
        ↓
imu_read_raw()
        ↓
imu_apply_calibration()
        ↓
imu_apply_filter()
        ↓
imu_compute_angles()
        ↓
imu_process()
        ↓
UART DMA Logging
````

Heavy operations such as I2C communication, filtering, angle computation, string formatting, and UART logging are executed in the main loop, not inside the timer ISR.

---

## ⚙️ Hardware Setup

* **STM32F407 Discovery**
* **LSM6DSO32 IMU**
* USB-to-Serial adapter
* Logic analyzer for timing validation

### Pin Configuration

| Function                  | Pin  |
| ------------------------- | ---- |
| I2C1_SCL                  | PB6  |
| I2C1_SDA                  | PB7  |
| USART2_TX                 | PA2  |
| USART2_RX                 | PA3  |
| Debug: IMU read           | PD12 |
| Debug: processing/logging | PD13 |

---

## ⏱️ Sampling System

Sampling is controlled using the **TIM2 update interrupt**.

### TIM2 Configuration

```c
Prescaler = 15999
AutoReload = 9
```

### Result

* Sampling frequency ≈ **100 Hz**
* Sampling period ≈ **10 ms**

### ISR Behavior

```c
void TIM2_IRQHandler(void)
{
    sample_flag = 1;
}
```

The interrupt only sets a flag. The actual IMU reading and processing are handled in the main loop.

---

## 📊 Timing Analysis

Two GPIO pins are used for profiling:

* **PD12** → HIGH during IMU read
* **PD13** → HIGH during processing and logging

### Before UART DMA

Previous blocking UART implementation:

* Total period ≈ **9.98 ms**
* IMU read time ≈ **1.7–1.8 ms**
* Processing + UART logging ≈ **3.0–3.2 ms**

### After UART DMA

With UART transmission moved to DMA:

* Total period ≈ **9.987 ms**
* Sampling frequency ≈ **100.1 Hz**
* IMU read time ≈ **1.78 ms**
* Processing + DMA logging launch ≈ **196 µs**

### Observations

* Sampling remains stable at 100 Hz
* UART transmission no longer blocks the CPU
* CPU load during logging is greatly reduced
* The dominant runtime cost is now the I2C IMU read
* The system has enough timing margin for additional estimation or control algorithms

---

## 🧪 Calibration

### Accelerometer

Target when stationary:

```text
AX ≈ 0 mg
AY ≈ 0 mg
AZ ≈ 1000 mg
```

Bias correction:

```c
a.x = accel_g.x * 1000 - AX_BIAS;
a.y = accel_g.y * 1000 - AY_BIAS;
a.z = accel_g.z * 1000 - AZ_BIAS;
```

### Gyroscope

Target when stationary:

```text
GX ≈ 0
GY ≈ 0
GZ ≈ 0
```

Bias correction:

```c
g.x = gyro_dps.x * 1000 - GX_BIAS;
g.y = gyro_dps.y * 1000 - GY_BIAS;
g.z = gyro_dps.z * 1000 - GZ_BIAS;
```

### Result

* Accelerometer correctly measures gravity
* Gyroscope residual bias is reduced to small noise levels
* Calibrated data is suitable for roll and pitch estimation

---

## 🟦 Real-Time Filtering

### Filter Type

First-order low-pass IIR filter.

### Equation

```text
y[n] = α * x[n] + (1 - α) * y[n-1]
```

### Implementation

```c
#define ALPHA 0.1f

a_f.x = ALPHA * a.x + (1.0f - ALPHA) * a_f.x;
a_f.y = ALPHA * a.y + (1.0f - ALPHA) * a_f.y;
a_f.z = ALPHA * a.z + (1.0f - ALPHA) * a_f.z;

g_f.x = ALPHA * g.x + (1.0f - ALPHA) * g_f.x;
g_f.y = ALPHA * g.y + (1.0f - ALPHA) * g_f.y;
g_f.z = ALPHA * g.z + (1.0f - ALPHA) * g_f.z;
```

### Effect

* Reduces high-frequency noise
* Produces smoother accelerometer and gyroscope signals
* Introduces a small expected delay
* Improves stability of angle estimation

---

## 🧭 Roll and Pitch Estimation

A complementary filter is used to estimate roll and pitch.

### Accelerometer-Based Angles

```c
float roll_acc  = atan2f(a_f.y, a_f.z) * RAD_TO_DEG;

float pitch_acc = atan2f(
    -a_f.x,
    sqrtf(a_f.y * a_f.y + a_f.z * a_f.z)
) * RAD_TO_DEG;
```

### Gyroscope Integration + Complementary Filter

```c
roll  = 0.98f * (roll  + gx_dps * DT) + 0.02f * roll_acc;
pitch = 0.98f * (pitch + gy_dps * DT) + 0.02f * pitch_acc;
```

### Result

* Roll and pitch are estimated in real time
* Gyroscope provides short-term response
* Accelerometer provides long-term correction
* Output is stable for stationary measurements

---

## 📡 UART DMA Data Logging

Data is transmitted through **USART2** using **DMA1 Stream 6 Channel 4**.

### DMA Mapping

```text
USART2_TX → DMA1 Stream 6 Channel 4
```

### DMA Configuration

```text
Direction: memory-to-peripheral
Peripheral increment: disabled
Memory increment: enabled
Peripheral data size: byte
Memory data size: byte
Mode: normal
Priority: low
Interrupt: transfer complete
```

### Logging Format

Current CSV output:

```text
sample,roll_i,pitch_i
```

where:

```text
roll_i  = roll  * 100
pitch_i = pitch * 100
```

Example:

```text
50469,22,123
50470,22,123
50471,21,124
```

This keeps UART messages short and reduces bandwidth usage.

---

## 🧵 UART DMA Runtime Behavior

The system uses a single transmit buffer for the first DMA implementation.

Before writing new data into the transmit buffer, the firmware checks whether DMA is busy:

```c
if (usart2_dma_tx_busy)
{
    return;
}
```

If DMA is free:

```text
format message
configure DMA addresses
set transfer length
enable USART DMA request
enable DMA stream
```

When the DMA transfer completes, the `DMA1_Stream6_IRQHandler()` clears the DMA flags and marks UART DMA as idle.

### Current Behavior

* Non-blocking UART logging
* CPU does not wait for UART byte transmission
* If DMA is busy, the current log line is skipped
* No double buffering yet

---

## 🐧 Linux Logging

Serial data is logged on Linux using:

```bash
stty -F /dev/ttyUSB0 115200 raw -echo
cat /dev/ttyUSB0 | tee imu_data.csv
```

---

## 📈 Data Validation

Logged CSV data was checked offline.

### Observations

* Sample counter increases continuously
* No missing samples observed in the validated log
* No duplicate samples observed
* Roll and pitch values remain stable during stationary operation
* One corrupted startup line may appear at the beginning of logging and is ignored during analysis

### Example Validated Output Range

```text
roll  ≈ 0.19° to 0.24°
pitch ≈ 1.20° to 1.25°
```

This confirms stable orientation output for a stationary setup with slight tilt.

---

## ⚠️ Current Limitations

* I2C IMU reading is still blocking
* UART DMA currently uses a single transmit buffer
* Log lines may be skipped if DMA is busy
* No double-buffered UART logger yet
* No ring-buffer logging yet
* `sprintf()` is still used for formatting
* No full 3D orientation estimation yet
* No quaternion-based attitude estimation yet
* No Kalman filter implemented yet

---

## ✅ Achievements

* Deterministic 100 Hz sampling implemented
* TIM2 interrupt used only for sample timing
* IMU acquisition through I2C working
* Accelerometer and gyroscope calibration completed
* Real-time filtering implemented
* Complementary filter for roll and pitch implemented
* UART DMA transmission implemented
* DMA driver infrastructure started
* Logic analyzer timing validation completed
* UART logging overhead reduced from milliseconds to microseconds
* CSV data logging and offline validation completed

---

