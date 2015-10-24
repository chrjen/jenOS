#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"


void sleep(int sleep)
{
	for (volatile long i = 0; i < 1000000 * sleep; i++)
	{
		/* DO NOTHING */
	}
}

static const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";

char* to_string(char str[], int num, int base)
{
	int i, rem, len = 0, n;

	n = num;
    do
    {
		len++;
        n /= base;
    } while (n != 0);
    
	for (i = 0; i < len; i++)
    {
        rem = num % base;
        num = num / base;
        str[len - (i + 1)] = digits[rem];
    }
    str[len] = '\0';
	return str;
}


void kmain()
{
	terminal_initialize();
	terminal_writestring("jenOS (c) v0.0.2!\n");
	terminal_setcolor(make_color(COLOR_LIGHT_RED, COLOR_BLACK));
	terminal_writestring("Hello World! :D\n");
	terminal_setcolor(make_color(COLOR_LIGHT_BLUE, COLOR_BLACK));

	char num[64];
	char c[2];
	c[1] = '\n';
	for (size_t i = 1; 1; i++)
	{
		if (i > 255)
		{
			i = 1;
		}

		terminal_writestring(to_string(num, terminal_row, 10));
		terminal_writestring(": The value ");
		terminal_writestring(to_string(num, i, 10));
		terminal_writestring(" (0x");
		terminal_writestring(to_string(num, i, 16));
		terminal_writestring(") ");
		terminal_writestring("is character ");
		c[0] = i;
		terminal_writestring(c);
		terminal_push();
		sleep(500);
	}
	terminal_push();
}
