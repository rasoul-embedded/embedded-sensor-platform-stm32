# Roll and Pitch Kalman Validation

## 1. Objective

The objective is to extend the reusable 1D Kalman filter from roll-only estimation to both roll and pitch, then compare the Kalman outputs against the previously validated complementary filter outputs.

The main validation questions were:

1. Do `roll_kalman` and `pitch_kalman` have the correct sign?
2. Do they stay close to `roll_comp` and `pitch_comp`?
3. Are they stable in stationary positions?
4. Do they follow dynamic roll/pitch motion without obvious delay, overshoot, or divergence?
5. Is the implementation stable enough to use in a demonstrable IMU application?

---

## 2. Firmware Configuration

1. Board: STM32F407 Discovery
2. IMU: LSM6DSO32
3. Sampling rate: 100 Hz
4. Sampling period: 0.01 s
5. UART logging: USART2 TX using DMA
6. DMA mapping: DMA1 Stream 6 Channel 4
7. Baseline estimator: complementary filter
8. Comparison estimator: 1D Kalman filter for roll and pitch

The Kalman filter module is reusable:

```text
Core/Inc/kalman_1d.h
Core/Src/kalman_1d.c
```

The roll and pitch Kalman outputs are integrated into:

```text
Inc/imu_estimator.h
Src/imu_estimator.c
```

---

## 3. CSV Logging Format

Validation log format was:

```text
START
counter,roll_comp,roll_kalman,pitch_comp,pitch_kalman
```

Column meaning:

1. `counter` — sample counter
2. `roll_comp` — complementary filter roll output
3. `roll_kalman` — Kalman filter roll output
4. `pitch_comp` — complementary filter pitch output
5. `pitch_kalman` — Kalman filter pitch output

All angle values are scaled by 100 in the firmware.

Example:

```text
2713 = 27.13 degrees
```

Only data after the final `START` marker was used for analysis.

---

## 4. Stationary Validation Results

Stationary datasets were collected for negative roll, positive pitch, and positive roll positions. The first 300 samples were ignored to remove estimator startup settling.

| Test           | Signal       |   Mean [deg] |   Std Dev [deg] |   Min [deg] |   Max [deg] |
|:---------------|:-------------|-------------:|----------------:|------------:|------------:|
| Negative roll  | roll_comp    |      -30.51  |           0.202 |      -30.97 |      -30.09 |
| Negative roll  | roll_kalman  |      -30.54  |           0.234 |      -32.04 |      -30.08 |
| Negative roll  | pitch_comp   |        4.625 |           0.025 |        4.56 |        4.67 |
| Negative roll  | pitch_kalman |        4.603 |           0.026 |        4.54 |        4.69 |
| Positive pitch | roll_comp    |        3.081 |           0.018 |        3.02 |        3.11 |
| Positive pitch | roll_kalman  |        3.055 |           0.018 |        3.01 |        3.1  |
| Positive pitch | pitch_comp   |       19.257 |           0.075 |       19.15 |       19.4  |
| Positive pitch | pitch_kalman |       19.238 |           0.095 |       19.12 |       20.01 |
| Positive roll  | roll_comp    |       27.211 |           0.066 |       27.11 |       27.35 |
| Positive roll  | roll_kalman  |       27.195 |           0.101 |       27.08 |       28.22 |
| Positive roll  | pitch_comp   |        1.41  |           0.008 |        1.39 |        1.43 |
| Positive roll  | pitch_kalman |        1.386 |           0.01  |        1.36 |        1.42 |

### Stationary interpretation

The stationary results show that both `roll_kalman` and `pitch_kalman` stay close to the complementary filter outputs. The signs are correct for positive and negative roll, and the pitch Kalman output is stable during the positive pitch test.

The standard deviations are small in the stable regions, which means the Kalman implementation is stable for stationary roll and pitch estimation.

---

## 5. Dynamic Validation Results

Dynamic datasets were collected for combined roll/pitch movement, positive pitch movement, positive roll movement, and negative roll movement. The first 300 samples were ignored during summary calculation.

| Test                   | Signal       |   Mean [deg] |   Std Dev [deg] |   Min [deg] |   Max [deg] |
|:-----------------------|:-------------|-------------:|----------------:|------------:|------------:|
| Combined dynamic       | roll_comp    |       -4.862 |          10.253 |      -26.43 |       20.1  |
| Combined dynamic       | roll_kalman  |       -4.914 |          10.286 |      -26.69 |       20.12 |
| Combined dynamic       | pitch_comp   |        3.891 |          12.154 |      -24.87 |       26.59 |
| Combined dynamic       | pitch_kalman |        3.863 |          12.137 |      -24.87 |       26.54 |
| Dynamic positive pitch | roll_comp    |       -1.602 |           0.914 |       -3.25 |        0.01 |
| Dynamic positive pitch | roll_kalman  |       -1.6   |           0.919 |       -3.62 |        0.15 |
| Dynamic positive pitch | pitch_comp   |        7.237 |           9.087 |       -2.8  |       21.15 |
| Dynamic positive pitch | pitch_kalman |        7.104 |           9.16  |       -2.8  |       21.13 |
| Dynamic positive roll  | roll_comp    |        4.138 |           6.418 |        0.32 |       18.32 |
| Dynamic positive roll  | roll_kalman  |        4.072 |           6.448 |        0.3  |       18.31 |
| Dynamic positive roll  | pitch_comp   |        2.274 |           0.843 |        0.58 |        2.92 |
| Dynamic positive roll  | pitch_kalman |        2.251 |           0.843 |        0.56 |        2.99 |
| Dynamic negative roll  | roll_comp    |       -6.747 |           9.121 |      -23.46 |        0.45 |
| Dynamic negative roll  | roll_kalman  |       -6.691 |           9.166 |      -23.44 |        0.47 |
| Dynamic negative roll  | pitch_comp   |        1.304 |           1.3   |       -1.64 |        2.52 |
| Dynamic negative roll  | pitch_kalman |        1.321 |           1.293 |       -1.67 |        2.57 |

---

## 6. Dynamic Comparison Metrics

Mean absolute difference was calculated between complementary and Kalman outputs. Lag was estimated using cross-correlation. A lag close to zero means no meaningful delay was detected between the complementary and Kalman signals.

| Test                   | File    |   Roll MAD [deg] |   Pitch MAD [deg] |   Roll max diff [deg] |   Pitch max diff [deg] |   Roll lag [samples] |   Pitch lag [samples] |   Roll lag [ms] |   Pitch lag [ms] |   Roll corr |   Pitch corr |
|:-----------------------|:--------|-----------------:|------------------:|----------------------:|-----------------------:|---------------------:|----------------------:|----------------:|-----------------:|------------:|-------------:|
| Combined dynamic       | dc.csv  |            0.114 |             0.055 |                  0.42 |                   0.31 |                    0 |                     0 |               0 |                0 |       1     |        1     |
| Dynamic positive pitch | dpp.csv |            0.08  |             0.163 |                  0.67 |                   0.59 |                   -4 |                     0 |             -40 |                0 |       0.991 |        1     |
| Dynamic positive roll  | dr.csv  |            0.074 |             0.025 |                  1.19 |                   0.07 |                    0 |                     0 |               0 |                0 |       1     |        1     |
| Dynamic negative roll  | drn.csv |            0.107 |             0.046 |                  2.22 |                   0.64 |                   -1 |                    -3 |             -10 |              -30 |       0.999 |        0.997 |

### Dynamic interpretation

The dynamic datasets show that `roll_kalman` and `pitch_kalman` follow the complementary filter outputs closely during motion.

Key observations:

1. Correct sign was observed for positive and negative roll movement.
2. Pitch Kalman followed positive pitch motion correctly.
3. The mean absolute difference between complementary and Kalman outputs was small, generally below 0.2 degrees.
4. No divergence or unstable behavior was observed.
5. No missing samples, duplicate counters, or malformed data were found after the final `START` marker.

---

## 7. Delay, Overshoot, Noise, Return-to-Zero, and Hold Stability

### Delay

The Kalman outputs followed the complementary outputs closely. Cross-correlation showed no meaningful delay in the tested datasets. Any estimated lag was very small and not visually significant for this 100 Hz application.

### Overshoot

No strong overshoot was observed. The Kalman outputs remained close to the complementary outputs during positive roll, negative roll, positive pitch, and combined motion tests.

### Noise

In stationary tests, the Kalman outputs were stable. The complementary filter was sometimes slightly smoother, but the Kalman filter remained close and did not amplify noise in a problematic way.

### Return-to-zero behavior

In the dynamic roll tests, the estimates returned close to the initial flat/slight-tilt region after motion. The final regions of `dr.csv` and `drn.csv` were stable and close to zero roll.

### Stability during hold

Stable hold behavior was observed in the stationary files and in the final regions of dynamic roll tests. The combined dynamic file still contained motion near the end, so its final region is not treated as a stationary hold metric.

---

## 8. Selected Kalman Parameters

The validation used the baseline Kalman parameters:

```c
#define KALMAN_Q_ANGLE      0.001f
#define KALMAN_Q_BIAS       0.003f
#define KALMAN_R_MEASURE    0.03f
```

Parameter meaning
1. `Q_angle` — process noise for angle prediction
2. `Q_bias` — process noise for gyro bias estimation
3. `R_measure` — accelerometer angle measurement noise

The parameters are stable enough for the current IMU setup. Further tuning may improve the noise/response tradeoff, but it is not required before moving to the Week 4 demo stage.

