#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <conio.h>
#include <math.h>
#include <string.h>

#include "font.h"

#define X_RES 320
#define Y_RES 200
#define FONT_SIZE 8

#define	DEG2RAD(angle_deg) (angle_deg * M_PI / 180.0)

typedef	unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned int uint16_t;

unsigned char *vga = (unsigned char *)0xa0000000L;
unsigned char *scr;

uint8_t	sine_table[256];

// sets VGA mode
void set_mode(unsigned char mode)
{
	asm {
		mov	al,[mode]
		xor	ah,ah
		int	0x10
	}
}

// waits for Vertical Blank
void wait_vbl(void)
{
	asm {
		mov	dx,0x3da
	}
 v0:
	asm {
		in	al,dx
		and	al,0x08
		jnz	v0
	}
 v1:
	asm {
		in	al,dx
		and	al,0x08
		jz	v1
	}
}

// set the VGA palette
void set_pal(uint8_t idx, uint8_t r, uint8_t g, uint8_t b)
{
	asm {
		mov	dx,0x3c8
		mov	al,[idx]
		out	dx,al
		inc	dx
		mov	al,[r]
		out	dx,al
		mov	al,[g]
		out	dx,al
		mov	al,[b]
		out	dx,al
	}
}

// copies data from local screen-buffer to VGA memory
void copy2vga(void)
{
	memcpy(vga,scr,X_RES*Y_RES);
}

// clears the screen buffer
void clr_scr(void)
{
	memset(scr, 0x00, X_RES*Y_RES);
}

// draws a char in screen memory
void draw_char(uint16_t x, uint8_t y, uint8_t color, char c)
{
	uint8_t 	i;
	uint8_t		v;
	uint16_t	idx;

	// simplified ATASCII to ASCII conversion!
	if ( (c > 32) && (c < 96) ) c -=32;

	// index calc
	idx = c * FONT_SIZE;

	// draw loop!
	for(i=0; i < 8; i++)
	{
	  v = magnus_fnt[idx+i];

	  if (v & 0x80)  *(scr + (y+i)*320 + (x+0)) = color;
	  if (v & 0x40)  *(scr + (y+i)*320 + (x+1)) = color;
	  if (v & 0x20)  *(scr + (y+i)*320 + (x+2)) = color;
	  if (v & 0x10)  *(scr + (y+i)*320 + (x+3)) = color;
	  if (v & 0x08)  *(scr + (y+i)*320 + (x+4)) = color;
	  if (v & 0x04)  *(scr + (y+i)*320 + (x+5)) = color;
	  if (v & 0x02)  *(scr + (y+i)*320 + (x+6)) = color;
	  if (v & 0x01)  *(scr + (y+i)*320 + (x+7)) = color;
	}
}

// draws string on screen buffer
void draw_string(uint16_t x, uint8_t y, uint8_t color, uint8_t *string)
{
  uint8_t idx = 0;

  while( string[idx] )
  {
	draw_char(x,y,color++,string[idx++]);
	x += FONT_SIZE;
  }
}

// draws message in screen buffer
void draw_sine_msg(char *msg, int8_t sx, int8_t dx, uint8_t y_shift, uint8_t color)
{
  int i,j;
  int x_center;
  int msg_len;


  msg_len = strlen(msg);

  // calculate center posision
  x_center = FONT_SIZE * (((X_RES/FONT_SIZE) - msg_len) / 2);

  for(i=0; i < msg_len; i++)
  {
   draw_char(x_center+(i*FONT_SIZE),y_shift + sine_table[(uint8_t)sx],color++,msg[i]);

   sx += dx;
  }
}

// main code starts here
void main(void)
{
	FILE 	in_file;
	int	i;

	int8_t	sx0,dx0,sx1,dx1;


	char	msg1[]="Happy Birthday Malfunction!";
	char	msg2[]="Let's all Your dreams come true! :)";
	char	msg3[]="2020.02.16";

	// precalculate the sine table for sine-wave effect
	for(i = 0; i < sizeof(sine_table); i++)
	{
	  sine_table[i] = 40 - 40 * sin( DEG2RAD(i * (360.0/256.0)));
	}

	// alloc memory for screen buffer!
	scr = malloc(X_RES*Y_RES);

	// allocation OK?
	if (scr == NULL)
	{
		printf("screen buffer allocation failed!\n");
		exit(-1);
	}

	// clear screen memory
	clr_scr();

	// set mode 13h (320x200 @ 256 colours)
	wait_vbl();
	set_mode(0x13);

	// initial parameters for message #1
	sx0 = 0;
	dx0 = 5;

	// initial parameters for message #2
	sx1 = 64;
	dx1 = 8;

	// animation loop!
	do
	{
		// wait for vertical-blank
		wait_vbl();
		// copy screen-buffer to vga frame-buffer
		copy2vga();

		// clear screen-buffer
		clr_scr();

		// draw message #1
		draw_sine_msg(msg1,sx0,dx0,0,0x20);
		sx0 += 1;

		// draw message #2
		draw_sine_msg(msg2,sx1,dx1,100,0x30);
		sx1 -= 3;

		// draw static message
		draw_string(240,192,0xc0,msg3);

	}while( !kbhit() );

	// free allocated memory
	free(scr);

	// restore text mode!
	wait_vbl();
	set_mode(0x03);
}
