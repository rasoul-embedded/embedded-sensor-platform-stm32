# STM32F407 Real-Time IMU Orientation Estimation

## 📌 Overview

This project implements a real-time IMU acquisition, processing, orientation-estimation, and visualization system on the **STM32F407 Discovery** using the **LSM6DSO32** inertial sensor.

The system started as a deterministic IMU data acquisition project and has been extended into a complete real-time roll/pitch estimation demo. It supports timer-driven 100 Hz sampling, calibrated accelerometer and gyroscope acquisition, low-pass filtering, complementary filtering, 1D Kalman filtering, UART DMA streaming, offline validation, and live 3D orientation visualization in Python.

The current system supports:

* Timer-driven deterministic sampling at 100 Hz
* Accelerometer and gyroscope acquisition via I2C
* Bias calibration for accelerometer and gyroscope
* Real-time low-pass filtering
* Roll and pitch estimation using a complementary filter
* Reusable 1D Kalman filter for roll and pitch
* UART data logging in CSV format
* UART transmission using DMA
* Timing analysis using GPIO and a logic analyzer
* Linux-compatible serial data logging
* Offline CSV validation and analysis
* Python-based live roll/pitch plotting
* Python-based live 3D orientation visualization

The system is now stable, measurable, and suitable as a base for future control-oriented embedded applications.

---

## 🎯 Project Objectives

* Implement deterministic timer-based IMU sampling
* Acquire accelerometer and gyroscope data from the LSM6DSO32
* Correct raw sensor outputs using bias calibration
* Reduce signal noise using real-time low-pass filtering
* Estimate roll and pitch using a complementary filter
* Implement reusable 1D Kalman filtering for roll and pitch
* Reduce UART logging overhead using DMA
* Validate timing behavior using GPIO and a logic analyzer
* Validate logged data quality using offline CSV analysis
* Stream real-time orientation data to a host PC
* Visualize roll and pitch live using Python
* Display a live 3D representation of the board orientation

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
IMU_Estimator_Update()
        ↓
Complementary filter update
        ↓
Roll/Pitch Kalman filter update
        ↓
imu_process()
        ↓
UART DMA streaming
        ↓
Python live plot / 3D visualization
```

Heavy operations such as I2C communication, calibration, filtering, angle estimation, Kalman filtering, string formatting, and UART transmission setup are executed in the main loop, not inside the timer ISR.

---

## ⚙️ Hardware Setup

* **STM32F407 Discovery**
* **LSM6DSO32 IMU**
* USB-to-Serial adapter
* Logic analyzer for timing validation
* Linux host PC for serial logging and Python visualization

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

The interrupt only sets a flag. The actual IMU reading, filtering, estimation, and UART logging are handled in the main loop.

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

Roll and pitch are estimated in real time using both a complementary filter and a reusable 1D Kalman filter.

The complementary filter combines short-term gyroscope integration with long-term accelerometer correction. The Kalman filter estimates the angle while also estimating gyroscope bias.

---

### Accelerometer-Based Angles

```c
float roll_acc = atan2f(a_f.y, a_f.z) * RAD_TO_DEG;

float pitch_acc = atan2f(
    -a_f.x,
    sqrtf(a_f.y * a_f.y + a_f.z * a_f.z)
) * RAD_TO_DEG;
```

---

### Complementary Filter

```c
roll_gyro  = roll  + gx_dps * DT;
pitch_gyro = pitch + gy_dps * DT;

roll  = 0.98f * roll_gyro  + 0.02f * roll_acc;
pitch = 0.98f * pitch_gyro + 0.02f * pitch_acc;
```

The gyroscope provides fast short-term response, while the accelerometer corrects long-term drift.

---

### 1D Kalman Filter

A reusable 1D Kalman filter module is implemented in:

```text
Core/Inc/kalman_1d.h
Core/Src/kalman_1d.c
```

The filter estimates:

```text
state[0] = angle
state[1] = gyro_bias
```

Prediction step:

```text
rate = gyro_rate - gyro_bias
angle = angle + dt * rate
```

Correction step:

```text
measurement = accelerometer_angle
error = measurement - predicted_angle
```

The same Kalman filter structure is used independently for roll and pitch.

Initial tuning parameters:

```c
#define KALMAN_Q_ANGLE      0.001f
#define KALMAN_Q_BIAS       0.003f
#define KALMAN_R_MEASURE    0.03f
```

### Result

* Roll and pitch are estimated in real time
* Complementary and Kalman outputs show correct sign and stable behavior
* Stationary and dynamic datasets confirm stable roll/pitch tracking
* No Kalman divergence was observed during validation
* The estimator output is suitable for live visualization and future control applications

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

---

## 🧵 UART DMA Runtime Behavior

The system uses a single transmit buffer for the current DMA implementation.

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

## 📤 Live Visualization Output Format

For the live Python visualization, the firmware streams compact CSV data:

```text
START
counter,roll,pitch
0,123,-45
1,124,-44
```

where:

```text
roll  = roll angle  * 100
pitch = pitch angle * 100
```

Example:

```text
0,123,-45
1,124,-44
```

This means:

```text
roll  = 1.23°
pitch = -0.45°
```

The compact format keeps UART bandwidth low and is suitable for real-time plotting and 3D visualization.

---

## 🐧 Linux Logging

Serial data can be logged on Linux using:

```bash
stty -F /dev/ttyUSB0 115200 raw -echo
cat /dev/ttyUSB0 | tee imu_data.csv
```

Stop logging with:

```text
CTRL + C
```

---

## 🖥️ Python Live 3D Orientation Visualization

A Python visualization tool is provided for real-time orientation display:

```text
tools/live_3d_orientation.py
```

The script:

* Opens `/dev/ttyUSB0`
* Reads UART CSV lines
* Parses roll and pitch
* Converts scaled integer values to degrees
* Displays a live 3D representation of the board
* Shows roll and pitch trends in real time
* Can continue parsing even if the STM32 was already running before the script started

Install dependencies:

```bash
python3 -m pip install pyserial matplotlib numpy
```

Run:

```bash
python3 tools/live_3d_orientation.py --port /dev/ttyUSB0 --baud 115200
```

The 3D representation uses roll and pitch only. Yaw is not shown because the current system does not use a magnetometer.

---

## 📈 Data Validation

Logged CSV data was checked offline using Python analysis scripts.

Validation checks included:

* Parsing data after the final `START` marker
* Checking malformed rows
* Checking missing counters
* Checking duplicate counters
* Converting scaled integer values back to degrees
* Calculating mean, standard deviation, minimum, and maximum
* Comparing complementary and Kalman outputs
* Checking dynamic tracking behavior
* Checking return-to-zero behavior
* Checking stationary stability during hold

### Observations

* Sample counters increased continuously in the validated logs
* No missing samples were observed after the `START` marker
* No duplicate samples were observed
* Roll and pitch values remained stable during stationary tests
* Positive and negative roll directions were correct
* Positive and negative pitch directions were correct
* Kalman outputs followed the complementary filter closely
* No Kalman divergence or unstable behavior was observed
* Dynamic roll/pitch movement was tracked correctly

### Stationary Validation

Stationary datasets were collected for flat/slight tilt, positive roll, negative roll, and positive pitch positions.

The final stable regions showed low standard deviation and close agreement between complementary and Kalman estimates.

### Dynamic Validation

Dynamic datasets were collected for:

* Positive roll motion
* Negative roll motion
* Positive pitch motion
* Combined roll/pitch motion

The Kalman output followed the complementary output with small average difference during motion. Return-to-zero behavior was stable after movement.

---

## 🧪 Offline Analysis Tools

Python scripts are used for plotting and validating logged CSV data.

Example validation script:

```text
tools/plot_roll_pitch_kalman.py
```

The script:

* Finds the last `START` marker
* Parses numeric CSV rows
* Removes duplicate counters
* Converts scaled values to degrees
* Plots roll complementary vs roll Kalman
* Plots pitch complementary vs pitch Kalman
* Calculates mean, standard deviation, minimum, and maximum

---

## ⚠️ Current Limitations

* I2C IMU reading is still blocking
* UART DMA currently uses a single transmit buffer
* Log lines may be skipped if DMA is busy
* No double-buffered UART logger yet
* No ring-buffer logging yet
* `sprintf()` is still used for formatting
* Only roll and pitch are estimated
* Yaw is not estimated because no magnetometer is used
* No quaternion-based attitude estimation yet
* No full 9-axis sensor fusion yet
* The live 3D visualization is based only on roll and pitch
* No RTOS is used
* No closed-loop control application implemented yet

---

## ✅ Achievements

* Deterministic 100 Hz sampling implemented
* TIM2 interrupt used only for sample timing
* IMU acquisition through I2C working
* Accelerometer and gyroscope calibration completed
* Real-time filtering implemented
* Complementary filter for roll and pitch implemented
* Reusable 1D Kalman filter module implemented
* Roll Kalman estimation validated
* Roll and pitch Kalman estimation validated
* UART DMA transmission implemented
* DMA driver infrastructure started
* Logic analyzer timing validation completed
* UART logging overhead reduced from milliseconds to microseconds
* CSV data logging and offline validation completed
* Python offline validation plots generated
* Python live roll/pitch plotting implemented
* Python live 3D orientation visualization implemented
