
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

int backn[1000];
int target = -1;
u8 color = DEFAULT_CHAR_COLOR;
int isESC = 0;
int endESC ;
char targetChar[100];
int lenChar=0;


/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	
	
	
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	p_tty->p_console->cursor = p_tty->p_console->original_addr;

	if (nr_tty == 0) {
		/* 第一个控制台沿用原来的光标位置 */
		p_tty->p_console->cursor = disp_pos / 2;
		disp_pos = 0;
	}
	else {
		out_char(p_tty->p_console, nr_tty + '0');
		out_char(p_tty->p_console, '#');
	}

	
	set_cursor(p_tty->p_console->cursor);
}


/*======================================================================*
			   is_current_console
*======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
	return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);

	switch(ch) {
	case '\n':
		if(isESC){
		int midk = p_con->cursor-startOp-lenChar;
		while(midk>0){
		*(p_vmem-2*(midk)+1)= DEFAULT_CHAR_COLOR;
		midk--;
		}
		int i=p_con->cursor-startOp;
		while(i>=lenChar){
			
			int match = 1,j=lenChar-1;
			while(j>=0){
				
				if(*(p_vmem-2*(i-j))!=targetChar[j]){
						match = 0;
						break;
					}
				j--;
 				}
			if(match){
				
				
						

			    j = lenChar-1;
				
			    while(j>=0){
				*(p_vmem-2*(i-j)+1)= 0x04;
				j--;
					}
 				}
				i--;
			  }
		lenChar = 0;
			}
		
		else{
		if (p_con->cursor < p_con->original_addr +
		    p_con->v_mem_limit - SCREEN_WIDTH) {
                	target++;
			backn[target] = p_con->cursor;
			p_con->cursor = p_con->original_addr + SCREEN_WIDTH * 
				((p_con->cursor - p_con->original_addr) /
				 SCREEN_WIDTH + 1);
		 }
		}
		break;
	case '\b':
		if (p_con->cursor > p_con->original_addr) {
			if(*(p_vmem-2)==0x0&&*(p_vmem-4)==' '){
			p_con->cursor--;
			*(p_vmem-2) = ' ';
			*(p_vmem-1) = DEFAULT_CHAR_COLOR;

			p_con->cursor--;
			*(p_vmem-2) = ' ';
			*(p_vmem-1) = DEFAULT_CHAR_COLOR;

			p_con->cursor--;
			*(p_vmem-2) = ' ';
			*(p_vmem-1) = DEFAULT_CHAR_COLOR;

			p_con->cursor--;
			*(p_vmem-2) = ' ';
			*(p_vmem-1) = DEFAULT_CHAR_COLOR;

			p_con->cursor--;
			*(p_vmem-2) = ' ';
			*(p_vmem-1) = DEFAULT_CHAR_COLOR;
	
			}
			else if((p_con->cursor - p_con->original_addr)%SCREEN_WIDTH==0){
			if(target>=0){
               		p_con->cursor = backn[target];
			target--;}
						
			}
			else{
			p_con->cursor--;
			*(p_vmem-2) = ' ';
			*(p_vmem-1) = DEFAULT_CHAR_COLOR;
			}
		}
		break;
         case 0x9:
                *p_vmem++ = ' ';
                *p_vmem++ = DEFAULT_CHAR_COLOR;
			p_con->cursor++;
                *p_vmem++ = ' ';
                *p_vmem++ = DEFAULT_CHAR_COLOR;
			p_con->cursor++;
                *p_vmem++ = ' ';
                *p_vmem++ = DEFAULT_CHAR_COLOR;
			p_con->cursor++;
                *p_vmem++ = ' ';
                *p_vmem++ = DEFAULT_CHAR_COLOR;
			p_con->cursor++;
                *p_vmem++ = 0x0;
                *p_vmem++ = DEFAULT_CHAR_COLOR;
			p_con->cursor++;
                 break;
	case 0x1:
                if(!isESC){
 		isESC = 1;
		endESC = p_con->cursor;
		*p_vmem++ = ' ';
                *p_vmem++ = DEFAULT_CHAR_COLOR;
		p_con->cursor++;
		color = 0x04;}
		else{
		lenChar = 0;
		isESC = 0;
		color = DEFAULT_CHAR_COLOR;
		while(p_con->cursor!=endESC){
		         p_con->cursor--;
			
			*(p_vmem-2) = ' ';
			*(p_vmem-1) = DEFAULT_CHAR_COLOR;
			p_vmem -=2;
		}
		int midk = p_con->cursor-startOp;
		while(midk>0){
		*(p_vmem-2*(midk)+1)= DEFAULT_CHAR_COLOR;
		midk--;
		}
		}
		
		break;
	case 0x2:
		if(!isESC){
			int i = 1;
			if(p_con->cursor>startOp){
			//while(i<=(2*(p_con->cursor-startOp+2))){
			while(p_con->cursor>startOp){
				p_con->cursor--;
				*(p_vmem-2*i) = ' ';
				*(p_vmem-2*i+1) = DEFAULT_CHAR_COLOR;	
				i++;		
			}}
		}
		break;
	default:
		if (p_con->cursor <
		    p_con->original_addr + p_con->v_mem_limit - 1) {
			*p_vmem++ = ch;
			*p_vmem++ = color;
			p_con->cursor++;
			if(isESC){
           		
			targetChar[lenChar] = ch;
			lenChar++;
	         	}
		}
		break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}

	flush(p_con);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
	if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
		return;
	}

	nr_current_console = nr_console;

	set_cursor(console_table[nr_console].cursor);
	set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

