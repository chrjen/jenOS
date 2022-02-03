/* Responsible for setting up and maintaining the terminal interface the user
 * sees. It has functions for printing and scrolling.
 *
 * Nothing will be visible until you call terminal_push() as everything is
 * written first to a buffer.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.h"


uint16_t terminal_buffer[TERMINAL_BUFFER_SIZE];

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_ptr;


uint8_t make_color(uint8_t fg, uint8_t bg)
{
	return fg | bg << 4;
}

uint16_t make_vgaentry(char c, uint8_t color)
{
	uint16_t c16 = c;
	uint16_t color16 = color;
	return c16 | color16 << 8;
}

size_t strlen(const char* str)
{
	size_t ret = 0;
	while ( str[ret] != 0 )
		ret++;
	return ret;
}


void terminal_initialize()
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);
	terminal_ptr = (uint16_t*) 0xB8000;
	for ( size_t y = 0; y < VGA_HEIGHT; y++ )
		for ( size_t x = 0; x < VGA_WIDTH; x++ )
		{
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = make_vgaentry(' ', terminal_color);
		}
}

void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = make_vgaentry(c, color);
}

void terminal_putchar(char c)
{
	if (c == '\n')
	{
		terminal_column = 0;
		if (terminal_row+1 >= VGA_HEIGHT)
		{
			terminal_scroll();
		}
		else
		{
			terminal_row++;
		}
		return;
	}

	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if ( ++terminal_column == VGA_WIDTH )
	{
		terminal_column = 0;
		if ( ++terminal_row == VGA_HEIGHT )
		{
			terminal_row = 0;
		}
	}
}

void terminal_scroll()
{
	for (size_t y = 2; y < VGA_HEIGHT-1; y++)
	{
		for (size_t x = 0; x < VGA_WIDTH; x++)
		{
			terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y+1) * VGA_WIDTH + x];
		}
	}

	/* Clears the new line before use.  */
	for (size_t x = 0; x < VGA_WIDTH; x++)
	{
		terminal_buffer[(VGA_HEIGHT-1) * VGA_WIDTH + x] = make_vgaentry(' ', terminal_color);
	}
}

void terminal_push()
{
	for ( size_t i = 0; i < TERMINAL_BUFFER_SIZE; i++ )
	{
		terminal_ptr[i] = terminal_buffer[i];
	}
}

void terminal_writestring(const char* data)
{
	size_t datalen = strlen(data);
	for ( size_t i = 0; i < datalen; i++ )
		terminal_putchar(data[i]);
}
