# Month 4 Week 1 — Complementary Filter Validation

## 1. Objective

Validate the roll and pitch estimation pipeline using accelerometer angles, gyro one-step prediction, and complementary filter output.

The goal of this week was to confirm that the current complementary filter is stable and reliable before moving to Kalman filter implementation.

---

## 2. Firmware Setup

1. Board: STM32F407 Discovery
2. IMU: LSM6DSO32
3. Sampling rate: 100 Hz
4. Sampling timer: TIM2
5. UART logging: USART2 TX using DMA
6. DMA mapping: DMA1 Stream 6 Channel 4
7. Estimator: complementary filter
8. Accelerometer correction weight: `COMP_ALPHA = 0.02`
9. Logged angle scaling: `logged_value / 100 = degrees`

---

## 3. CSV Format

The UART DMA log uses the following format:

```text
START
counter,roll,pitch,roll_acc,pitch_acc,roll_gyro,pitch_gyro
## 4. Stationary Validation Results

For stationary tests, the first few seconds were ignored to remove estimator startup settling. Mean and standard deviation were calculated from the stable region.

The mean value shows the estimated static angle, while the standard deviation shows the noise/stability of the estimator output.

### Stationary Dataset 1 — Flat / Slight Tilt

| Signal | Mean [deg] | Std Dev [deg] |
|---|---:|---:|
| roll | 0.384 | 0.010 |
| pitch | 2.821 | 0.008 |
| roll_acc | 0.338 | 0.015 |
| pitch_acc | 2.809 | 0.014 |
| roll_gyro | 0.385 | 0.010 |
| pitch_gyro | 2.821 | 0.008 |

### Stationary Dataset 2 — Negative Roll Tilt

| Signal | Mean [deg] | Std Dev [deg] |
|---|---:|---:|
| roll | -27.04 | 0.075 |
| pitch | 3.09 | 0.039 |
| roll_acc | -27.09 | 0.075 |
| pitch_acc | 3.09 | 0.042 |
| roll_gyro | -27.04 | 0.075 |
| pitch_gyro | 3.09 | 0.039 |

### Stationary Dataset 3 — Positive Roll Tilt

Stable final region only:

| Signal | Mean [deg] | Std Dev [deg] |
|---|---:|---:|
| roll | 42.93 | 0.050 |
| pitch | 3.33 | 0.020 |

### Interpretation

The stationary results show that the complementary filter output is stable. In the flat/slight-tilt test, the complementary filter reduced noise compared with the accelerometer-only angle:

- roll standard deviation improved from 0.015° to 0.010°
- pitch standard deviation improved from 0.014° to 0.008°

The tilted tests also show stable behavior, with no missing samples after the `START` marker. These results are good enough to use the complementary filter as the baseline before implementing the Kalman filter.
