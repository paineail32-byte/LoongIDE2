/*
 * Copyright (C) 2021-2024 Suzhou Tiancheng Software Inc. All Rights Reserved.
 *
 */
/*
 * ls2k_adc_hw.h
 *
 * created: 2024-06-09
 *  author: Bian
 */

#ifndef _LS2K_ADC_HW_H
#define _LS2K_ADC_HW_H

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------------------
// ADC 设备
//-------------------------------------------------------------------------------------------------

#define ADC_BASE        0x1611c000

/*
 * ADC 控制器
 */
typedef struct
{
	volatile unsigned int sr;			/* 0x00 32 ADC 状态寄存器 */
	volatile unsigned int cr1;			/* 0x04 32 ADC 控制寄存器1 */
	volatile unsigned int cr2;			/* 0x08 32 ADC 控制寄存器2 */
	volatile unsigned int smpr1;		/* 0x0C 32 ADC 采样时间寄存器1 */
	volatile unsigned int smpr2;		/* 0x10 32 ADC 采样时间寄存器2 */
	volatile unsigned int jofr1;		/* 0x14 32 ADC 注入通道偏移寄存器1 */
	volatile unsigned int jofr2;		/* 0x18 32 ADC 注入通道偏移寄存器2 */
	volatile unsigned int jofr3;		/* 0x1C 32 ADC 注入通道偏移寄存器3 */
	volatile unsigned int jofr4;		/* 0x20 32 ADC 注入通道偏移寄存器4 */
	volatile unsigned int htr;			/* 0x24 32 ADC 看门狗高阈值寄存器 */
	volatile unsigned int ltr;			/* 0x28 32 ADC 看门狗低阈值寄存器 */
	volatile unsigned int sqr1;			/* 0x2C 32 ADC 规则序列寄存器1 */
	volatile unsigned int sqr2;			/* 0x30 32 ADC 规则序列寄存器2 */
	volatile unsigned int sqr3;			/* 0x34 32 ADC 规则序列寄存器3 */
	volatile unsigned int jsqr;			/* 0x38 32 ADC 注入序列寄存器 */
	volatile unsigned int jdr1;			/* 0x3C 32 ADC 注入数据寄存器1 */
	volatile unsigned int jdr2;			/* 0x40 32 ADC 注入数据寄存器2 */
	volatile unsigned int jdr3;			/* 0x44 32 ADC 注入数据寄存器3 */
	volatile unsigned int jdr4;			/* 0x48 32 ADC 注入数据寄存器4 */
	volatile unsigned int dr;			/* 0x4c 32 ADC 规则数据寄存器 */
} HW_ADC_t;

/*
 * ADC Channels
 */
#define ADC_Channel_0           0x00
#define ADC_Channel_1           0x01
#define ADC_Channel_2           0x02
#define ADC_Channel_3           0x03
#define ADC_Channel_4           0x08
#define ADC_Channel_5           0x09
#define ADC_Channel_6           0x0a
#define ADC_Channel_7           0x0b

#define ADC_Channel_8           0x18
#define ADC_Channel_9           0x19
#define ADC_Channel_10          0x1A
#define ADC_Channel_11          0x1B
#define ADC_Channel_12          0x1C
#define ADC_Channel_13          0x1D
#define ADC_Channel_14          0x1E
#define ADC_Channel_15          0x1F
#define ADC_Channel_16          0x10
#define ADC_Channel_17          0x11

/**
 * ADC_SR - ADC 状态寄存器
 */
#define ADC_SR_START			bit(4)			/* RW 规则通道开始标志位. 0: 规则通道转换未开始, 1: 规则通道转换已开始 */
#define ADC_SR_JSTART			bit(3)			/* RW 注入通道开始标志位. 0: 注入通道组转换未开始, 1: 注入通道组转换已开始 */
#define ADC_SR_JEOC				bit(2)			/* RW 注入通道转换结束标志位. 0: 转换未完成, 1: 转换完成 */
#define ADC_SR_EOC				bit(1)			/* RW 转换结束标志位. 0: 转换未完成, 1: 转换完成 */
#define ADC_SR_AWD				bit(0)			/* RW 模拟看门狗标志位. 0: 没有发生模拟看门狗事件, 1: 发生模拟看门狗事件 */

/**
 * ADC_CR1 - ADC 控制寄存器1
 */
#define ADC_CR1_OPS_MASK		(0x3<<30)		/* RW bit[31:30] ADC 控制信号相位调节. */
#define ADC_CR1_OPS_SHIFT		30
#define ADC_CR1_OPS_CLKSAME		(0<<30)			/* 0: 与ADCCLK 上升沿同时刻 */
#define ADC_CR1_OPS_CLKBEFORE1	(1<<30)			/* 1: 较ADCCLK 上升沿前一pclk */
#define ADC_CR1_OPS_CLKAFTER1	(2<<30)			/* 2: 较ADCCLK 上升沿后一pclk */
#define ADC_CR1_CLK_MASK		(0x3F<<24)		/* RW bit[29:24] ADCCLK 分频系数[5:0]位 */
#define ADC_CR1_CLK_SHIFT		24
#define ADC_CR1_AWDEN			bit(23)			/* RW 规则通道启用模拟看门狗.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 在规则通道上禁用模拟看门狗, 1: 在规则通道上启用模拟看门狗 */
#define ADC_CR1_JAWDEN			bit(22)			/* RW 注入通道启用模拟看门狗.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 在注入通道上禁用模拟看门狗, 1: 在注入通道上启用模拟看门狗 */
#define ADC_CR1_DIFFMOD			bit(20)			/* RW 差分模式使能, 启用时仅可对低四对模拟输入进行比较.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 不启用差分模式, 1: 启用差分模式 */

#define ADC_CR1_DISCNUM_MASK	(0x7<<13)		/* RW bit[15:13] 间断模式通道计数, 在间断模式下,
 	 	 	 	 	 	 	 	 	 	 	 	 *    收到外部触发后转换规则通道的数目. */
#define ADC_CR1_DISCNUM_SHIFT	13				// 000: 1 个通道; 001: 2 个通道; …… ; 111: 8 个通道
#define ADC_CR1_JDISCEN			bit(12)			/* RW 在注入通道上的间断模式, 用于开启或关闭注入通道组上的间断模式.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 注入通道停用间断模式, 1: 注入通道启用间断模式
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR1_DISCEN			bit(11)			/* RW 在规则通道上的间断模式, 用于开启或关闭规则通道组上的间断模式.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 规则通道停用间断模式, 1: 规则通道启用间断模式
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR1_JAUTO			bit(10)			/* RW 自动注入通道组开转换, 用于开启或关闭规则通道组转换结束后自动的注入通道组转换.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 关闭自动的注入通道组转换, 1: 开启自动的注入通道组转换
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR1_AWDSGL			bit(9)			/* RW 扫描模式中在一个单一的通道上使用看门狗, 用于开启或关闭
 	 	 	 	 	 	 	 	 	 	 	 	 *    由AWDCH[4:0]位指定的通道上的模拟看门狗功能.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 在所有的通道上使用模拟看门狗, 1: 在单一通道上使用模拟看门狗
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR1_SCAN			bit(8)			/* RW 扫描模式. 用于开启或关闭扫描模式. 在扫描模式中,
 	 	 	 	 	 	 	 	 	 	 	 	 *    转换由ADC_SQRx或ADC_JSQRx 寄存器选中的通道.
 	 	 	 	 	 	 	 	 	 	 	 	 *    只有在最后一个通道转换完毕后,  才会根据EOCIE 或JEOCIE 位产生EOC 或JEOC 中断.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 关闭扫描模式, 1: 使用扫描模式
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR1_JEOCIE			bit(7)			/* RW 注入通道中断使能. 用于禁止或允许所有注入通道转换结束后产生中断.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 禁止JEOC 中断, 1: 允许JEOC 中断. 当硬件设置JEOC 位时产生中断
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR1_AWDIE			bit(6)			/* RW 模拟看门狗中断使能. 用于禁止或允许模拟看门狗产生中断. 在扫描模式下,
 	 	 	 	 	 	 	 	 	 	 	 	 *    如果看门狗检测到超范围的数值时, 只有在设置了该位时扫描才会中止.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 禁止模拟看门狗中断, 1: 允许模拟看门狗中断
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR1_EOCIE			bit(5)			/* RW EOC 中断使能. 用于禁止或允许转换结束后产生中断.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 禁止EOC 中断, 1: 允许EOC 中断. 当硬件设置EOC 位时产生中断
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR1_AWDCH_MASK		0x1F			/* RW bit[4:0] 模拟看门狗通道选择. 用于选择模拟看门狗保护的输入通道.
												 *	  00000: ADC 模拟输入通道0
												 *	  00001: ADC 模拟输入通道1
												 *	  ……
												 *	  00111: ADC 模拟输入通道7
 	 	 	 	 	 	 	 	 	 	 	 	 */

#define CR1_CLEAR_MASK_R        (~(ADC_CR1_OPS_MASK | \
                                   ADC_CR1_CLK_MASK | \
                                   ADC_CR1_AWDEN |   \
                                   ADC_CR1_DIFFMOD | \
                                   ADC_CR1_DISCNUM_MASK | \
                                   ADC_CR1_SCAN |   \
                                   ADC_CR1_DISCEN | \
                                   ADC_CR1_EOCIE ))

#define CR1_CLEAR_MASK_J        (~(ADC_CR1_JAWDEN |  \
                                   ADC_CR1_JDISCEN | \
                                   ADC_CR1_JAUTO |   \
                                   ADC_CR1_JEOCIE))


static inline int adc_set_discnum(HW_ADC_t *hwADC, int num, int Jmode)
{
    unsigned int cr1 = hwADC->cr1;
    
    cr1 &= ~(ADC_CR1_DISCNUM_MASK | ADC_CR1_JDISCEN | ADC_CR1_DISCEN);
    
    if (num > 0)
    {
        cr1 |= ((num - 1) << ADC_CR1_DISCNUM_SHIFT) & ADC_CR1_DISCNUM_MASK;
        if (Jmode)
            cr1 |= ADC_CR1_JDISCEN;
        else
            cr1 |= ADC_CR1_DISCEN;
    }
    
    hwADC->cr1 = cr1;
    return 0;
}

/**
 * ADC_CR2 - ADC 控制寄存器2
 */
//#define ADC_CR2_RESET			bit(31)
#define ADC_CR2_EDGE_DOWN		bit(30)			/* RW ADC 时钟触发沿选择. 0: 上升沿触发, 1: 下降沿触发 */
#define ADC_CR2_CLK_MASK		(0xF<<26)		/* RW bit[29:26] ADC 时钟分频系数[9:6]位域位域名称访问描述 */
#define ADC_CR2_CLK_SHIFT		26
#define ADC_CR2_JTRIGMOD_MASK	(0x3<<24)		/* RW bit[25:24] 注入触发模式选择
												 *    0: 结束当前规则组转换并立即开始注入通道转换
												 *    1: 结束当前规则组转换并在插入一拍ADC 复位控制信号后开始注入通道转换
												 *    2: 在当前规则组转换结束后开始注入通道转换
												 */
#define ADC_CR2_JTRIGMOD_SHIFT	24
#define ADC_CR2_SWSTART			bit(22)			/* RW 开始转换规则通道. 由软件设置该位以启动转换,
 	 	 	 	 	 	 	 	 	 	 	 	 *    转换开始后硬件马上清除此位.
 	 	 	 	 	 	 	 	 	 	 	 	 *    如果在extsel[2:0]位中选择了swstart 为触发事件,
 	 	 	 	 	 	 	 	 	 	 	 	 *    该位用于启动一组规则通道的转换.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 复位状态, 1: 开始转换规则通道
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR2_JSWSTART		bit(21)			/* RW 开始转换注入通道. 由软件设置该位以启动转换,软件可清除此位
 	 	 	 	 	 	 	 	 	 	 	 	 *    或在转换开始后硬件马上清除此位.
 	 	 	 	 	 	 	 	 	 	 	 	 *    如果在jextsel[2:0]位中选择了jswstart 为触发事件,
                                                 *    该位用于启动一组注入通道的转换.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 复位状态, 1: 开始转换注入通道
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR2_EXTTRIG			bit(20)			/* RW 规则通道的外部触发转换模式该位由软件设置和清除,
 	 	 	 	 	 	 	 	 	 	 	 	 *    用于开启或禁止可以启动规则通道组转换的外部触发事件
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 不用外部事件启动转换, 1: 使用外部事件启动转换
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR2_EXTSEL_MASK		(0x7<<17)		/* RW bit[19:17] 选择启动规则通道组转换的外部事件,
 	 	 	 	 	 	 	 	 	 	 	 	 *    这些位选择用于启动规则通道组转换的外部事件. */
#define ADC_CR2_EXTSEL_SHIFT	17				/*    触发配置如下: */
#define ADC_CR2_EXTSEL_ATIM_CC1 (0<<17)			/*    3'b000: ATIM\_CC1 事件 */
#define ADC_CR2_EXTSEL_ATIM_CC2 (1<<17)			/*    3'b001: ATIM\_CC2 事件 */
#define ADC_CR2_EXTSEL_ATIM_CC3 (2<<17)			/*    3'b010: ATIM\_CC3 事件 */
#define ADC_CR2_EXTSEL_GTIM_CC2 (3<<17)			/*    3'b011: GTIM\_CC2 事件 */
#define ADC_CR2_EXTSEL_EXTI_11	(6<<17)			/*    3'b110: EXTI 线11 */
#define ADC_CR2_EXTSEL_SWATART	(7<<17)			/*    3'b111: swstart */

#define ADC_CR2_JEXTTRIG		bit(15)			/* RW 注入通道的外部触发转换模式. 该位由软件设置和清除,
 	 	 	 	 	 	 	 	 	 	 	 	 *    用于开启或禁止可以启动注入通道组转换的外部触发事件
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 不用外部事件启动转换, 1: 使用外部事件启动转换
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR2_JEXTSEL_MASK	(0x7<<12)		/* RW bit[14:12] 选择启动注入通道组转换的外部事件.
 	 	 	 	 	 	 	 	 	 	 	 	 *    这些位选择用于启动规则通道组转换的外部事件, */
#define ADC_CR2_JEXTSEL_SHIFT	12				/*    触发配置如下: */
#define ADC_CR2_JEXTSEL_ATIM_TRGO	(0<<12)		/*    3'b000: ATIM_TRGO 事件 */
#define ADC_CR2_JEXTSEL_ATIM_CC4	(1<<12)		/*    3'b001: ATIM_CC4 事件 */
#define ADC_CR2_JEXTSEL_GTIM_TRGO	(2<<12)		/*    3'b010: GTIM_TRGO 事件 */
#define ADC_CR2_JEXTSEL_GTIM_CC1	(3<<12)		/*    3'b011: GTIM_CC1 事件 */
#define ADC_CR2_JEXTSEL_EXTI_15		(6<<12)		/*    3'b110: EXTI 线15 */
#define ADC_CR2_JEXTSEL_JSWSTART	(7<<12)		/*    3'b111: JSWSTART */

#define ADC_CR2_ALIGN_LEFT		bit(11)			/* RW 数据对齐. 0: 右对齐, 1: 左对齐 */
#define ADC_CR2_DMA				bit(8)			/* RW 直接存储器访问模式. 0: 不使用DMA 模式, 1: 使用DMA 模式 */
#define ADC_CR2_RSTCAL  	    bit(3)		    /* RW 该位由软件设置并由硬件清除. 在校准寄存器被初始化后, 该位将被清除.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 校准寄存器已初始化, 1: 初始化校准寄存器
 	 	 	 	 	 	 	 	 	 	 	 	 */

#define ADC_CR2_CAL				bit(2)			/* RW AD 校准. 该位由软件设置以开始校准,并在校准结束时由硬件清除.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 校准完成, 1: 开始校准 */
#define ADC_CR2_CONT			bit(1)			/* RW 连续转换. 该位由软件设置和清除. 如果设置了此位,
 	 	 	 	 	 	 	 	 	 	 	 	 *    则转换将连续进行直到该位被清除.
 	 	 	 	 	 	 	 	 	 	 	 	 *    0: 单次转换模式, 1: 连续转换模式
 	 	 	 	 	 	 	 	 	 	 	 	 */
#define ADC_CR2_ADON			bit(0)			/* RW 开/关AD 转换器. 该位由软件设置和清除. 当该位为'0' 时,
 	 	 	 	 	 	 	 	 	 	 	 	 *    写入'1' 将把ADC 从断电模式下唤醒,
												 *    该位为'1' 时,写入'1' 将启动转换
												 *    0: 关闭ADC 转换/校准,并进入断电模式
												 *    1: 开启ADC 并启动转换
												 */

#define CR2_CLEAR_MASK_R        (~(ADC_CR2_EDGE_DOWN | \
                                   ADC_CR2_CLK_MASK  | \
                                   ADC_CR2_SWSTART   | \
                                   ADC_CR2_EXTTRIG   | \
                                   ADC_CR2_EXTSEL_MASK | \
                                   ADC_CR2_ALIGN_LEFT  | \
                                   ADC_CR2_DMA    | \
                                   ADC_CR2_RSTCAL | \
                                   ADC_CR2_CAL    | \
                                   ADC_CR2_CONT   | \
                                   ADC_CR2_ADON))

#define CR2_CLEAR_MASK_J        (~(ADC_CR2_JTRIGMOD_MASK | \
                                   ADC_CR2_JSWSTART | \
                                   ADC_CR2_JEXTTRIG | \
                                   ADC_CR2_JEXTSEL_MASK))

/**
 * ADC_SMPR1 - ADC 采样时间寄存器1
 */
#if 0
/*
 * move to ls2k_adc.h
 */
#define ADC_SMP_1P				0				/* 000: 1 个周期 */
#define ADC_SMP_2P				1				/* 001: 2 个周期 */
#define ADC_SMP_4P				2				/* 010: 4 个周期 */
#define ADC_SMP_8P				3				/* 011: 8 个周期 */
#define ADC_SMP_16P				4				/* 100: 16 个周期 */
#define ADC_SMP_32P				5				/* 101: 32 个周期 */
#define ADC_SMP_64P				6				/* 110: 64 个周期 */
#define ADC_SMP_128P			7				/* 111: 128 个周期 */
#endif

#define ADC_SMPR1_SMP18_MASK	(0x7<<21)		/* RW bit[23:21] 通道18的采样时间. 建议至少配置为2 个周期 */
#define ADC_SMPR1_SMP18_SHIFT	21
#define ADC_SMPR1_SMP17_MASK	(0x7<<18)		/* RW bit[20:18] 通道17的采样时间. */
#define ADC_SMPR1_SMP17_SHIFT	18
#define ADC_SMPR1_SMP16_MASK	(0x7<<15)		/* RW bit[17:15] 通道16的采样时间. */
#define ADC_SMPR1_SMP16_SHIFT	15
#define ADC_SMPR1_SMP15_MASK	(0x7<<12)		/* RW bit[14:12] 通道15的采样时间. */
#define ADC_SMPR1_SMP15_SHIFT	12
#define ADC_SMPR1_SMP14_MASK	(0x7<<9)		/* RW bit[11:9] 通道14的采样时间. */
#define ADC_SMPR1_SMP14_SHIFT	9
#define ADC_SMPR1_SMP13_MASK	(0x7<<6)		/* RW bit[8:6] 通道13的采样时间. */
#define ADC_SMPR1_SMP13_SHIFT	6
#define ADC_SMPR1_SMP12_MASK	(0x7<<3)		/* RW bit[5:3] 通道12的采样时间. */
#define ADC_SMPR1_SMP12_SHIFT	3
#define ADC_SMPR1_SMP11_MASK	(0x7<<0)		/* RW bit[2:0] 通道11的采样时间. */
#define ADC_SMPR1_SMP11_SHIFT	0

/**
 * ADC_SMPR2 - ADC 采样时间寄存器2
 */
#define ADC_SMPR2_SMP10_MASK	(0x7<<27)		/* RW bit[29:27] 通道10的采样时间. */
#define ADC_SMPR2_SMP10_SHIFT	27
#define ADC_SMPR2_SMP9_MASK		(0x7<<24)		/* RW bit[26:24] 通道9的采样时间. */
#define ADC_SMPR2_SMP9_SHIFT	24
#define ADC_SMPR2_SMP8_MASK		(0x7<<21)		/* RW bit[23:21] 通道8的采样时间. */
#define ADC_SMPR2_SMP8_SHIFT	21
#define ADC_SMPR2_SMP7_MASK		(0x7<<18)		/* RW bit[20:18] 通道7的采样时间. */
#define ADC_SMPR2_SMP7_SHIFT	18
#define ADC_SMPR2_SMP6_MASK		(0x7<<15)		/* RW bit[17:15] 通道6的采样时间. */
#define ADC_SMPR2_SMP6_SHIFT	15
#define ADC_SMPR2_SMP5_MASK		(0x7<<12)		/* RW bit[14:12] 通道5的采样时间. */
#define ADC_SMPR2_SMP5_SHIFT	12
#define ADC_SMPR2_SMP4_MASK		(0x7<<9)		/* RW bit[11:9] 通道4的采样时间. */
#define ADC_SMPR2_SMP4_SHIFT	9
#define ADC_SMPR2_SMP3_MASK		(0x7<<6)		/* RW bit[8:6] 通道3的采样时间. */
#define ADC_SMPR2_SMP3_SHIFT	6
#define ADC_SMPR2_SMP2_MASK		(0x7<<3)		/* RW bit[5:3] 通道2的采样时间. */
#define ADC_SMPR2_SMP2_SHIFT	3
#define ADC_SMPR2_SMP1_MASK		(0x7<<0)		/* RW bit[2:0] 通道1的采样时间. */
#define ADC_SMPR2_SMP1_SHIFT	0

//-----------------------------------------------------------------------------
// 设置adc通道的采样周期. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_set_samp_time(HW_ADC_t *hwADC, int rank, int t)
{
	unsigned int mask, val;

    if ((rank > 0) && (rank <= 10))
	{
		mask = 0x7 << ((rank - 1) * 3);
		val  = (t & 0x7) << ((rank - 1) * 3);
		hwADC->smpr2 &= ~mask;
		hwADC->smpr2 |= val;
		return 0;
	}
    else if ((rank > 10) && (rank <= 18))
	{
		mask = 0x7 << ((rank - 11) * 3);
		val  = (t & 0x7) << ((rank - 11) * 3);
		hwADC->smpr1 &= ~mask;
		hwADC->smpr1 |= val;
		return 0;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// 获取adc通道的采样周期. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_get_samp_time(HW_ADC_t *hwADC, int rank)
{
	if ((rank > 0) && (rank <= 10))
	{
		return (int)(hwADC->smpr2 >> ((rank - 1) * 3)) & 0x7;
	}
    else if ((rank > 10) && (rank <= 18))
	{
		return (int)(hwADC->smpr1 >> ((rank - 11) * 3)) & 0x7;
	}

	return -1;
}

/**
 * ADC_JOFRx (1-4) - ADC 注入通道数据偏移寄存器x (1-4)
 */
#define ADC_JOFFSET_MASK		0xFFF			/* RW bit[11:0] 注入通道x 的数据偏移. 当转换注入通道时,
 	 	 	 	 	 	 	 	 	 	 	 	 *    这些位定义了用于从原始转换数据中
 	 	 	 	 	 	 	 	 	 	 	 	 *    减去的数值. 转换的结果可以在ADC_JDRx 寄存器中读出.
 	 	 	 	 	 	 	 	 	 	 	 	 */

//-----------------------------------------------------------------------------
// 设置 adc 注入通道数据偏移. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_set_joff(HW_ADC_t *hwADC, int rank, int off)
{
	if (!((rank > 0) && (rank <= 4)))
		return -1;

	switch (rank)
	{
		case 1: hwADC->jofr1 = (unsigned)off & ADC_JOFFSET_MASK; break;
		case 2: hwADC->jofr2 = (unsigned)off & ADC_JOFFSET_MASK; break;
		case 3: hwADC->jofr3 = (unsigned)off & ADC_JOFFSET_MASK; break;
		case 4: hwADC->jofr4 = (unsigned)off & ADC_JOFFSET_MASK; break;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// 获取 adc 注入通道数据偏移. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_get_joff(HW_ADC_t *hwADC, int rank)
{
	int ret = -1;

	switch (rank)
	{
		case 1: ret = (int)(hwADC->jofr1 & ADC_JOFFSET_MASK); break;
		case 2: ret = (int)(hwADC->jofr2 & ADC_JOFFSET_MASK); break;
		case 3: ret = (int)(hwADC->jofr3 & ADC_JOFFSET_MASK); break;
		case 4: ret = (int)(hwADC->jofr4 & ADC_JOFFSET_MASK); break;
	}

	return ret;
}

/**
 * ADC_HTR - ADC 看门狗高阈值寄存器
 */
#define ADC_HT_MASK				0xFFF			/* RW bit[11:0] 模拟看门狗高阀值 */

/**
 * ADC_LTR - ADC 看门狗低阈值寄存器
 */
#define ADC_LT_MASK				0xFFF			/* RW bit[11:0] 模拟看门狗低阀值 */

/**
 * ADC_SQR1 - ADC 规则序列寄存器1
 */
#define ADC_SQR1_LEN_MASK		(0xF<<20)		/* RW bit[23:20] 规则通道序列长度
												 *    0000: 1 个转换
												 *    0001: 2 个转换
												 *    ……
												 *    1111: 16 个转换
												 */
#define ADC_SQR1_LEN_SHIFT		20

/*
 * 注入序列转换的的输入源
 */
#define ADC_SQR1_SQ16_MASK		(0x1F<<15)		/* RW bit[19:15] 注入序列第16个转换的的输入源 */
#define ADC_SQR1_SQ16_SHIFT		15
#define ADC_SQR1_SQ15_MASK		(0x1F<<10)		/* RW bit[14:10] 规则序列第15个转换的输入源 */
#define ADC_SQR1_SQ15_SHIFT		10
#define ADC_SQR1_SQ14_MASK		(0x1F<<5)		/* RW bit[9:5] 规则序列第14个转换的输入源 */
#define ADC_SQR1_SQ14_SHIFT		5
#define ADC_SQR1_SQ13_MASK		(0x1F<<0)		/* RW bit[4:0] 规则序列第13个转换的输入源 */
#define ADC_SQR1_SQ13_SHIFT		0

/**
 * ADC_SQR2 - ADC 规则序列寄存器1
 */
#define ADC_SQR1_SQ12_MASK		(0x1F<<25)		/* RW bit[29:25] 规则序列第12个转换的输入源 */
#define ADC_SQR1_SQ12_SHIFT		25
#define ADC_SQR1_SQ11_MASK		(0x1F<<20)		/* RW bit[24:20] 规则序列第11个转换的输入源 */
#define ADC_SQR1_SQ11_SHIFT		20
#define ADC_SQR1_SQ10_MASK		(0x1F<<15)		/* RW bit[19:15] 规则序列第10个转换的输入源 */
#define ADC_SQR1_SQ10_SHIFT		15
#define ADC_SQR1_SQ9_MASK		(0x1F<<10)		/* RW bit[14:10] 规则序列第9个转换的输入源 */
#define ADC_SQR1_SQ9_SHIFT		10
#define ADC_SQR1_SQ8_MASK		(0x1F<<5)		/* RW bit[9:5] 规则序列第8个转换的输入源 */
#define ADC_SQR1_SQ8_SHIFT		5
#define ADC_SQR1_SQ7_MASK		(0x1F<<0)		/* RW bit[4:0] 规则序列第7个转换的输入源 */
#define ADC_SQR1_SQ7_SHIFT		0

/**
 * ADC_SQR3 - ADC 规则序列寄存器3
 */
#define ADC_SQR1_SQ6_MASK		(0x1F<<25)		/* RW bit[29:25] 规则序列第6个转换的输入源 */
#define ADC_SQR1_SQ6_SHIFT		25
#define ADC_SQR1_SQ5_MASK		(0x1F<<20)		/* RW bit[24:20] 规则序列第5个转换的输入源 */
#define ADC_SQR1_SQ5_SHIFT		20
#define ADC_SQR1_SQ4_MASK		(0x1F<<15)		/* RW bit[19:15] 规则序列第4个转换的输入源 */
#define ADC_SQR1_SQ4_SHIFT		15
#define ADC_SQR1_SQ3_MASK		(0x1F<<10)		/* RW bit[14:10] 规则序列第3个转换的输入源 */
#define ADC_SQR1_SQ3_SHIFT		10
#define ADC_SQR1_SQ2_MASK		(0x1F<<5)		/* RW bit[9:5] 规则序列第2个转换的输入源 */
#define ADC_SQR1_SQ2_SHIFT		5
#define ADC_SQR1_SQ1_MASK		(0x1F<<0)		/* RW bit[4:0] 规则序列第1个转换的输入源 */
#define ADC_SQR1_SQ1_SHIFT		0

//-----------------------------------------------------------------------------
// 设置 adc 规则通道序列长度. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_set_sq_len(HW_ADC_t *hwADC, int len)
{
    if ((len > 0) && (len <= 16))
	{
		unsigned int val;

		val = ((len - 1) << ADC_SQR1_LEN_SHIFT) & ADC_SQR1_LEN_MASK;
		hwADC->sqr1 &= ~ADC_SQR1_LEN_MASK;
		hwADC->sqr1 |= val;

		return 0;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// 获取 adc 规则通道序列长度. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_get_sq_len(HW_ADC_t *hwADC)
{
	int sql = (int)(hwADC->sqr1 & ADC_SQR1_LEN_MASK) >> ADC_SQR1_LEN_SHIFT;
	return sql + 1;
}

//-----------------------------------------------------------------------------
// 设置 adc 通道规则序列. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_set_sq(HW_ADC_t *hwADC, int rank, int channelID)
{
	unsigned int mask, val;

	if ((rank > 0) && (rank <= 6))
	{
		mask = 0x1F << ((rank - 1) * 5);
		val  = (channelID & 0x1F) << ((rank - 1) * 5);
		hwADC->sqr3 &= ~mask;
		hwADC->sqr3 |= val;
		return 0;
	}
	else if ((rank > 6) && (rank <= 12))
	{
		mask = 0x1F << ((rank - 7) * 5);
		val  = (channelID & 0x1F) << ((rank - 7) * 5);
		hwADC->sqr2 &= ~mask;
		hwADC->sqr2 |= val;
		return 0;
	}
	else if ((rank > 12) && (rank <= 16))
	{
		mask = 0x1F << ((rank - 13) * 5);
		val  = (channelID & 0x1F) << ((rank - 13) * 5);
		hwADC->sqr1 &= ~mask;
		hwADC->sqr1 |= val;
		return 0;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// 获取 adc 通道规则序列. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_get_sq(HW_ADC_t *hwADC, int rank)
{
	if ((rank > 0) && (rank <= 6))
	{
		return (int)(hwADC->sqr3 >> ((rank - 1) * 5)) & 0x1F;
	}
	else if ((rank > 6) && (rank <= 12))
	{
		return (int)(hwADC->sqr2 >> ((rank - 7) * 5)) & 0x1F;
	}
	else if ((rank > 12) && (rank <= 16))
	{
		return (int)(hwADC->sqr1 >> ((rank - 13) * 5)) & 0x1F;
	}

	return -1;
}

/**
 * ADC_JSQR - ADC 注入序列寄存器
 */
/*
 * 注入通道序列长度
 */
#define ADC_JLEN_1				0				/* 00: 1 个转换 */
#define ADC_JLEN_2				1				/* 01: 2 个转换 */
#define ADC_JLEN_3				2				/* 10: 3 个转换 */
#define ADC_JLEN_4				3				/* 11: 4 个转换 */

#define ADC_JSQR_LEN_MASK		(0x3<<20)		/* RW bit[21:20] 注入通道序列长度 */
#define ADC_JSQR_LEN_SHIFT		20

#define ADC_JSQR_4_MASK			(0x1F<<15)		/* RW bit[19:15] 注入序列第4 个转换的输入源 */
#define ADC_JSQR_4_SHIFT		15
#define ADC_JSQR_3_MASK			(0x1F<<10)		/* RW bit[14:10] 注入序列第3 个转换的输入源 */
#define ADC_JSQR_3_SHIFT		10
#define ADC_JSQR_2_MASK			(0x1F<<5)		/* RW bit[9:5] 注入序列第2 个转换的输入源 */
#define ADC_JSQR_2_SHIFT		5
#define ADC_JSQR_1_MASK			(0x1F<<0)		/* RW bit[4:0] 注入序列第1 个转换的输入源 */
#define ADC_JSQR_1_SHIFT		0

//-----------------------------------------------------------------------------
// 设置 adc 注入通道序列长度. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_set_jsq_len(HW_ADC_t *hwADC, int len)
{
	if ((len > 0) && (len <= 4))
	{
		unsigned int val;

		val = ((len - 1) << ADC_JSQR_LEN_SHIFT) & ADC_JSQR_LEN_MASK;
		hwADC->jsqr &= ~ADC_JSQR_LEN_MASK;
		hwADC->jsqr |= val;

		return 0;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// 获取 adc 注入通道序列长度. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_get_jsq_len(HW_ADC_t *hwADC)
{
	int ret = (int)(hwADC->jsqr & ADC_JSQR_LEN_MASK) >> ADC_JSQR_LEN_SHIFT;
	return ret + 1;
}

//-----------------------------------------------------------------------------
// 设置 adc 注入通道序列. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_set_jsq(HW_ADC_t *hwADC, int rank, int jchannelID)
{
	unsigned int mask, val;

	if ((rank > 0) && (rank <= 4))
	{
		mask = 0x1F << ((rank - 1) * 5);
		val  = (jchannelID & 0x1F) << ((rank - 1) * 5);
		hwADC->jsqr &= ~mask;
		hwADC->jsqr |= val;

		return 0;
	}

	return -1;
}

//-----------------------------------------------------------------------------
// 获取 adc 注入通道序列. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_get_jsq(HW_ADC_t *hwADC, int rank)
{
	if ((rank > 0) && (rank <= 4))
	{
		return (int)(hwADC->jsqr >> ((rank - 1) * 5)) & 0x1F;
	}

	return -1;
}

/**
 * ADC_JDRx (1-4) - ADC 注入数据寄存器
 */
#define ADC_JDATA_MASK			0xFFFF			/* RO bit[15:0] ADC 注入转换结果 */

/**
 * ADC_DR - ADC 规则数据寄存器
 */
#define ADC_DATA_MASK			0xFFFF			/* RO ADC 规则转换结果受控文档 */

//-----------------------------------------------------------------------------
// 获取 adc 注入通道转换值. 从 1 开始编号
//-----------------------------------------------------------------------------

static inline int adc_get_jsq_result(HW_ADC_t *hwADC, int rank)
{
	int ret = -1;

	switch (rank)
	{
		case 1: ret = (int)(hwADC->jdr1 & ADC_JDATA_MASK); break;
		case 2: ret = (int)(hwADC->jdr2 & ADC_JDATA_MASK); break;
		case 3: ret = (int)(hwADC->jdr3 & ADC_JDATA_MASK); break;
		case 4: ret = (int)(hwADC->jdr4 & ADC_JDATA_MASK); break;
	}

	return ret;
}

#ifdef __cplusplus
}
#endif

#endif // _LS2K_ADC_HW_H


