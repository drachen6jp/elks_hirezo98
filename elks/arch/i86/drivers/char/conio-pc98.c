/*
 * conio API for PC98 Headless Console
 */

#include <linuxmt/config.h>
#include <linuxmt/types.h>
#include "conio.h"

void conio_init()
{
    early_getinit();
    cursor_on();
}

void bell(void)
{
}

void conio_putc(byte_t c)
{
    int tvram_x;
    int i;
    static int esc_seq = 0;
    static int esc_num = 0;


	if (c == 0x1B)
	    esc_seq = 1;
	else if ((c == '[') && (esc_seq == 1)) {
	    esc_seq = 2;
	}
	else if ((c >= 0x30) && (c <= 0x39) && (esc_seq == 2)) {
	    esc_num *= 10;
	    esc_num += (c-0x30);
	}
	else if ((c > 0x40) && (esc_seq == 2)) {
	    esc_seq = 3;
	    if (c == 'C') {
	        tvram_x = read_tvram_x();
	        tvram_x += esc_num * 2;
	        write_tvram_x(tvram_x);
	    }
	    else if (c == 'K') {
	        clear_tvram();
	    }
	    else if (c == 'H') {
	        write_tvram_x(0);
	    }
	    else if (c == 'J') {
	        for (i = 0; i < 2000; i++) {
	            early_putchar(' ');
	        }
	        write_tvram_x(0);
	    }
	}
	else {
	    esc_seq = 0;
	    esc_num = 0;
	}

	if (!esc_seq) {
	    early_putchar(c);
	}
	cursor_set(read_tvram_x());
}
