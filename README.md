STM32F407 Discovery + LSM6DSO32 IMU Bring-Up Month 1 – Embedded Foundations, Peripheral Drivers, and Sensor Data Streaming Overview.
The work in this first month focuses on building low-level embedded fundamentals by developing and using custom drivers for:
GPIO,I2C,USART, and TIM.These drivers are then integrated to initialize and communicate with the LSM6DSO32 inertial sensor, acquire accelerometer and gyroscope data at a fixed interval, and stream the measured values over UART.
The goal of this phase is not only to make the sensor work, but to understand the complete embedded path from peripheral configuration to real-time data acquisition.
Month 1 Objectives: Set up the STM32F407 as an embedded development platform, Build familiarity with register-level peripheral programming, Develop and use custom drivers for GPIO, I2C, USART, and TIM, Interface with the LSM6DSO32 IMU using I2C, Use a hardware timer interrupt to trigger periodic sampling, Stream accelerometer and gyroscope data over USART2,Create a clean baseline for later work in calibration, filtering, and sensor fusion.
Hardware Used : STM32F407 Discovery, LSM6DSO32 IMU, Serial terminal over USART2.
External wiring through: PB6 → I2C1_SCL, PB7 → I2C1_SDA, PA2 → USART2_TX, PA3 → USART2_RX. 
Software Architecture: The system is built around four low-level peripheral blocks:
GPIO:GPIO is used to configure alternate-function pins for: I2C1, USART2
I2C1 is used to communicate with the LSM6DSO32 sensor:
Standard modeACK enabledDevice initialization and register access handled through the IMU driver
USART2 is used for serial output:
115200 baud8 data bits1 stop bitNo parityNo hardware flow controlTimer
TIM2 is configured as a periodic time base:
Prescaler and auto-reload configured in the custom timer driverUpdate interrupt sets a sampling flagMain loop reads sensor data when the flag is setProject Structureundefined
Core application│
├── main.c
│
├── Drivers
│   ├── stm32f407xx_gpio_driver.c/.h
│   ├── stm32f407xx_i2c_driver.c/.h
│   ├── stm32f407xx_usart_driver.c/.h
│   ├── stm32f407xx_tim_driver.c/.h
│
├── Sensor
│   ├── lsm6dso32.c/.h
│
└── README.md

Implemented Features1. GPIO Alternate Function ConfigurationConfigured the required pins for peripheral communication:
PB6 / PB7 for I2C1PA2 / PA3 for USART2Key settings used:
Alternate function modeOpen-drain for I2CPush-pull for USARTPull-up on USART pinsFast speed configuration2. I2C1 InitializationConfigured I2C1 with:
ACK enabledStandard mode clock speedDuty cycle setting present for future fast-mode supportLocal device address configured in the driver handleI2C is used by the LSM6DSO32 driver to:
initialize the sensorread accelerometer dataread gyroscope data3. USART2 InitializationConfigured USART2 for terminal output with:
115200 baudTX/RX enabled8-bit word length1 stop bitno parityno hardware flow controlUSART is currently used for:
initialization status messagesstreaming IMU measurementsExample serial output:
undefined
LSM6DSO32 init OK
AX=12 AY=-8 AZ=998 | GX=15 GY=-2 GZ=6
AX=10 AY=-9 AZ=1001 | GX=14 GY=-3 GZ=5
4. TIM2 Periodic SamplingTIM2 is used to generate a periodic update interrupt.
Current timer flow:
TIM2 update interrupt
        ↓
sample_flag = 1
        ↓
main loop detects flag
        ↓
read accelerometer + gyroscope
        ↓
format message
        ↓
send data over USART2


This separates:
timing event generation fromsensor reading and UART transmissionwhich is a better embedded design than doing everything inside the ISR.
5. LSM6DSO32 Sensor IntegrationThe LSM6DSO32 driver is used to:
initialize the sensorread acceleration in gread angular rate in dpsData is scaled before printing:
accelerometer values multiplied by 1000gyroscope values multiplied by 1000This makes the serial output easier to inspect numerically.
Main Application FlowThe application logic is structured as follows:
Initialize GPIO
Initialize I2C1
Initialize USART2
Initialize TIM2
Initialize LSM6DSO32

If initialization succeeds:
    start TIM2
    print status message

Main loop:
    wait for sample_flag
    read accel and gyro
    format data
    send over USART2


Interrupt StrategyThe timer interrupt handler is intentionally minimal:
void TIM2_IRQHandler(void)
{
    if (tim2Handle.pTIMx->SR & TIM_SR_UIF)
    {
        tim2Handle.pTIMx->SR &= ~TIM_SR_UIF;
        sample_flag = 1;
    }
}


This design keeps ISR execution short and avoids doing expensive operations such as:
I2C transactionssprintfUART transmissioninside the interrupt itself.
Driver-Level Learning Goals AchievedThis project helped build practical understanding in:
memory-mapped peripheral accessalternate function GPIO configurationI2C peripheral setup and sensor communicationUSART configuration for debugging and telemetrytimer base generation and update interruptsinterrupt-driven sampling architecturemodular embedded code organizationDesign DecisionsWhy use a timer interrupt?Using a timer creates a repeatable sampling event and is a first step toward deterministic sensor acquisition.
Why use a flag instead of reading inside the ISR?I2C reads and formatted UART output are relatively slow compared with ISR execution.Keeping the interrupt short improves system behavior and is closer to good embedded practice.
Why stream raw sensor data first?Before calibration, filtering, or fusion, raw data streaming is necessary to:
verify communicationinspect axis behaviorvalidate scalingconfirm the sensor is alive and respondingCurrent LimitationsAt this stage, the system is functional but still basic.
Functional limitationsno calibration yetno digital filtering yetno sensor fusion yetno DMAno ring buffer for serial outputno explicit error recovery during runtimeTiming limitationssampling is timer-triggered only through a software flagactual loop timing is affected by:I2C transaction timesprintfUSART transmit timeSo this is a periodic acquisition framework, but not yet a fully optimized real-time data pipeline.
What Was Learned from Month 1This month established the embedded foundation required for later work.The key outcome was moving from simple peripheral experiments to a working multi-peripheral sensor system.
By the end of this stage, the system can:
initialize a real IMU sensorperiodically acquire accelerometer and gyroscope datastream measurements over serialrely on custom-written low-level peripheral driversThis forms the base for more advanced work in:
calibrationdeterministic sampling analysisdigital filteringcomplementary and Kalman filteringvibration and motion analysisNext StepsPlanned work for Month 2:
validate the true sampling periodimprove timing structureperform accelerometer and gyroscope bias calibrationimplement digital low-pass filteringestimate roll and pitchbuild a complementary filterimprove repository structure and documentationAuthor BackgroundThis project is part of a transition toward embedded systems and sensor-based engineering, building on prior experience in:
mechatronics and roboticscontrol and estimationsignal processingoptical and vibration measurement systems
