/*
 * adc_test.c
 *
 * created: 2024-08-03
 *  author: 
 */

#include "bsp.h"

#if BSP_USE_ADC

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ls2k_adc.h"

void adc_test(void)
{
    #if 1
    {
        /*
         * 单通道测试
         */
        int val;

        ls2k_adc_open(NULL, NULL);

        #if 0
        {
            ls2k_adc_read(NULL, &val, 4, (void *)ADC_CH_4);
            printk("channel4=%d\n", val);

            ls2k_adc_read(NULL, &val, 4, (void *)ADC_CH_5);
            printk("channel5=%d\n", val);
            
            ls2k_adc_read(NULL, &val, 4, (void *)ADC_CH_6);
            printk("channel6=%d\n", val);
            
            ls2k_adc_read(NULL, &val, 4, (void *)ADC_CH_7);
            printk("channel7=%d\n", val);
        }
        #else
        {
            val = adc_read_1_fast(ADC_CH_4,  ADC_SAMP_64P);
            printk("channel4=%d\n", val);
            val = adc_read_1_fast(ADC_CH_5,  ADC_SAMP_64P);
            printk("channel5=%d\n", val);
            val = adc_read_1_fast(ADC_CH_6,  ADC_SAMP_64P);
            printk("channel6=%d\n", val);
            val = adc_read_1_fast(ADC_CH_7,  ADC_SAMP_64P);
            printk("channel7=%d\n", val);
        }
        #endif

        ls2k_adc_close(NULL, NULL);
    }
    #else
    {
        /*
         * 多通道测试
         */
         
        int i, values[16];
        ADC_Mode_t mode;
        
        mode.OutPhaseSel = ADC_OPHASE_SAME;
        mode.ClkDivider = 1;
        mode.DiffMode = 0;
        mode.ScanMode = 1;
        mode.ContinuousMode = 0;
        mode.TrigEdgeDown = 0;
        mode.ExternalTrigSrc = ADC_TRIG_SWATART;
        mode.DataAlignLeft = 0;                  
        
        mode.RegularChannelCount = 2;
	    mode.RegularChannels[0] = ADC_CH_4;
	    mode.SampleClocks[0] = ADC_SAMP_64P;
	    mode.RegularChannels[1] = ADC_CH_5;
	    mode.SampleClocks[1] = ADC_SAMP_64P;

        ls2k_adc_open(NULL, &mode);

        for (i=0; i<5; i++)
        {
            ls2k_adc_read(NULL, values, 8, 0);
            printk("ch[4]=%d, ch[5]=%d\n", values[0], values[1]);
        }

        ls2k_adc_close(NULL, NULL);

    }
    #endif
}

#endif
    
