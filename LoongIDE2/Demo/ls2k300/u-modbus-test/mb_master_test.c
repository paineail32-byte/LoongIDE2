/*
 * mb_master_test.c
 *
 * created: 2020/6/26
 * authour: 
 */

#include "bsp.h"
#include "modbus/app/mb_cfg.h"

#if MODBUS_CFG_MASTER_EN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osal.h"

#include "ls2k_uart.h"
#include "modbus/src/mb.h"

//-----------------------------------------------------------------------------

#define MBSVR_STACK_SIZE    4*1024
#define MBSVR_PRIO          8

static osal_task_t  mb_svr_task;

//-----------------------------------------------------------------------------

/*
 * 可以配置多通道
 */
static MODBUS_t *mb_Master;

#define MB_NODEADDR     0

void mb_cfg_master(void)
{
    mb_Master = modbus_config_node(MB_NODEADDR,        // Node Address
                                   MODBUS_MASTER,      // master
                                   100,                // rx timeout = ticks?
                                   MODBUS_MODE_RTU,    // MODBUS_MODE_ASCII,  //
                                   (void *)devUART6,   // Modbus device
                                   115200,             // baudrate
                                   8,                  // bits
                                   'N',                // parity: 'N'/'E'/''
                                   1,                  // stops
                                   MODBUS_WR_EN );     // wr_en

}

static void mb_master_task(void *arg)
{
    /*
     * Add mbsvr initialize code here.
     */
    mb_cfg_master();

    osal_msleep(10);

    for ( ; ; )
    {
        uint16_t err, regs[10];

        // modbus_set_address(mb_Master, 15);

        err = modbus_master_fc03_read_holding_register(
                        mb_Master,          // master, uart4
                        15,                 // slave_node,
                        0,                  // slave_addr,
                        regs,               // p_reg_tbl,
                        10                  // nbr_regs
                        );

        if (err == 0)
        {
            int i;
            for (i=0; i<10; i++)
                printk("fc03 read(Addr=15): holding[%i]=%i\r\n", i, (int)regs[i]);

            printk("\r\n");
        }
        else
        {
            printk("fc03 read(Addr=15): err = %i\r\n", (int)err);
        }

#if 0
        osal_msleep(10);

        modbus_set_address(mb_Master, 18);
        err = modbus_master_fc04_read_in_register(
                        mb_Master,          // master, uart4
                        18,                 // slave_node,
                        0,                  // slave_addr,
                        regs,               // p_reg_tbl,
                        2                   // nbr_regs
                        );

        if (err == 0)
            printk("fc04 read(Addr=18): in[0]=0x%04x, in[1]=0x%04x\r\n", (int)regs[0], (int)regs[1]);
        else
            printk("fc04 read(Addr=18): err = %i\r\n", (int)err);

#endif

        osal_msleep(100);  // 100 可用.
    }
}

/*
 * 创建一个 modbus master 线程
 */
int start_mb_master_task(void)
{
    mb_svr_task = osal_task_create("Modbus Svr",
                                    MBSVR_STACK_SIZE,
                                    MBSVR_PRIO,
                                    10,
                                    mb_master_task,
                                    NULL);

    if (mb_svr_task == NULL)
    {
        printk("create modbus master task fail!\r\n");
		return -1;
	}

    printk("create modbus master task successful.\r\n");
    
    return 0;
}

#endif // #if MODBUS_CFG_MASTER_EN

