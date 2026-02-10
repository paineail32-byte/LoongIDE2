/*
 * ls2k301_board_gpio_test.c
 *
 * created: 2025-11-13
 *  author: 
 */

/**
 * 用于测试 2K301 开发板的排针正确性
 */

#include "osal.h"

#include "ls2k300.h"
#include "ls2k300_irq.h"

#include "ls2k_gpio.h"

#define TEST_WITH_ISR_1     0
#define TEST_WITH_ISR_2     0

#define TEST_ALL            1

#define TEST_48_50_1        0
#define TEST_48_50_2        1       /* 有问题的GPIO */

//-----------------------------------------------------------------------------

#if !TEST_ALL

static void gpio_xxx_isr(int gpioNum, void *arg)
{
    printk("%i\n", gpioNum);
}

/*
 * 安装中断向量
 */
#define GPIO_IN_WITH_ISR(x)  do {       \
    gpio_enable(x, DIR_IN );            \
    ls2k300_gpio_interrupt_disable(x);  \
    ls2k300_gpio_isr_install(x, gpio_xxx_isr, NULL); \
    ls2k300_gpio_interrupt_enable(x, GPIO_INT_TRIG_EDGE_UP); \
} while (0)

#endif // #if !TEST_ALL

//-----------------------------------------------------------------------------

/*
 * set gpio pin usage, TODO Should modify DIRECTION!
 */
static void ls2k_set_pinmux_as_gpio(void)
{
#if TEST_WITH_ISR_1
    /**************************************************
     * J4
     */
    gpio_enable(102,	DIR_OUT);	// Pin: SDIO1_D0
    GPIO_IN_WITH_ISR(103);	        // Pin: SDIO1_D1

    gpio_enable(101,	DIR_OUT);	// Pin: SDIO1_CMD
    GPIO_IN_WITH_ISR(104);	        // Pin: SDIO1_D2

    gpio_enable(100,	DIR_OUT);	// Pin: SDIO1_CLK
    GPIO_IN_WITH_ISR(105);	        // Pin: SDIO1_D3

    gpio_enable(63,		DIR_OUT);	// Pin: SPI1_CS0
    GPIO_IN_WITH_ISR(60);	        // Pin: SPI1_CLK

    gpio_enable(61,		DIR_OUT);	// Pin: SPI1_MISO
    GPIO_IN_WITH_ISR(62);	        // Pin: SPI1_MOSI

    gpio_enable(67,		DIR_OUT);	// Pin: SPI2_CS0
    GPIO_IN_WITH_ISR(64);	        // Pin: SPI2_CLK

    gpio_enable(65,		DIR_OUT);	// Pin: SPI2_MISO
    GPIO_IN_WITH_ISR(66);	        // Pin: SPI2_MOSI

    gpio_enable(20,		DIR_OUT);	// Pin: LCD_R0
    GPIO_IN_WITH_ISR(13);	        // Pin: LCD_G1

    gpio_enable(22,		DIR_OUT);	// Pin: LCD_R2
    GPIO_IN_WITH_ISR(12);	        // Pin: LCD_G0

    gpio_enable(6,		DIR_OUT);	// Pin: LCD_B2
    GPIO_IN_WITH_ISR(21);	        // Pin: LCD_R1

    gpio_enable(4,		DIR_OUT);	// Pin: LCD_B0
    GPIO_IN_WITH_ISR(5);	        // Pin: LCD_B1

    /**************************************************
     * J5
     */
    gpio_enable(54,		DIR_OUT);	// Pin: I2C3_SCL
    GPIO_IN_WITH_ISR(55);	        // Pin: I2C3_SDA

    gpio_enable(52,		DIR_OUT);	// Pin: I2C2_SCL
    GPIO_IN_WITH_ISR(53);	        // Pin: I2C2_SDA

    gpio_enable(50,		DIR_OUT);	// Pin: I2C1_SCL
    GPIO_IN_WITH_ISR(51);	        // Pin: I2C1_SDA

    gpio_enable(48,		DIR_OUT);	// Pin: I2C0_SCL
    GPIO_IN_WITH_ISR(49);	        // Pin: I2C0_SDA

    gpio_enable(77,		DIR_OUT);	// Pin: I2S_BCLK
    GPIO_IN_WITH_ISR(79);	        // Pin: I2S_DI

    gpio_enable(76,		DIR_OUT);	// Pin: I2S_MCLK
    GPIO_IN_WITH_ISR(80);	        // Pin: I2S_DO

    gpio_enable(78,		DIR_OUT);	// Pin: I2S_LR       // XXX
    GPIO_IN_WITH_ISR(89);	        // Pin: TIM2_CH3     // XXX

    gpio_enable(88,		DIR_OUT);	// Pin: TIM2_CH2
    GPIO_IN_WITH_ISR(87);	        // Pin: TIM2_CH1

    gpio_enable(83,		DIR_OUT);	// Pin: TIM1_CH3
    GPIO_IN_WITH_ISR(86);	        // Pin: TIM1_CH3n

    gpio_enable(82,		DIR_OUT);	// Pin: TIM1_CH2
    GPIO_IN_WITH_ISR(85);	        // Pin: TIM1_CH2n

    gpio_enable(81,		DIR_OUT);	// Pin: TIM1_CH1
    GPIO_IN_WITH_ISR(84);	        // Pin: TIM1_CH1n

    gpio_enable(46,		DIR_OUT);	// Pin: UART3_TX
    GPIO_IN_WITH_ISR(47);	        // Pin: UART3_RX

    gpio_enable(44,		DIR_OUT);	// Pin: UART2_TX
    GPIO_IN_WITH_ISR(45);	        // Pin: UART2_RX

    gpio_enable(43,		DIR_OUT);	// Pin: UART1_TX
    GPIO_IN_WITH_ISR(42);	        // Pin: UART1_RX

    gpio_enable(69,		DIR_OUT);	// Pin: CAN0_TX
    GPIO_IN_WITH_ISR(68);	        // Pin: CAN0_RX

    gpio_enable(71,		DIR_OUT);	// Pin: CAN1_TX
    GPIO_IN_WITH_ISR(70);	        // Pin: CAN1_RX

    gpio_enable(73,		DIR_OUT);	// Pin: CAN2_TX
    GPIO_IN_WITH_ISR(72);	        // Pin: CAN2_RX

    gpio_enable(75,		DIR_OUT);	// Pin: CAN3_TX
    GPIO_IN_WITH_ISR(74);	        // Pin: CAN3_RX

#elif TEST_WITH_ISR_2
    /**************************************************
     * J4
     */
    GPIO_IN_WITH_ISR(102);	        // Pin: SDIO1_D0
    gpio_enable(103,	DIR_OUT);	// Pin: SDIO1_D1

    GPIO_IN_WITH_ISR(101);	        // Pin: SDIO1_CMD
    gpio_enable(104,	DIR_OUT);	// Pin: SDIO1_D2

    GPIO_IN_WITH_ISR(100);	        // Pin: SDIO1_CLK
    gpio_enable(105,	DIR_OUT);	// Pin: SDIO1_D3

    GPIO_IN_WITH_ISR(63);	        // Pin: SPI1_CS0
    gpio_enable(60,		DIR_OUT);	// Pin: SPI1_CLK

    GPIO_IN_WITH_ISR(61);	        // Pin: SPI1_MISO
    gpio_enable(62,		DIR_OUT);	// Pin: SPI1_MOSI

    GPIO_IN_WITH_ISR(67);	        // Pin: SPI2_CS0
    gpio_enable(64,		DIR_OUT);	// Pin: SPI2_CLK

    GPIO_IN_WITH_ISR(65);	        // Pin: SPI2_MISO
    gpio_enable(66,		DIR_OUT);	// Pin: SPI2_MOSI

    GPIO_IN_WITH_ISR(20);	        // Pin: LCD_R0
    gpio_enable(13,		DIR_OUT);	// Pin: LCD_G1

    GPIO_IN_WITH_ISR(22);	        // Pin: LCD_R2
    gpio_enable(12,		DIR_OUT);	// Pin: LCD_G0

    GPIO_IN_WITH_ISR(6);	        // Pin: LCD_B2
    gpio_enable(21,		DIR_OUT);	// Pin: LCD_R1

    GPIO_IN_WITH_ISR(4);	        // Pin: LCD_B0
    gpio_enable(5,		DIR_OUT);	// Pin: LCD_B1

    /**************************************************
     * J5
     */
    GPIO_IN_WITH_ISR(54);	        // Pin: I2C3_SCL
    gpio_enable(55,		DIR_OUT);	// Pin: I2C3_SDA

    GPIO_IN_WITH_ISR(52);	        // Pin: I2C2_SCL
    gpio_enable(53,		DIR_OUT);	// Pin: I2C2_SDA

    GPIO_IN_WITH_ISR(50);	        // Pin: I2C1_SCL
    gpio_enable(51,		DIR_OUT);	// Pin: I2C1_SDA

    GPIO_IN_WITH_ISR(48);	        // Pin: I2C0_SCL
    gpio_enable(49,		DIR_OUT);	// Pin: I2C0_SDA

    GPIO_IN_WITH_ISR(77);	        // Pin: I2S_BCLK
    gpio_enable(79,		DIR_OUT);	// Pin: I2S_DI

    GPIO_IN_WITH_ISR(76);	        // Pin: I2S_MCLK
    gpio_enable(80,		DIR_OUT);	// Pin: I2S_DO

    GPIO_IN_WITH_ISR(78);	        // Pin: I2S_LR       // XXX
    gpio_enable(89,		DIR_OUT);	// Pin: TIM2_CH3     // XXX

    GPIO_IN_WITH_ISR(88);	        // Pin: TIM2_CH2
    gpio_enable(87,		DIR_OUT);	// Pin: TIM2_CH1

    GPIO_IN_WITH_ISR(83);	        // Pin: TIM1_CH3
    gpio_enable(86,		DIR_OUT);	// Pin: TIM1_CH3n

    GPIO_IN_WITH_ISR(82);	        // Pin: TIM1_CH2
    gpio_enable(85,		DIR_OUT);	// Pin: TIM1_CH2n

    GPIO_IN_WITH_ISR(81);	        // Pin: TIM1_CH1
    gpio_enable(84,		DIR_OUT);	// Pin: TIM1_CH1n

    GPIO_IN_WITH_ISR(46);	        // Pin: UART3_TX
    gpio_enable(47,		DIR_OUT);	// Pin: UART3_RX

    GPIO_IN_WITH_ISR(44);	        // Pin: UART2_TX
    gpio_enable(45,		DIR_OUT);	// Pin: UART2_RX

    GPIO_IN_WITH_ISR(43);	        // Pin: UART1_TX
    gpio_enable(42,		DIR_OUT);	// Pin: UART1_RX

    GPIO_IN_WITH_ISR(69);	        // Pin: CAN0_TX
    gpio_enable(68,		DIR_OUT);	// Pin: CAN0_RX

    GPIO_IN_WITH_ISR(71);	        // Pin: CAN1_TX
    gpio_enable(70,		DIR_OUT);	// Pin: CAN1_RX

    GPIO_IN_WITH_ISR(73);	        // Pin: CAN2_TX
    gpio_enable(72,		DIR_OUT);	// Pin: CAN2_RX

    GPIO_IN_WITH_ISR(75);	         // Pin: CAN3_TX
    gpio_enable(74,		DIR_OUT);	// Pin: CAN3_RX

#elif TEST_ALL
    /**************************************************
     * J4
     */
    gpio_enable(102,	DIR_OUT);	// Pin: SDIO1_D0
    gpio_enable(103,	DIR_OUT);	// Pin: SDIO1_D1

    gpio_enable(101,	DIR_OUT);	// Pin: SDIO1_CMD
    gpio_enable(104,	DIR_OUT);	// Pin: SDIO1_D2

    gpio_enable(100,	DIR_OUT);	// Pin: SDIO1_CLK
    gpio_enable(105,	DIR_OUT);	// Pin: SDIO1_D3

    gpio_enable(63,		DIR_OUT);	// Pin: SPI1_CS0
    gpio_enable(60,		DIR_OUT);	// Pin: SPI1_CLK

    gpio_enable(61,		DIR_OUT);	// Pin: SPI1_MISO
    gpio_enable(62,		DIR_OUT);	// Pin: SPI1_MOSI

    gpio_enable(67,		DIR_OUT);	// Pin: SPI2_CS0
    gpio_enable(64,		DIR_OUT);	// Pin: SPI2_CLK

    gpio_enable(65,		DIR_OUT);	// Pin: SPI2_MISO
    gpio_enable(66,		DIR_OUT);	// Pin: SPI2_MOSI

    gpio_enable(20,		DIR_OUT);	// Pin: LCD_R0
    gpio_enable(13,		DIR_OUT);	// Pin: LCD_G1
    
    gpio_enable(22,		DIR_OUT);	// Pin: LCD_R2
    gpio_enable(12,		DIR_OUT);	// Pin: LCD_G0
    
    gpio_enable(6,		DIR_OUT);	// Pin: LCD_B2
    gpio_enable(21,		DIR_OUT);	// Pin: LCD_R1

    gpio_enable(4,		DIR_OUT);	// Pin: LCD_B0
    gpio_enable(5,		DIR_OUT);	// Pin: LCD_B1

    /**************************************************
     * J5
     */
    gpio_enable(54,		DIR_OUT);	// Pin: I2C3_SCL
    gpio_enable(55,		DIR_OUT);	// Pin: I2C3_SDA

    gpio_enable(52,		DIR_OUT);	// Pin: I2C2_SCL
    gpio_enable(53,		DIR_OUT);	// Pin: I2C2_SDA

    gpio_enable(50,		DIR_OUT);	// Pin: I2C1_SCL
    gpio_enable(51,		DIR_OUT);	// Pin: I2C1_SDA
    
    gpio_enable(48,		DIR_OUT);	// Pin: I2C0_SCL
    gpio_enable(49,		DIR_OUT);	// Pin: I2C0_SDA
    
    gpio_enable(77,		DIR_OUT);	// Pin: I2S_BCLK
    gpio_enable(79,		DIR_OUT);	// Pin: I2S_DI
    
    gpio_enable(76,		DIR_OUT);	// Pin: I2S_MCLK
    gpio_enable(80,		DIR_OUT);	// Pin: I2S_DO
    
    gpio_enable(78,		DIR_OUT);	// Pin: I2S_LR       // XXX
    gpio_enable(89,		DIR_OUT);	// Pin: TIM2_CH3     // XXX

    gpio_enable(88,		DIR_OUT);	// Pin: TIM2_CH2
    gpio_enable(87,		DIR_OUT);	// Pin: TIM2_CH1

    gpio_enable(83,		DIR_OUT);	// Pin: TIM1_CH3
    gpio_enable(86,		DIR_OUT);	// Pin: TIM1_CH3n

    gpio_enable(82,		DIR_OUT);	// Pin: TIM1_CH2
    gpio_enable(85,		DIR_OUT);	// Pin: TIM1_CH2n

    gpio_enable(81,		DIR_OUT);	// Pin: TIM1_CH1
    gpio_enable(84,		DIR_OUT);	// Pin: TIM1_CH1n

    gpio_enable(46,		DIR_OUT);	// Pin: UART3_TX
    gpio_enable(47,		DIR_OUT);	// Pin: UART3_RX

    gpio_enable(44,		DIR_OUT);	// Pin: UART2_TX
    gpio_enable(45,		DIR_OUT);	// Pin: UART2_RX

    gpio_enable(43,		DIR_OUT);	// Pin: UART1_TX
    gpio_enable(42,		DIR_OUT);	// Pin: UART1_RX

    gpio_enable(69,		DIR_OUT);	// Pin: CAN0_TX
    gpio_enable(68,		DIR_OUT);	// Pin: CAN0_RX

    gpio_enable(71,		DIR_OUT);	// Pin: CAN1_TX
    gpio_enable(70,		DIR_OUT);	// Pin: CAN1_RX

    gpio_enable(73,		DIR_OUT);	// Pin: CAN2_TX
    gpio_enable(72,		DIR_OUT);	// Pin: CAN2_RX

    gpio_enable(75,		DIR_OUT);	// Pin: CAN3_TX
    gpio_enable(74,		DIR_OUT);	// Pin: CAN3_RX

#else

    /**
     * 这两脚不能触发中断
     */
  #if TEST_48_50_1

    gpio_enable(50,		DIR_OUT);	// Pin: I2C1_SCL
    GPIO_IN_WITH_ISR(51);	        // Pin: I2C1_SDA

    gpio_enable(48,		DIR_OUT);	// Pin: I2C0_SCL
    GPIO_IN_WITH_ISR(49);	        // Pin: I2C0_SDA

  #elif TEST_48_50_2

    GPIO_IN_WITH_ISR(50);	        // Pin: I2C1_SCL
    gpio_enable(51,		DIR_OUT);	// Pin: I2C1_SDA

    GPIO_IN_WITH_ISR(48);	        // Pin: I2C0_SCL
    gpio_enable(49,		DIR_OUT);	// Pin: I2C0_SDA

  #endif
  
#endif
}

//-----------------------------------------------------------------------------

static osal_task_t m_gpio_task = NULL;

static void gpio_task(void *arg)
{
    int outval = 0;
    
    ls2k_set_pinmux_as_gpio();

#if TEST_WITH_ISR_1 || TEST_WITH_ISR_2
    ls2k_interrupt_enable(EXTI2_GPIO_0_3_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_4_7_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_8_11_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_12_15_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_16_19_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_20_23_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_24_27_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_28_31_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_32_35_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_36_39_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_40_43_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_44_47_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_48_51_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_52_55_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_56_59_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_60_63_IRQ);
    ls2k_interrupt_enable(EXTI2_GPIO_64_67_IRQ);
    ls2k_interrupt_enable(EXTI3_GPIO_68_71_IRQ);
    ls2k_interrupt_enable(EXTI3_GPIO_72_75_IRQ);
    ls2k_interrupt_enable(EXTI3_GPIO_76_79_IRQ);
    ls2k_interrupt_enable(EXTI3_GPIO_80_83_IRQ);
    ls2k_interrupt_enable(EXTI3_GPIO_84_87_IRQ);
    ls2k_interrupt_enable(EXTI3_GPIO_88_91_IRQ);
    ls2k_interrupt_enable(EXTI3_GPIO_92_95_IRQ);
    ls2k_interrupt_enable(EXTI3_GPIO_96_99_IRQ);
    ls2k_interrupt_enable(EXTI3_GPIO_100_103_IRQ);
    ls2k_interrupt_enable(EXTI3_GPIO_104_105_IRQ);
#elif TEST_48_50_1 || TEST_48_50_2
    ls2k_interrupt_enable(EXTI2_GPIO_48_51_IRQ);
#endif

    for ( ; ; )
    {
        outval = ~outval;

#if TEST_WITH_ISR_1
        /**************************************************
         * J4
         */

        gpio_write(102, outval);        /* 1 */
        gpio_write(101, outval);        /* 3 */
        gpio_write(100, outval);        /* 5 */

        gpio_write(63, outval);         /* 9 */
        gpio_write(61, outval);         /* 11 */

        gpio_write(67, outval);         /* 15 */
        gpio_write(65, outval);         /* 17 */

        gpio_write(20, outval);         /* 21 */
        gpio_write(22, outval);         /* 23 */

        gpio_write( 6, outval);         /* 25 */
        gpio_write( 4, outval);         /* 27 */

        /**************************************************
         * J5
         */
        gpio_write(54, outval);         /* 11 */
        gpio_write(52, outval);         /* 13 */

        gpio_write(50, outval);         /* 15 */
        gpio_write(48, outval);         /* 17 */

        gpio_write(77, outval);         /* 19 */
        gpio_write(76, outval);         /* 21 */

        gpio_write(78, outval);         /* 23 */

        gpio_write(88, outval);         /* 27 */
        gpio_write(83, outval);         /* 29 */

        gpio_write(82, outval);         /* 31 */
        gpio_write(81, outval);         /* 33 */

        gpio_write(46, outval);         /* 35 */
        gpio_write(44, outval);         /* 37 */
        gpio_write(43, outval);         /* 39 */

        gpio_write(69, outval);         /* 47 */
        gpio_write(71, outval);         /* 49 */

        gpio_write(73, outval);         /* 51 */
        gpio_write(75, outval);         /* 53 */

#elif TEST_WITH_ISR_2
        /**************************************************
         * J4
         */

        gpio_write(103, outval);        /* 2 */
        gpio_write(104, outval);        /* 4 */
        gpio_write(105, outval);        /* 6 */

        gpio_write(60, outval);         /* 10 */
        gpio_write(62, outval);         /* 12 */

        gpio_write(64, outval);         /* 16 */
        gpio_write(66, outval);         /* 18 */

        gpio_write(13, outval);         /* 22 */
        gpio_write(12, outval);         /* 24 */

        gpio_write(21, outval);         /* 26 */
        gpio_write( 5, outval);         /* 28 */

        /**************************************************
         * J5
         */
        gpio_write(55, outval);         /* 12 */
        gpio_write(53, outval);         /* 14 */

        gpio_write(51, outval);         /* 16 */
        gpio_write(49, outval);         /* 18 */

        gpio_write(79, outval);         /* 20 */
        gpio_write(80, outval);         /* 22 */

        gpio_write(89, outval);         /* 25 */

        gpio_write(87, outval);         /* 28 */
        gpio_write(86, outval);         /* 30 */

        gpio_write(85, outval);         /* 32 */
        gpio_write(84, outval);         /* 34 */

        gpio_write(47, outval);         /* 36 */
        gpio_write(45, outval);         /* 38 */
        gpio_write(42, outval);         /* 40 */

        gpio_write(68, outval);         /* 48 */
        gpio_write(70, outval);         /* 50 */

        gpio_write(72, outval);         /* 52 */
        gpio_write(74, outval);         /* 54 */

#elif TEST_ALL
        /**************************************************
         * J4
         */

        gpio_write(102, outval);        /* 1 */
        gpio_write(103, outval);        /* 2 */
        gpio_write(101, outval);        /* 3 */
        gpio_write(104, outval);        /* 4 */
        gpio_write(100, outval);        /* 5 */
        gpio_write(105, outval);        /* 6 */

        gpio_write(63, outval);         /* 9 */
        gpio_write(60, outval);         /* 10 */
        gpio_write(61, outval);         /* 11 */
        gpio_write(62, outval);         /* 12 */

        gpio_write(67, outval);         /* 15 */
        gpio_write(64, outval);         /* 16 */
        gpio_write(65, outval);         /* 17 */
        gpio_write(66, outval);         /* 18 */

        gpio_write(20, outval);         /* 21 */
        gpio_write(13, outval);         /* 22 */
        gpio_write(22, outval);         /* 23 */
        gpio_write(12, outval);         /* 24 */

        gpio_write( 6, outval);         /* 25 */
        gpio_write(21, outval);         /* 26 */
        gpio_write( 4, outval);         /* 27 */
        gpio_write( 5, outval);         /* 28 */
        
        /**************************************************
         * J5
         */
        gpio_write(54, outval);         /* 11 */
        gpio_write(55, outval);         /* 12 */
        gpio_write(52, outval);         /* 13 */
        gpio_write(53, outval);         /* 14 */

        gpio_write(50, outval);         /* 15 */
        gpio_write(51, outval);         /* 16 */
        gpio_write(48, outval);         /* 17 */
        gpio_write(49, outval);         /* 18 */

        gpio_write(77, outval);         /* 19 */
        gpio_write(79, outval);         /* 20 */
        gpio_write(76, outval);         /* 21 */
        gpio_write(80, outval);         /* 22 */

        gpio_write(78, outval);         /* 23 */
        gpio_write(89, outval);         /* 25 */

        gpio_write(88, outval);         /* 27 */
        gpio_write(87, outval);         /* 28 */
        gpio_write(83, outval);         /* 29 */
        gpio_write(86, outval);         /* 30 */

        gpio_write(82, outval);         /* 31 */
        gpio_write(85, outval);         /* 32 */
        gpio_write(81, outval);         /* 33 */
        gpio_write(84, outval);         /* 34 */

        gpio_write(46, outval);         /* 35 */
        gpio_write(47, outval);         /* 36 */
        gpio_write(44, outval);         /* 37 */
        gpio_write(45, outval);         /* 38 */
        gpio_write(43, outval);         /* 39 */
        gpio_write(42, outval);         /* 40 */

        gpio_write(69, outval);         /* 47 */
        gpio_write(68, outval);         /* 48 */
        gpio_write(71, outval);         /* 49 */
        gpio_write(70, outval);         /* 50 */

        gpio_write(73, outval);         /* 51 */
        gpio_write(72, outval);         /* 52 */
        gpio_write(75, outval);         /* 53 */
        gpio_write(74, outval);         /* 54 */

#else
    #if TEST_48_50_1
        gpio_write(50, outval);         /* 15 */
        gpio_write(48, outval);         /* 17 */
    #elif TEST_48_50_2
        gpio_write(51, outval);         /* 16 */
        gpio_write(49, outval);         /* 18 */
    #endif
#endif

        osal_task_sleep(500);
    }
}


//-----------------------------------------------------------------------------

void ls2k301_gpio_test_start(void)
{
    m_gpio_task = osal_task_create("demotask1",
                                    4096,
                                    0,
                                    0,
                                    gpio_task,
                                    NULL );
    if (m_gpio_task)
    {
        printk("create gpio task successful\r\n");
    }

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

