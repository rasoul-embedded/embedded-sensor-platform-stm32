/* stm32f407xx.h
 *
 *  Created on: Jan 18, 2026
 *      Author: Rasoul
 *
 *  Purpose:
 *  Low-level device header for STM32F407xx
 *  Defines memory map, peripheral base addresses,
 *  register structures, and common macros.
 */

#ifndef INC_STM32F407XX_H_
#define INC_STM32F407XX_H_

#include <stdint.h>

#define __vo volatile

#if !defined(__weak)
  #if defined(__GNUC__)
    #define __weak __attribute__((weak))
  #else
    #define __weak
  #endif
#endif

/*********************************** Processor Specific Details ***********************************/
/*
 * ARM Cortex-M4 Processor NVIC ISERx register addresses
 */
#define NVIC_ISER0              ((__vo uint32_t*)0xE000E100UL)
#define NVIC_ISER1              ((__vo uint32_t*)0xE000E104UL)
#define NVIC_ISER2              ((__vo uint32_t*)0xE000E108UL)
#define NVIC_ISER3              ((__vo uint32_t*)0xE000E10CUL)

/*
 * ARM Cortex-M4 Processor NVIC ICERx register addresses
 */
#define NVIC_ICER0              ((__vo uint32_t*)0xE000E180UL)
#define NVIC_ICER1              ((__vo uint32_t*)0xE000E184UL)
#define NVIC_ICER2              ((__vo uint32_t*)0xE000E188UL)
#define NVIC_ICER3              ((__vo uint32_t*)0xE000E18CUL)

/*
 * ARM Cortex-M4 Processor Priority register base address (IPR)
 */
#define NVIC_IPR_BASE_ADDR      ((__vo uint32_t*)0xE000E400UL)
#define NVIC_PR_BASE_ADDR       NVIC_IPR_BASE_ADDR

#define NO_PR_BITS_IMPLEMENTED  4U

/****************************************
 *        Base Address of memories      *
 ****************************************/
#define FLASH_BASE_ADDR         0x08000000UL
#define SRAM1_BASE_ADDR         0x20000000UL
#define SRAM2_BASE_ADDR         0x2001C000UL
#define SRAM_BASE_ADDR          SRAM1_BASE_ADDR
#define ROM_BASE_ADDR           0x1FFF0000UL
#define CCMRAM_BASE_ADDR        0x10000000UL

/****************************************
 *        Peripheral base addresses     *
 ****************************************/
#define PERIPH_BASE_ADDR        0x40000000UL
#define APB1PERIPH_BASE_ADDR    PERIPH_BASE_ADDR
#define APB2PERIPH_BASE_ADDR    0x40010000UL
#define AHB1PERIPH_BASE_ADDR    0x40020000UL
#define AHB2PERIPH_BASE_ADDR    0x50000000UL

/****************************************
 *             AHB1 peripherals         *
 ****************************************/
#define GPIOA_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x0000UL)
#define GPIOB_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x0400UL)
#define GPIOC_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x0800UL)
#define GPIOD_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x0C00UL)
#define GPIOE_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x1000UL)
#define GPIOF_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x1400UL)
#define GPIOG_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x1800UL)
#define GPIOH_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x1C00UL)
#define GPIOI_BASE_ADDR         (AHB1PERIPH_BASE_ADDR + 0x2000UL)

#define RCC_BASE_ADDR           (AHB1PERIPH_BASE_ADDR + 0x3800UL)

/****************************************
 *             APB1 peripherals         *
 ****************************************/
#define SPI2_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x3800UL)
#define SPI3_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x3C00UL)

#define USART2_BASE_ADDR        (APB1PERIPH_BASE_ADDR + 0x4400UL)
#define USART3_BASE_ADDR        (APB1PERIPH_BASE_ADDR + 0x4800UL)
#define UART4_BASE_ADDR         (APB1PERIPH_BASE_ADDR + 0x4C00UL)
#define UART5_BASE_ADDR         (APB1PERIPH_BASE_ADDR + 0x5000UL)

#define I2C1_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x5400UL)
#define I2C2_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x5800UL)
#define I2C3_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x5C00UL)

#define TIM2_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x0000UL)
#define TIM3_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x0400UL)
#define TIM4_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x0800UL)
#define TIM5_BASE_ADDR          (APB1PERIPH_BASE_ADDR + 0x0C00UL)



/****************************************
 *             APB2 peripherals         *
 ****************************************/
#define USART1_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x1000UL)
#define USART6_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x1400UL)

#define SPI1_BASE_ADDR          (APB2PERIPH_BASE_ADDR + 0x3000UL)

#define SYSCFG_BASE_ADDR        (APB2PERIPH_BASE_ADDR + 0x3800UL)
#define EXTI_BASE_ADDR          (APB2PERIPH_BASE_ADDR + 0x3C00UL)

/***************************************************************************************
 *                  Peripheral Register Definition Structures
 ***************************************************************************************/
typedef struct
{
    __vo uint32_t MODER;
    __vo uint32_t OTYPER;
    __vo uint32_t OSPEEDR;
    __vo uint32_t PUPDR;
    __vo uint32_t IDR;
    __vo uint32_t ODR;
    __vo uint32_t BSRR;
    __vo uint32_t LCKR;
    __vo uint32_t AFR[2];
} GPIO_RegDef_t;

typedef struct
{
    __vo uint32_t CR1;
    __vo uint32_t CR2;
    __vo uint32_t SR;
    __vo uint32_t DR;
    __vo uint32_t CRCPR;
    __vo uint32_t RXCRCR;
    __vo uint32_t TXCRCR;
    __vo uint32_t I2SCFGR;
    __vo uint32_t I2SPR;
} SPI_RegDef_t;

typedef struct
{
    __vo uint32_t CR1;
    __vo uint32_t CR2;
    __vo uint32_t OAR1;
    __vo uint32_t OAR2;
    __vo uint32_t DR;
    __vo uint32_t SR1;
    __vo uint32_t SR2;
    __vo uint32_t CCR;
    __vo uint32_t TRISE;
    __vo uint32_t FLTR;
} I2C_RegDef_t;

typedef struct
{
    __vo uint32_t SR;
    __vo uint32_t DR;
    __vo uint32_t BRR;
    __vo uint32_t CR1;
    __vo uint32_t CR2;
    __vo uint32_t CR3;
    __vo uint32_t GTPR;
} USART_RegDef_t;

typedef struct
{
    __vo uint32_t CR;
    __vo uint32_t PLLCFGR;
    __vo uint32_t CFGR;
    __vo uint32_t CIR;
    __vo uint32_t AHB1RSTR;
    __vo uint32_t AHB2RSTR;
    __vo uint32_t AHB3RSTR;
    uint32_t RESERVED0;
    __vo uint32_t APB1RSTR;
    __vo uint32_t APB2RSTR;
    uint32_t RESERVED1[2];
    __vo uint32_t AHB1ENR;
    __vo uint32_t AHB2ENR;
    __vo uint32_t AHB3ENR;
    uint32_t RESERVED2;
    __vo uint32_t APB1ENR;
    __vo uint32_t APB2ENR;
    uint32_t RESERVED3[2];
    __vo uint32_t AHB1LPENR;
    __vo uint32_t AHB2LPENR;
    __vo uint32_t AHB3LPENR;
    uint32_t RESERVED4;
    __vo uint32_t APB1LPENR;
    __vo uint32_t APB2LPENR;
    uint32_t RESERVED5[2];
    __vo uint32_t BDCR;
    __vo uint32_t CSR;
    uint32_t RESERVED6[2];
    __vo uint32_t SSCGR;
    __vo uint32_t PLLI2SCFGR;
    __vo uint32_t PLLSAICFGR;
    __vo uint32_t DCKCFGR;
} RCC_RegDef_t;

typedef struct
{
    __vo uint32_t IMR;
    __vo uint32_t EMR;
    __vo uint32_t RTSR;
    __vo uint32_t FTSR;
    __vo uint32_t SWIER;
    __vo uint32_t PR;
} EXTI_RegDef_t;

typedef struct
{
    __vo uint32_t MEMRMP;
    __vo uint32_t PMC;
    __vo uint32_t EXTICR[4];
    __vo uint32_t CMPCR;
} SYSCFG_RegDef_t;


typedef struct
{
    __vo uint32_t CR1;
    __vo uint32_t CR2;
    __vo uint32_t SMCR;
    __vo uint32_t DIER;
    __vo uint32_t SR;
    __vo uint32_t EGR;
    __vo uint32_t CCMR1;
    __vo uint32_t CCMR2;
    __vo uint32_t CCER;
    __vo uint32_t CNT;
    __vo uint32_t PSC;
    __vo uint32_t ARR;
    __vo uint32_t RESERVED1;
    __vo uint32_t CCR1;
    __vo uint32_t CCR2;
    __vo uint32_t CCR3;
    __vo uint32_t CCR4;
    __vo uint32_t RESERVED2;
    __vo uint32_t DCR;
    __vo uint32_t DMAR;
    __vo uint32_t OR;
} TIM_RegDef_t;

/***********************************************************
 *          Peripheral definitions (typecasted)            *
 ***********************************************************/
#define GPIOA                   ((GPIO_RegDef_t*)GPIOA_BASE_ADDR)
#define GPIOB                   ((GPIO_RegDef_t*)GPIOB_BASE_ADDR)
#define GPIOC                   ((GPIO_RegDef_t*)GPIOC_BASE_ADDR)
#define GPIOD                   ((GPIO_RegDef_t*)GPIOD_BASE_ADDR)
#define GPIOE                   ((GPIO_RegDef_t*)GPIOE_BASE_ADDR)
#define GPIOF                   ((GPIO_RegDef_t*)GPIOF_BASE_ADDR)
#define GPIOG                   ((GPIO_RegDef_t*)GPIOG_BASE_ADDR)
#define GPIOH                   ((GPIO_RegDef_t*)GPIOH_BASE_ADDR)
#define GPIOI                   ((GPIO_RegDef_t*)GPIOI_BASE_ADDR)

#define SPI1                    ((SPI_RegDef_t*)SPI1_BASE_ADDR)
#define SPI2                    ((SPI_RegDef_t*)SPI2_BASE_ADDR)
#define SPI3                    ((SPI_RegDef_t*)SPI3_BASE_ADDR)

#define I2C1                    ((I2C_RegDef_t*)I2C1_BASE_ADDR)
#define I2C2                    ((I2C_RegDef_t*)I2C2_BASE_ADDR)
#define I2C3                    ((I2C_RegDef_t*)I2C3_BASE_ADDR)

#define USART1                  ((USART_RegDef_t*)USART1_BASE_ADDR)
#define USART2                  ((USART_RegDef_t*)USART2_BASE_ADDR)
#define USART3                  ((USART_RegDef_t*)USART3_BASE_ADDR)
#define UART4                   ((USART_RegDef_t*)UART4_BASE_ADDR)
#define UART5                   ((USART_RegDef_t*)UART5_BASE_ADDR)
#define USART6                  ((USART_RegDef_t*)USART6_BASE_ADDR)

#define RCC                     ((RCC_RegDef_t*)RCC_BASE_ADDR)
#define EXTI                    ((EXTI_RegDef_t*)EXTI_BASE_ADDR)
#define SYSCFG                  ((SYSCFG_RegDef_t*)SYSCFG_BASE_ADDR)

#define TIM2                    ((TIM_RegDef_t*)TIM2_BASE_ADDR)
#define TIM3                    ((TIM_RegDef_t*)TIM3_BASE_ADDR)
#define TIM4                    ((TIM_RegDef_t*)TIM4_BASE_ADDR)
#define TIM5                    ((TIM_RegDef_t*)TIM5_BASE_ADDR)



/***********************************************************
 *                 Clock enable macros                     *
 ***********************************************************/
/* GPIO */
#define GPIOA_PCLK_EN()         (RCC->AHB1ENR |=  (1U << 0))
#define GPIOB_PCLK_EN()         (RCC->AHB1ENR |=  (1U << 1))
#define GPIOC_PCLK_EN()         (RCC->AHB1ENR |=  (1U << 2))
#define GPIOD_PCLK_EN()         (RCC->AHB1ENR |=  (1U << 3))
#define GPIOE_PCLK_EN()         (RCC->AHB1ENR |=  (1U << 4))
#define GPIOF_PCLK_EN()         (RCC->AHB1ENR |=  (1U << 5))
#define GPIOG_PCLK_EN()         (RCC->AHB1ENR |=  (1U << 6))
#define GPIOH_PCLK_EN()         (RCC->AHB1ENR |=  (1U << 7))
#define GPIOI_PCLK_EN()         (RCC->AHB1ENR |=  (1U << 8))

/* SPI */
#define SPI1_PCLK_EN()          (RCC->APB2ENR |=  (1U << 12))
#define SPI2_PCLK_EN()          (RCC->APB1ENR |=  (1U << 14))
#define SPI3_PCLK_EN()          (RCC->APB1ENR |=  (1U << 15))

/* I2C */
#define I2C1_PCLK_EN()          (RCC->APB1ENR |=  (1U << 21))
#define I2C2_PCLK_EN()          (RCC->APB1ENR |=  (1U << 22))
#define I2C3_PCLK_EN()          (RCC->APB1ENR |=  (1U << 23))

/* USART / UART */
#define USART1_PCLK_EN()        (RCC->APB2ENR |=  (1U << 4))
#define USART2_PCLK_EN()        (RCC->APB1ENR |=  (1U << 17))
#define USART3_PCLK_EN()        (RCC->APB1ENR |=  (1U << 18))
#define UART4_PCLK_EN()         (RCC->APB1ENR |=  (1U << 19))
#define UART5_PCLK_EN()         (RCC->APB1ENR |=  (1U << 20))
#define USART6_PCLK_EN()        (RCC->APB2ENR |=  (1U << 5))

/* SYSCFG */
#define SYSCFG_PCLK_EN()        (RCC->APB2ENR |=  (1U << 14))

/* TIM */
#define TIM2_PCLK_EN()          (RCC->APB1ENR |=  (1U << 0))
#define TIM3_PCLK_EN()          (RCC->APB1ENR |=  (1U << 1))
#define TIM4_PCLK_EN()          (RCC->APB1ENR |=  (1U << 2))
#define TIM5_PCLK_EN()          (RCC->APB1ENR |=  (1U << 3))



/***********************************************************
 *                 Clock disable macros                    *
 ***********************************************************/
/* GPIO */
#define GPIOA_PCLK_DI()         (RCC->AHB1ENR &= ~(1U << 0))
#define GPIOB_PCLK_DI()         (RCC->AHB1ENR &= ~(1U << 1))
#define GPIOC_PCLK_DI()         (RCC->AHB1ENR &= ~(1U << 2))
#define GPIOD_PCLK_DI()         (RCC->AHB1ENR &= ~(1U << 3))
#define GPIOE_PCLK_DI()         (RCC->AHB1ENR &= ~(1U << 4))
#define GPIOF_PCLK_DI()         (RCC->AHB1ENR &= ~(1U << 5))
#define GPIOG_PCLK_DI()         (RCC->AHB1ENR &= ~(1U << 6))
#define GPIOH_PCLK_DI()         (RCC->AHB1ENR &= ~(1U << 7))
#define GPIOI_PCLK_DI()         (RCC->AHB1ENR &= ~(1U << 8))

/* SPI */
#define SPI1_PCLK_DI()          (RCC->APB2ENR &= ~(1U << 12))
#define SPI2_PCLK_DI()          (RCC->APB1ENR &= ~(1U << 14))
#define SPI3_PCLK_DI()          (RCC->APB1ENR &= ~(1U << 15))

/* I2C */
#define I2C1_PCLK_DI()          (RCC->APB1ENR &= ~(1U << 21))
#define I2C2_PCLK_DI()          (RCC->APB1ENR &= ~(1U << 22))
#define I2C3_PCLK_DI()          (RCC->APB1ENR &= ~(1U << 23))

/* USART / UART */
#define USART1_PCLK_DI()        (RCC->APB2ENR &= ~(1U << 4))
#define USART2_PCLK_DI()        (RCC->APB1ENR &= ~(1U << 17))
#define USART3_PCLK_DI()        (RCC->APB1ENR &= ~(1U << 18))
#define UART4_PCLK_DI()         (RCC->APB1ENR &= ~(1U << 19))
#define UART5_PCLK_DI()         (RCC->APB1ENR &= ~(1U << 20))
#define USART6_PCLK_DI()        (RCC->APB2ENR &= ~(1U << 5))

/* SYSCFG */
#define SYSCFG_PCLK_DI()        (RCC->APB2ENR &= ~(1U << 14))

/* TIM */
#define TIM2_PCLK_DI()          (RCC->APB1ENR &= ~(1U << 0))
#define TIM3_PCLK_DI()          (RCC->APB1ENR &= ~(1U << 1))
#define TIM4_PCLK_DI()          (RCC->APB1ENR &= ~(1U << 2))
#define TIM5_PCLK_DI()          (RCC->APB1ENR &= ~(1U << 3))



/***********************************************************
 *                 Register reset macros                   *
 ***********************************************************/
/* GPIO */
#define GPIOA_REG_RESET()       do { RCC->AHB1RSTR |=  (1U << 0); RCC->AHB1RSTR &= ~(1U << 0); } while (0)
#define GPIOB_REG_RESET()       do { RCC->AHB1RSTR |=  (1U << 1); RCC->AHB1RSTR &= ~(1U << 1); } while (0)
#define GPIOC_REG_RESET()       do { RCC->AHB1RSTR |=  (1U << 2); RCC->AHB1RSTR &= ~(1U << 2); } while (0)
#define GPIOD_REG_RESET()       do { RCC->AHB1RSTR |=  (1U << 3); RCC->AHB1RSTR &= ~(1U << 3); } while (0)
#define GPIOE_REG_RESET()       do { RCC->AHB1RSTR |=  (1U << 4); RCC->AHB1RSTR &= ~(1U << 4); } while (0)
#define GPIOF_REG_RESET()       do { RCC->AHB1RSTR |=  (1U << 5); RCC->AHB1RSTR &= ~(1U << 5); } while (0)
#define GPIOG_REG_RESET()       do { RCC->AHB1RSTR |=  (1U << 6); RCC->AHB1RSTR &= ~(1U << 6); } while (0)
#define GPIOH_REG_RESET()       do { RCC->AHB1RSTR |=  (1U << 7); RCC->AHB1RSTR &= ~(1U << 7); } while (0)
#define GPIOI_REG_RESET()       do { RCC->AHB1RSTR |=  (1U << 8); RCC->AHB1RSTR &= ~(1U << 8); } while (0)

/* SPI */
#define SPI1_REG_RESET()        do { RCC->APB2RSTR |=  (1U << 12); RCC->APB2RSTR &= ~(1U << 12); } while (0)
#define SPI2_REG_RESET()        do { RCC->APB1RSTR |=  (1U << 14); RCC->APB1RSTR &= ~(1U << 14); } while (0)
#define SPI3_REG_RESET()        do { RCC->APB1RSTR |=  (1U << 15); RCC->APB1RSTR &= ~(1U << 15); } while (0)

/* I2C */
#define I2C1_REG_RESET()        do { RCC->APB1RSTR |=  (1U << 21); RCC->APB1RSTR &= ~(1U << 21); } while (0)
#define I2C2_REG_RESET()        do { RCC->APB1RSTR |=  (1U << 22); RCC->APB1RSTR &= ~(1U << 22); } while (0)
#define I2C3_REG_RESET()        do { RCC->APB1RSTR |=  (1U << 23); RCC->APB1RSTR &= ~(1U << 23); } while (0)

/* USART / UART */
#define USART1_REG_RESET()      do { RCC->APB2RSTR |=  (1U << 4); RCC->APB2RSTR &= ~(1U << 4); } while (0)
#define USART2_REG_RESET()      do { RCC->APB1RSTR |=  (1U << 17); RCC->APB1RSTR &= ~(1U << 17); } while (0)
#define USART3_REG_RESET()      do { RCC->APB1RSTR |=  (1U << 18); RCC->APB1RSTR &= ~(1U << 18); } while (0)
#define UART4_REG_RESET()       do { RCC->APB1RSTR |=  (1U << 19); RCC->APB1RSTR &= ~(1U << 19); } while (0)
#define UART5_REG_RESET()       do { RCC->APB1RSTR |=  (1U << 20); RCC->APB1RSTR &= ~(1U << 20); } while (0)
#define USART6_REG_RESET()      do { RCC->APB2RSTR |=  (1U << 5); RCC->APB2RSTR &= ~(1U << 5); } while (0)

/* SYSCFG */
#define SYSCFG_REG_RESET()      do { RCC->APB2RSTR |=  (1U << 14); RCC->APB2RSTR &= ~(1U << 14); } while (0)

/* TIM */
#define TIM2_REG_RESET()        do { RCC->APB1RSTR |=  (1U << 0); RCC->APB1RSTR &= ~(1U << 0); } while (0)
#define TIM3_REG_RESET()        do { RCC->APB1RSTR |=  (1U << 1); RCC->APB1RSTR &= ~(1U << 1); } while (0)
#define TIM4_REG_RESET()        do { RCC->APB1RSTR |=  (1U << 2); RCC->APB1RSTR &= ~(1U << 2); } while (0)
#define TIM5_REG_RESET()        do { RCC->APB1RSTR |=  (1U << 3); RCC->APB1RSTR &= ~(1U << 3); } while (0)




/***********************************************************
 *                 Generic helper macros                   *
 ***********************************************************/
#define GPIO_BASEADDR_TO_CODE(x) \
    (((x) == GPIOA) ? 0U : \
     ((x) == GPIOB) ? 1U : \
     ((x) == GPIOC) ? 2U : \
     ((x) == GPIOD) ? 3U : \
     ((x) == GPIOE) ? 4U : \
     ((x) == GPIOF) ? 5U : \
     ((x) == GPIOG) ? 6U : \
     ((x) == GPIOH) ? 7U : \
     ((x) == GPIOI) ? 8U : 0U)

#define ENABLE                  1U
#define DISABLE                 0U
#define SET                     ENABLE
#define RESET                   DISABLE
#define FLAG_SET                SET
#define FLAG_RESET              RESET

#define GPIO_PIN_SET            SET
#define GPIO_PIN_RESET          RESET

#define I2C_DISABLE_SR          DISABLE
#define I2C_ENABLE_SR           ENABLE

/***********************************************************
 *                 IRQ numbers (NVIC)                      *
 ***********************************************************/
/* EXTI */
#define IRQ_NO_EXTI0            6U
#define IRQ_NO_EXTI1            7U
#define IRQ_NO_EXTI2            8U
#define IRQ_NO_EXTI3            9U
#define IRQ_NO_EXTI4            10U
#define IRQ_NO_EXTI9_5          23U
#define IRQ_NO_EXTI15_10        40U

/* I2C */
#define IRQ_NO_I2C1_EV          31U
#define IRQ_NO_I2C1_ER          32U
#define IRQ_NO_I2C2_EV          33U
#define IRQ_NO_I2C2_ER          34U
#define IRQ_NO_I2C3_EV          72U
#define IRQ_NO_I2C3_ER          73U

/* USART / UART */
#define IRQ_NO_USART1           37U
#define IRQ_NO_USART2           38U
#define IRQ_NO_USART3           39U
#define IRQ_NO_UART4            52U
#define IRQ_NO_UART5            53U
#define IRQ_NO_USART6           71U


/* TIM */
#define IRQ_NO_TIM2             28U



#endif /* INC_STM32F407XX_H_ */
