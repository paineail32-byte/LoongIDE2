/*
 * dc_test.c
 *
 * created: 2025-03-16
 *  author: 
 */

#include "bsp.h"

#if BSP_USE_DC

#include "ls2k_dc.h"

void dc_test(void)
{
    char buf[32];

    fb_open();
    fb_cons_clear();

    fb_cons_puts("ª∂”≠ π”√¡˙–æ LS2K0300!\r\n");
    fb_cons_puts("ª∂”≠ π”√¡˙–æ LS2K0300!\r\n");

//  fb_showbmp(400, 200, "/mmc0/bmp/logo64.bmp");
    fb_showbmp(360, 180, "/mmc0/logo64.bmp");

/*
    fb_drawline(20, 20, 800-20, 480-20, cidxWHITE);
    fb_drawrect(50, 50, 800-50, 480-50, cidxRED);
 */
}

#endif

