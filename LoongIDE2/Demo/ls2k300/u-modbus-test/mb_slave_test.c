/*
 * mb_slave_test.c
 *
 * created: 2020/6/26
 * authour: 
 */

#include "bsp.h"
#include "modbus/app/mb_cfg.h"

#if MODBUS_CFG_SLAVE_EN

#include "ls2k_uart.h"
#include "modbus/src/mb.h"

//-----------------------------------------------------------------------------

/*
 * 可以配置多通道
 */
 
static MODBUS_t *mb_Slave;

#define MB_NODEADDR     15

void mb_cfg_slave(void)
{
    mb_Slave = modbus_config_node(MB_NODEADDR,        // Node Address
                                  MODBUS_SLAVE,       // slave
                                  0,                  // rx timeout = 0 when slave
                                  MODBUS_MODE_RTU,    // MODBUS_MODE_ASCII,  //
                                  (void *)devUART4,   // Modbus device of ...
                                  115200,             // baudrate
                                  8,                  // bits
                                  'N',                // parity: 'N'/'E'/''
                                  1,                  // stops
                                  MODBUS_WR_EN );     // wr_en

}

#endif // #if MODBUS_CFG_SLAVE_EN

