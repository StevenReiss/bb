/************************************************************************/
/*									*/
/*	terminal.c -- terminal package using curses library		*/
/*									*/
/************************************************************************/

#include "global.h"
#include "parms.h"
#include "FILES"
#include "editor.h"

#undef TRUE
#undef FALSE
#include <curses.h>



/************************************************************************/
/*									*/
/*	Parameters							*/
/*									*/
/************************************************************************/


#define MAXWINDOW	10

#ifndef A_STANDOUT
#define A_STANDOUT	__STANDOUT
#endif



/************************************************************************/
/*									*/
/*	Local storage							*/
/*									*/
/************************************************************************/


static	WINDOW *	base_win = NULL;
static	WINDOW *	cur_win;
static	int		num_win;
static	int		cur_winid;
static	WINDOW *	aux_wins[MAXWINDOW];
static	int		backsp = '\b';
static	int		is_xterm = 0;




/************************************************************************/
/*									*/
/*	TRnew -- set up for a new terminal				*/
/*									*/
/************************************************************************/


void TRnew(typ)
   char * typ;
{
   char * s;

   if (base_win != NULL) TRend();

   base_win = initscr();
   keypad(base_win,TRUE);
   scrollok(base_win,FALSE);

   s = longname();
   if (strncmp(s,"xterm ",6) == 0) {
      is_xterm = 1;
      s = "\33[?9h";
      write(1,s,strlen(s));
    };

   cur_win = base_win;
   cur_winid = 0;

   wrefresh(cur_win);
}




/************************************************************************/
/*									*/
/*	TRclear -- clear the current window				*/
/*									*/
/************************************************************************/


void TRclear()
{
   if (base_win == NULL) TRnew(NULL);

   wclear(cur_win);
   wmove(cur_win,0,0);

   wrefresh(cur_win);
}



/************************************************************************/
/*									*/
/*	TRend -- finish with terminal					*/
/*									*/
/************************************************************************/


void TRend()
{
   char * s;

   if (is_xterm) {
      s = "\33[?9l";
      write(1,s,strlen(s));
    };

   endwin();

   base_win = cur_win = NULL;
   num_win = 0;
}



/************************************************************************/
/*									*/
/*	TRdisp -- update display					*/
/*	TRdisp_goto -- update display and move				*/
/*									*/
/************************************************************************/


void TRdisp()
{
   wrefresh(cur_win);
}




void TRdisp_goto(x,y)
   int x,y;
{
   wmove(cur_win,y,x);
   wrefresh(cur_win);
}



/************************************************************************/
/*									*/
/*	TRprint -- output line to display				*/
/*									*/
/************************************************************************/


void TRprint(line,data)
   int line;
   char * data;
{
   TRinsn(0,line,data,COLS);
}



/************************************************************************/
/*									*/
/*	TRins, TRinsn -- insert text at location			*/
/*									*/
/************************************************************************/


void TRinsn(col,line,data,ct)
   int col,line,ct;
   char * data;
{
   int ch;
   char * ptr;
   int i,x,y;

   if (line >= LINES) return;
   if (col + ct >= COLS) ct = COLS - col;
   if (data == NULL) data = "";

   getyx(cur_win,y,x);

   wmove(cur_win,line,col);

   ptr = data;

   for (i = 0; i < ct; ++i) {
      if (*ptr == 0) ch = ' ';
      else ch = *ptr++;

      if (iscntrl(ch)) {
	 wstandout(cur_win);
	 waddch(cur_win,ch + 'A' - 1);
	 wstandend(cur_win);
       }
      else if (ch == '\177') {
	 wstandout(cur_win);
	 waddch(cur_win,'?');
	 wstandend(cur_win);
       }
      else waddch(cur_win,ch);
    };

   wmove(cur_win,y,x);
}





void TRins(col,line,data)
   int col,line;
   char * data;
{
   if (data != NULL) TRinsn(col,line,data,strlen(data));
}




/************************************************************************/
/*									*/
/*	TRtype -- handle character typed				*/
/*									*/
/************************************************************************/


void TRtype(ch,fg)
   int ch;
   int fg;
{
   int och;
   int so;

   if (ch != '\b' && ch != backsp && iscntrl(ch)) {
      och = ch + 'A' - 1;
      so = TRUE;
    }
   else if (ch == '\177') {
      och = '?';
      so = TRUE;
    }
   else {
      och = ch;
      so = FALSE;
    };

   if (so) wstandout(cur_win);

   if (ch == backsp) {
      if (fg) {
	 waddch(cur_win,'\b');
	 wdelch(cur_win);
       }
      else waddstr(cur_win,"\b \b");
    }
   else {
      if (fg) winsch(cur_win,' ');
      waddch(cur_win,och);
    };

   wrefresh(cur_win);
}




/************************************************************************/
/*									*/
/*	TRbells -- output a given number of bells			*/
/*									*/
/************************************************************************/


void TRbells(ct)
   int ct;
{
   int i;

   for (i = 0; i < ct; ++i) {
      if (base_win != NULL) beep();
      else {
	 char buf[2];
	 buf[0] = '\7';
	 buf[1] = 0;
	 write(1,buf,1);
       };
    };
}




/************************************************************************/
/*									*/
/*	TRgoto -- goto given screen position				*/
/*									*/
/************************************************************************/


void TRgoto(x,y)
   int x,y;
{
   wmove(cur_win,y,x);
   wrefresh(cur_win);
}



/************************************************************************/
/*									*/
/*	TRsgoto -- go to given screen position silently 		*/
/*									*/
/************************************************************************/


void TRsgoto(x,y)
   int x,y;
{
   wmove(cur_win,y,x);
}




/************************************************************************/
/*									*/
/*	TRinfo -- get/set status info					*/
/*									*/
/************************************************************************/


int TRinfo(rq)
   char * rq;
{
   int fg,old;

   if (rq == NULL) return -1;

   fg = TRUE;
   if (*rq == '-') { ++rq; fg = FALSE; }

   switch (*rq) {
      case 's' : return FALSE;
      case 'w' : return COLS;
      case 'l' : return LINES;
      case 'd' : return FALSE;
      case 'e' : return FALSE;
      case 'b' : old = backsp;
		 if (*++rq != 0) backsp = (*rq)&0xff;
		 return old;
      default  : abort();
    };
}




/************************************************************************/
/*									*/
/*	TRwindow -- setup up new window, return id			*/
/*									*/
/************************************************************************/


int
TRwindow(upx,upy,btx,bty)
   int upx,upy,btx,bty;
{
   if (num_win >= MAXWINDOW) return -1;
   if (upy > bty || upx > btx) return -1;

   aux_wins[num_win++] = newwin(bty-upy+1,btx-upx+1,upy,upx);

   return num_win;
}




/************************************************************************/
/*									*/
/*	TRwinset -- change to given window				*/
/*									*/
/************************************************************************/


int
TRwinset(wd)
   int wd;
{
   int owd;

   owd = cur_winid;

   if (wd < 0 || wd > num_win) wd = 0;

   if (wd == 0) cur_win = base_win;
   else cur_win = aux_wins[wd-1];

   cur_winid = wd;

   return owd;
}




/************************************************************************/
/*									*/
/*	TRmark -- mark or unmark an area of the screen			*/
/*									*/
/************************************************************************/


void TRmark(fx,fy,tx,ty,bit)
   int fx,fy,tx,ty;
   int bit;
{
   int x,y,x0,y0,x1;
   int ch;

   for (y = fy; y <= ty; ++y) {
      x0 = (y == fy ? fx : 0);
      x1 = (y == ty ? tx : COLS-1);
      for (x = x0; x <= x1; ++x) {
	 ch = mvwinch(cur_win,y,x);
	 if (!bit) ch &= ~ A_STANDOUT;
	 else ch |= A_STANDOUT;
	 mvwaddch(cur_win,y,x,ch);
       };
    };
}




/************************************************************************/
/*									*/
/*	TRcmark -- mark current character				*/
/*									*/
/************************************************************************/


void TRcmark()
{
   int ch,y,x;

   getyx(cur_win,y,x);

   ch = winch(cur_win);
   ch ^= A_STANDOUT;
   waddch(cur_win,ch);

   wmove(cur_win,y,x);
}




/* end of terminal.c */
