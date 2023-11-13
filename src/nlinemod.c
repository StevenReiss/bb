/********************************************************/
/*							*/
/*		LINEMOD.C				*/
/*							*/
/*	This module contains the code for maintaining	*/
/*	the current file as a list of lines.  It keeps	*/
/*	a table of line numbers.  For each line, it	*/
/*	saves the pointer for access into the temporary */
/*	file.						*/
/*							*/
/*	The line number table is segmented into rather	*/
/*	large blocks.  This allows for both dynamic	*/
/*	allocation and unlimited (virtually) growth.	*/
/*	There is a fixed table that shows where each	*/
/*	block begins.  Since these blocks are fixed	*/
/*	size, it is easy to actually find a line	*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include "global.h"
#include "editor.h"
#include "linemod.h"
#include "bufmod.h"
#include "tempmod.h"
#include "lbuff.h"
#include "parms.h"




/********************************************************/
/*							*/
/*	External routines				*/
/*							*/
/********************************************************/


extern		lmove();	/* move long data (asm) */

extern		FLget();	/* read line from file	*/
extern		FLput();	/* write line to file	*/




/********************************************************/
/*							*/
/*	Internal routines				*/
/*							*/
/********************************************************/


static		loadfile();
static		dumpfile();
static		linins();
static		lindel();
static	void	find_line();
static	void	move_gap();
static	void	add_to_gap();
static	void	insert_in_gap();
static	void	extend_gap();




/********************************************************/
/*							*/
/*	Functions/ Parameter definitions		*/
/*							*/
/********************************************************/


#define DELTA_SIZE	20380
#define MIN_SIZE	40860
#define GAP_SIZE	1024
#define EXPAND_SIZE	512

#define NMMARKS 	32



/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/


static	INT	region_size;
static	INT	file_size;
static	INT	gap_size;
static	STRING	buf_ptr;
static	INT	gap_line;
static	STRING	gap_pos;
static	INT	cur_line;
static	STRING	cur_pos;
static	BYTE	edit_on_get;

static	INT	mrkcnt; 	/* number marks current */
static	LINE	mrksav[NMMARKS];/* mark line buffer	*/
static	INT	mrkid[NMMARKS]; /* id of markers	*/
static	BYTE	mrkfil[NMMARKS];/* file id of markers	*/

extern	INT	TEMPdirtcnt;	/* # writes done counter*/

static	LINE	lastline;
static	BOOL	readyfg = FALSE;
static	BOOL	loadingfg = FALSE;





/********************************************************/
/*							*/
/*	LNinit -- initialize line record for file	*/
/*							*/
/********************************************************/


LNinit()
{
   static BOOL firsttime = TRUE;
   register INT i,j;

   TMinit();

   if (firsttime) region_size = 0;

   i = FLget_size();
   i = i + DELTA_SIZE;
   if (i < MIN_SIZE) i = MIN_SIZE;

   if (region_size < i && region_size > 0) {
      free(buf_ptr);
      region_size = 0;
    };

   if (region_size == 0) {
      region_size = i;
      buf_ptr = (char *) malloc(i);
    };

   gap_pos = buf_ptr;
   gap_size = GAP_SIZE;
   file_size = 0;
   cur_line = 0;
   cur_pos = buf_ptr+GAP_SIZE;
   gap_line = 0;
   gap_pos = buf_ptr;
   *buf_ptr = 0;
   edit_on_get = FALSE;

   lastline = 0;			/* empty	*/
   PSET(filesize,0);

   if (firsttime)
    { mrkcnt = 0;
      for (i = 0; i < NMMARKS; ++i) mrkid[i] = 0;
    };

   loadfile();				/* get input	*/

   readyfg = TRUE;
   firsttime = FALSE;
   TEMPdirtcnt = 0;
}





/********************************************************/
/*							*/
/*	LNclose -- close line areas			*/
/*							*/
/********************************************************/


LNclose(savefg)
   INT savefg;
{
   if (!readyfg) return FALSE;

   if (savefg) dumpfile();

   TMclose();

   readyfg = FALSE;

   return TRUE;
}





/********************************************************/
/*							*/
/*	LNcheck -- check on user given position 	*/
/*							*/
/********************************************************/


LINE LNcheck(pos)
   LINE pos;
{
   if (pos < 0) pos = 0;
   if (pos > lastline) pos = lastline;

   return pos;
}





/********************************************************/
/*							*/
/*	LNforce -- ensure file goes to given position	*/
/*							*/
/********************************************************/


LNforce(pos)
   LINE pos;
{
   if (pos < 0) ABORT;

   while (pos > lastline) linins(lastline,NULL);
}






/********************************************************/
/*							*/
/*	LNget -- fill line buffer with given line	*/
/*							*/
/********************************************************/


LNget(ln)
   LINE ln;
{
   register STRING s1,s2;

   if (ln < 0) ABORT;

   if (ln >= lastline) LBlinebuf[0] = 0;
   else {
      find_line(ln);
      s1 = (STRING) LBlinebuf;
      for (s2 = cur_pos; *s2 != '\n' && *s2 != 0; ++s2) *s1++ = *s2;
      *s1 = 0;
      if (edit_on_get) LBedit(LBF_INTAB|LBF_FMT1);
    };

   return (ln < lastline);
}





/********************************************************/
/*							*/
/*	LNput -- replace current line			*/
/*							*/
/********************************************************/


LNput(ln,insfg)
   LINE ln;
   INT insfg;
{
   CHAR tempbuf[LINESIZE];
   ADDR v;

   if (ln < 0) ABORT;

   if (insfg != 0) return LNputin(ln);

   move_gap(ln);
   add_to_gap(tempbuf);

   v = TMput(tempbuf);
   HSline(HSL_PUT,ln,v);

   insert_in_gap(LBlinebuf);

   return TRUE;
}





/********************************************************/
/*							*/
/*	LNputin -- insert given line at end of file	*/
/*							*/
/********************************************************/


LNputin(ln)
   LINE ln;
{
   register INT j;

   if (ln < 0) ABORT;

   HSline(HSL_INS,ln,(LINE)0);

   linins(ln,LBlinebuf);

   return TRUE;
}




/********************************************************/
/*							*/
/*	LNgetdel -- delete current line 		*/
/*							*/
/********************************************************/


LNgetdel(ln)
   LINE ln;
{
   register INT i;
   ADDR v;

   if (ln < 0) ABORT;

   if (ln >= lastline) {
      LBlinebuf[0] = 0;
      return FALSE;
    };

   lindel(ln,LBlinebuf);

   v = TMput(LBlinebuf);
   HSline(HSL_DEL,ln,v);

   return TRUE;
}





/********************************************************/
/*							*/
/*	LNundo -- undo line changes			*/
/*							*/
/********************************************************/


LNundo(typ,line,addr)
   HISTLINE typ;
   LINE line;
   ADDR addr;
{
   CHAR tempbuf[LINESIZE];

   switch(typ) {
      case HSL_PUT	:
	 move_gap(line);
	 add_to_gap(tempbuf);
	 TMget(addr,tempbuf);
	 insert_in_gap(tempbuf);
	 break;

      case HSL_INS	:
	 lindel(line);
	 break;

      case HSL_DEL	:
	 TMget(addr,tempbuf);
	 linins(line,tempbuf);
	 break;
    };
}






/********************************************************/
/*							*/
/*	Markers 					*/
/*							*/
/*	The following routines supply a 'smart' marking */
/*	facility that takes modifications to the file	*/
/*	into account.  It allows the user to specify	*/
/*	up to 256 markers (actual limit is given in	*/
/*	this modules header).  Each mark has a line	*/
/*	number associated with it.  The editor attempts */
/*	to insure that the marker remains pointing to	*/
/*	that line regardless of what else happens to	*/
/*	the file, or if the user switches file.  The	*/
/*	code is not fail proof however, and will not	*/
/*	save the position under all circumstances.	*/
/*							*/
/********************************************************/


/********************************************************/
/*							*/
/*	LNmark -- save marker for current line		*/
/*							*/
/********************************************************/


LNmark(id,ln)
   INT id;			/* marker id		*/
   LINE ln;			/* line number to store */
{
   register INT i;

   for (i = 0; i < mrkcnt; ++i) /* find place for id	*/
      if (mrkid[i] == id) break;
   if (i == mrkcnt) ++mrkcnt;

   HSlmark(mrkid[i],mrksav[i],mrkfil[i]);

   mrkid[i] = id;		/* save marker		*/
   mrksav[i] = ln;
   mrkfil[i] = PVAL(fileid);
}





/********************************************************/
/*							*/
/*	LNgetmark -- get marker value given id		*/
/*							*/
/********************************************************/


LINE
LNgetmark(id)
   INT id;			/* marker id to go to	*/
{
   register INT i;

   for (i = 0; i < mrkcnt; ++i) /* find marker in table */
      if (mrkid[i] == id) break;
   if (i == mrkcnt) return -1;
   if (mrkfil[i] != PVAL(fileid)) return -1;

   return mrksav[i];		/* return resultant line*/
}





/********************************************************/
/*							*/
/*	LNunmark -- unmark given id			*/
/*							*/
/********************************************************/


LNunmark(id)
   INT id;			/* id of marker to del	*/
{
   register INT i;

   for (i = 0; i < mrkcnt; ++i) /* find id in table	*/
      if (mrkid[i] == id) break;
   if (i == mrkcnt) return;

   --mrkcnt;
   for ( ; i < mrkcnt; ++i)
    { mrkid[i] = mrkid[i+1];
      mrksav[i] = mrksav[i+1];
      mrkfil[i] = mrkfil[i+1];
    };
}





/********************************************************/
/*							*/
/*	loadfile, dumpfile -- handle i/o of file	*/
/*							*/
/********************************************************/


static
loadfile()		 /* input source file	 */
{
   loadingfg = TRUE;

   LBmove(FLget,0l,0l,
	  LNputin,0l,0l,
	  LBF_DPARM|LBF_DINCP|LBF_INTAB|LBF_STREAM|LBF_FMT1);

   loadingfg = FALSE;
}





static
dumpfile()		 /* output source file	 */
{
   loadingfg = TRUE;

   LBmove(LNget,0l,0l,
	  FLput,0l,0l,
	  LBF_SPARM|LBF_SINCP|LBF_OUTAB|LBF_FMT2);

   loadingfg = FALSE;
}






/********************************************************/
/*							*/
/*	linins -- insert line in file			*/
/*							*/
/********************************************************/


static
linins(lnum,ins)
   LINE lnum;
   STRING ins;
{
   register LINE ptr,bptr;
   register INT i;

   if (ins == NULL) ins = "";

   move_gap(lnum);
   insert_in_gap(ins);

   ++lastline;

   if (!loadingfg)
      for (i = 0; i < mrkcnt; ++i)
	 if (mrksav[i] >= lnum && mrkfil[i] == PVAL(fileid)) ++mrksav[i];

   PSET(filesize,lastline);

   return;
}





/********************************************************/
/*							*/
/*	lindel -- delete line from file 		*/
/*							*/
/********************************************************/


static
lindel(lnum,buf)
   LINE lnum;			/* line to delete	*/
{
   register LINE bptr,ptr;
   register INT i;

   if (lnum >= lastline) return;

   move_gap(lnum);
   add_to_gap(buf);

   --lastline;

   if (!loadingfg)
      for (i = 0; i < mrkcnt; ++i)
	 if (mrksav[i] > lnum && mrkfil[i] == PVAL(fileid)) --mrksav[i];

   PSET(filesize,lastline);

   return;
}





/********************************************************/
/*							*/
/*	find_line -- get ptr to given line in buffer	*/
/*							*/
/********************************************************/


static void
find_line(line)
   INT line;
{
   INT baseln;
   STRING basepos,gend;
   INT i0;

   gend = gap_pos + gap_size;

   i0 = line;
   baseln = 0;
   basepos = (gap_line == 0 ? gend : buf_ptr);

   if (i0 > abs(lastline - line)) {
      i0 = abs(lastline - line);
      baseln = lastline;
      basepos = buf_ptr+file_size+gap_size;
    };

   if (i0 > abs(gap_line - line)) {
      i0 = abs(gap_line - line);
      baseln = gap_line;
      basepos = gend;
    };

   if (i0 > abs(cur_line - line)) {
      i0 = abs(cur_line - line);
      baseln = cur_line;
      basepos = cur_pos;
    };

   while (baseln < line) {
      while (*basepos++ != '\n');
      ++baseln;
      if (baseln == gap_line) basepos += gap_size;
    };

   while (baseln > line) {
      if (basepos == gend) basepos -= gap_size;
      --basepos;
      while (basepos != gend && *--basepos != '\n');
      if (basepos != gend) ++basepos;
      --baseln;
    };

   cur_pos = basepos;
   cur_line = baseln;
}




/********************************************************/
/*							*/
/*	move_gap -- move gap to before given line	*/
/*							*/
/********************************************************/


static void
move_gap(ln)
   INT ln;
{
   if (gap_line == ln) return;

   find_line(ln);

   if (ln > gap_line) {
      bcopy(gap_pos+gap_size,gap_pos,cur_pos-gap_pos-gap_size);
    }
   else {
      bcopy(cur_pos,cur_pos+gap_size,gap_pos-cur_pos);
    };

   gap_pos = cur_pos-gap_size;
   gap_line = cur_line;
}




/********************************************************/
/*							*/
/*	add_to_gap -- add current line to gap		*/
/*							*/
/********************************************************/


static void
add_to_gap(buf)
   STRING buf;
{
   register INT i;
   register STRING pt;

   pt = gap_pos+gap_size;
   i = 0;

   while (*pt != '\n') {
      *buf++ = *pt++;
      ++i;
    };

   i += 1;
   *buf = 0;

   gap_size += i;
   file_size -= i;

   if (cur_line == gap_line) cur_pos += i;
}





/********************************************************/
/*							*/
/*	insert_in_gap -- add text into gap before line	*/
/*							*/
/********************************************************/


static void
insert_in_gap(txt)
   STRING txt;
{
   register INT i;
   register STRING pt;

   i = strlen(txt)+1;
   if (i >= gap_size) extend_gap(i);

   pt = gap_pos;
   while (*txt != 0) *pt++ = *txt++;
   *pt++ = '\n';

   gap_size -= i;
   file_size += i;
   gap_pos += i;
}






/********************************************************/
/*							*/
/*	extend_gap -- add more space to gap		*/
/*							*/
/********************************************************/


static void
extend_gap(mn)
   INT mn;
{
   INT exp,nsz;
   STRING nptr,gend,fend;

#define ADJUST(x) ((long) x) - ((long) buf_ptr) + nptr

   for (exp = EXPAND_SIZE; exp < mn*2; exp *= 2);

   if (file_size+gap_size+exp >= region_size) {
      for (nsz = region_size*2; nsz < file_size+gap_size+exp; nsz *= 2);
      nptr = (char *) realloc(buf_ptr,nsz);
      cur_pos = ADJUST(cur_pos);
      gap_pos = ADJUST(gap_pos);
      buf_ptr = nptr;
    };

   gend = gap_pos+gap_size;
   fend = buf_ptr+file_size+gap_size;

   bcopy(gend,gend+exp,fend-gend);

   gap_size += exp;

   if (cur_pos >= gap_pos) cur_pos += exp;
}




/* end of linemod.c */
