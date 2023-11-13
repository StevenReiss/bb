/************************************************************************/
/*									*/
/*	termcurses.c -- terminal package using curses library		*/
/*									*/
/************************************************************************/

#include "global.h"
#include "parms.h"
#include "FILES"

#undef TRUE
#undef FALSE
#include <curses.h>



/************************************************************************/
/*									*/
/*	Parameters							*/
/*									*/
/************************************************************************/


#define MAXWINDOW	10


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




/************************************************************************/
/*									*/
/*	TRnew -- set up for a new terminal				*/
/*									*/
/************************************************************************/


TRnew(typ)
   char * typ;
{
   if (base_win != NULL) TRend();

   base_win = initscr();
   keypad(base_win,TRUE);
   scrollok(base_win,FALSE);

   cur_win = base_win;
   cur_winid = 0;

   wrefresh(cur_win);
}




/************************************************************************/
/*									*/
/*	TRclear -- clear the current window				*/
/*									*/
/************************************************************************/


TRclear()
{
   if (base_win == NULL) TRnew(NULL);

   wclear(cur_win);

   wrefresh(cur_win);
}



/************************************************************************/
/*									*/
/*	TRend -- finish with terminal					*/
/*									*/
/************************************************************************/


TRend()
{
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


TRdisp()
{
   wrefresh(cur_win);
}




TRdisp_goto(x,y)
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


TRprint(line,data)
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


TRins(col,line,data)
   int col,line;
   char * data;
{
   if (data != NULL) TRinsn(col,line,data,strlen(data));
}




TRinsn(col,line,data,ct)
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





/************************************************************************/
/*									*/
/*	TRtype -- handle character typed				*/
/*									*/
/************************************************************************/


TRtype(ch,fg)
   int ch;
   int fg;
{
   int och;
   int so;

   if (ch != '\b' && iscntrl(ch)) {
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

   if (fg && ch != '\b') winsch(cur_win,' ');

   waddch(cur_win,och);

   if (fg && ch == '\b') wdelch(cur_win);

   wrefresh(cur_win);
}




/************************************************************************/
/*									*/
/*	TRbells -- output a given number of bells			*/
/*									*/
/************************************************************************/


TRbells(ct)
   int ct;
{
   int i;

   for (i = 0; i < ct; ++i) beep();
}




/************************************************************************/
/*									*/
/*	TRgoto -- goto given screen position				*/
/*									*/
/************************************************************************/


TRgoto(x,y)
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


TRsgoto(x,y)
   int x,y;
{
   wmove(cur_win,y,x);
}




/************************************************************************/
/*									*/
/*	TRinfo -- get/set status info					*/
/*									*/
/************************************************************************/


TRinfo(rq)
   char * rq;
{
   int fg;

   if (rq == NULL) return -1;

   fg = TRUE;
   if (*rq == '-') { ++rq; fg = FALSE; }

   switch (*rq) {
      case 's' : return FALSE;
      case 'w' : return COLS;
      case 'l' : return LINES;
      case 'd' : return FALSE;
      case 'e' : return FALSE;
      case 'b' : return FALSE;
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


TRmark(fx,fy,tx,ty,bit)
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
	 if (!bit) ch &= ~ _STANDOUT;
	 else ch |= _STANDOUT;
	 mvwaddch(cur_win,y,x,ch);
       };
    };
}




/************************************************************************/
/*									*/
/*	TRcmark -- mark current character				*/
/*									*/
/************************************************************************/


TRcmark()
{
   int ch,y,x;

   getyx(cur_win,y,x);

   ch = winch(cur_win);
   ch ^= _STANDOUT;
   waddch(cur_win,ch);

   wmove(cur_win,y,x);
}




/* end of termcurses.c */
