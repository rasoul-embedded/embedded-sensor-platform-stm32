# STM32F407 + LSM6DSO32 IMU System

## Month 2 — Real-Time Sampling, Calibration, and Filtering

---

## 📌 Overview

This project implements a real-time IMU acquisition and processing system on the **STM32F407 Discovery** using the **LSM6DSO32** inertial sensor.

At the end of Month 2, the system supports:

* Timer-driven deterministic sampling
* Accelerometer and gyroscope acquisition via I2C
* Bias calibration (accelerometer and gyroscope)
* Real-time low-pass filtering
* UART data logging (Linux-compatible)
* Timing analysis using GPIO + logic analyzer

The system is now stable, measurable, and ready for orientation estimation.

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
imu_process() → UART
```

---

## ⚙️ Hardware Setup

* **STM32F407 Discovery**
* **LSM6DSO32 IMU**
* USB-to-Serial adapter
* Logic analyzer (for timing validation)

### Pin Configuration

| Function                | Pin  |
| ----------------------- | ---- |
| I2C1_SCL                | PB6  |
| I2C1_SDA                | PB7  |
| USART2_TX               | PA2  |
| USART2_RX               | PA3  |
| Debug (IMU read)        | PD12 |
| Debug (processing/UART) | PD13 |

---

## ⏱️ Sampling System

Sampling is controlled using **TIM2 interrupt**.

### Configuration

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

Heavy operations (I2C, UART) are executed in the main loop, not inside the ISR.

---

## 📊 Timing Analysis

Two GPIO pins are used for profiling:

* **PD12** → HIGH during IMU read
* **PD13** → HIGH during processing + UART

### Measured values

* Total period ≈ **9.98 ms**
* IMU read time ≈ **1.7–1.8 ms**
* Processing + UART ≈ **3.0–3.2 ms**

### Observations

* System is stable at 100 Hz
* UART formatting contributes significantly to runtime
* Enough margin remains for additional processing

---

## 🧪 Calibration

### Accelerometer

Target (stationary):

```text
AX ≈ 0 mg
AY ≈ 0 mg
AZ ≈ 1000 mg
```

Bias correction:

```c
a.x = accel_g.x * 1000 - AX_BIAS;
```

### Gyroscope

Target (stationary):

```text
GX ≈ 0
GY ≈ 0
GZ ≈ 0
```

Bias correction:

```c
g.x = gyro_dps.x * 1000 - GX_BIAS;
```

### Result

* Accelerometer correctly measures gravity (~1 g)
* Gyroscope residual error reduced to small noise levels

---

## 🟦 Real-Time Filtering

### Filter Type

First-order low-pass filter (IIR)

### Equation

```text
y[n] = α * x[n] + (1 - α) * y[n-1]
```

### Implementation

```c
#define ALPHA 0.1f

a_f.x = ALPHA * a.x + (1.0f - ALPHA) * a_f.x;
g_f.x = ALPHA * g.x + (1.0f - ALPHA) * g_f.x;
```

### Effect

* Reduces high-frequency noise
* Produces smoother signals
* Introduces small delay (expected)

---

## 📡 Data Logging

Data is transmitted via UART in CSV format.

### Example format

```text
sample,AX_raw,AX_filt,AY_raw,AY_filt,...
```

### Linux logging

```bash
stty -F /dev/ttyUSB0 115200 raw -echo
cat /dev/ttyUSB0 | tee imu_data.csv
```

---

## 📈 Data Analysis

Python scripts are used to:

* clean corrupted lines
* plot raw vs filtered data
* evaluate filter performance

### Observations

* Raw signals contain visible noise
* Filtered signals are smoother
* Mean values remain consistent
* Filter introduces small lag

---

## ⚠️ Current Limitations

* UART transmission is blocking
* `sprintf()` is computationally expensive
* Occasional corrupted serial lines
* No orientation estimation yet
* No DMA or interrupt-based UART
is README with your Git commits (so everything matches cleanly)
* or directly start structuring your **Month 3 Week 1 refactor (`imu_app.c`)** 👍
