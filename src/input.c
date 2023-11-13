/********************************************************/
/*							*/
/*		INPUT.C 				*/
/*							*/
/*	This program is a generalized input processor	*/
/*   that accepts table specified character translates	*/
/*   and signal processing.  It can be used to return	*/
/*   commands (in the range 128-255) decoded from one	*/
/*   or more character sequences.  It is designed	*/
/*   primarily for use with the b editor.		*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/


#include <curses.h>
#ifdef TRUE
#undef TRUE
#undef FALSE
#endif

#include "global.h"
#include "editor.h"
#include "parms.h"
#include "cmds.h"
#include <signal.h>
#include <ctype.h>
#include <setjmp.h>
#include <sys/termios.h>

#include "FILES"

#ifdef TULIP
#undef getc
#define getc TULIPgetc
#endif




/********************************************************/
/*							*/
/*	Parameters					*/
/*							*/
/********************************************************/


#define DFLTTERM  "dv"          /* default terminal     */

#define BUFLEN	  4096		/* buffer length	*/




/********************************************************/
/*							*/
/*	External routines -- must be user supplied	*/
/*							*/
/********************************************************/


extern	void	INCLEAN();	/* clean up on abort	*/
extern	INT	INVERIFY();	/* verify a signal	*/
extern	void	ILLCHR();	/* illegal char seq	*/





/********************************************************/
/*							*/
/*	External -- system routines			*/
/*							*/
/********************************************************/





/********************************************************/
/*							*/
/*	Forward routine definitions			*/
/*							*/
/********************************************************/


static	void	INsignal();	/* process signals	*/
static	void	INabort();	/* abort required	*/
static	void	setupterminal();/* setup terminal modes */
static	void	setsig();	/* set signal from tbl	*/
static	int	seclookup();	/* look up secondard str*/
static	void	definput();	/* define input tables	*/
static	void	inittbls();	/* initialize tables	*/
static	void	findentry();	/* get descriptor str	*/
static	STRING	prcdef();	/* process definition	*/
static	STRING	prcsignal();	/* process signal	*/
static	STRING	prcsecond();	/* secondary header	*/
static	STRING	prcsecdef();	/* secondary definition */
static	STRING	prccurses();	/* curses definition	*/
static	void	finitbls();	/* clean up tables	*/
static	int	getcdef();	/* get char definition	*/
static	int	cmdtrans();	/* get command from name*/
static	int	getchr();	/* single character get */
static	LONG	srchtbl();	/* search name table	*/
static	void	secenter();	/* add secondary string */
static	LONG	sechash();	/* hash a secondary str */
static	int	term_name();	/* check for id match	*/
static	void	badinput();	/* abort on bad definit */
static	INT	nextchar();

#define KNOCKDOWN(rch,nch) (rch = (((rch = (nch)) == '?') ? '\177' : (rch&0x1f)))





/********************************************************/
/*							*/
/*	Storage definitions				*/
/*							*/
/********************************************************/


#define INFID 0 		/* input file id	*/

static	INT	sigchr; 	/* signal result char	*/
static	INT	unputch;	/* unput result char	*/
static	INT	unputch1;	/* second unput char	*/
static	INT	unputch2;	/* third unput char	*/
static	BOOL	intrfg; 	/* interrupt occurred	*/
static	BOOL	sigfg;		/* doing signal processing */
static	jmp_buf env;		/* interrupt environment*/
static	BOOL	envok = FALSE;	/* ok to longjmp	*/

static	BOOL	termdefd;	/* terminal defined flag*/

static	BOOL	INecho; 	/* echo mode		*/
static	BOOL	INlcnt; 	/* lcnt mode		*/
static	BOOL	INcbreak;	/* cbreak mode		*/
static	BOOL	INraw;		/* raw mode		*/
static	BOOL	INnline;	/* nline mode		*/

static	BYTE	secpfxln;	/* size of string	*/

static	BOOL	use_curses = FALSE;
static	BOOL	in_mouse = FALSE;



/********************************************************/
/*							*/
/*	Tables -- fixed definitions			*/
/*							*/
/********************************************************/

				/* FROM USER PROGRAM	*/
extern	LONG	CMDTBL[];	/* command names	*/
extern	INT	CMDTBLEN;	/* number of commands	*/
extern	BYTE	CMDVTBL[];	/* command char values	*/

#define C4(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))

				/* SYSTEM DEPENDENT	*/
#define SIGTBLEN 32		/* number of signals	*/
#define SIGFIRST 1
#define SIGLAST  31
				/* signal numbers should   */
				/*   correspond to actuals */
struct {			/* signal name-number pairs */
   int sig_num;
   char * sig_name;
} SIGTBL[] = {
   { SIGHUP, "HUP" },
   { SIGINT, "INT" },
   { SIGQUIT, "QUIT" },
   { SIGILL, "ILL" },
   { SIGTRAP, "TRAP" },
   { SIGIOT, "IOT" },
   { SIGABRT, "ABRT" },
#ifdef SIGEMT
   { SIGEMT, "EMT" },
#endif
   { SIGFPE, "FPE" },
   { SIGKILL, "KILL" },
   { SIGBUS, "BUS" },
   { SIGSEGV, "SEGV" },
#ifdef SIGSYS
   { SIGSYS, "SYS" },
#endif
   { SIGPIPE, "PIPE" },
   { SIGALRM, "ALRM" },
   { SIGTERM, "TERM" },
   { SIGUSR1, "USR1" },
   { SIGUSR2, "USR2" },
#ifdef SIGCLD
   { SIGCLD, "CLD" },
#endif
   { SIGCHLD, "CHLD" },
   { SIGWINCH, "WINCH" },
   { SIGURG, "URG" },
#ifdef SIGPOLL
   { SIGPOLL, "POLL" },
#endif
   { SIGIO, "IO" },
   { SIGSTOP, "STOP" },
   { SIGTSTP, "TSTP" },
   { SIGCONT, "CONT" },
   { SIGTTIN, "TTIN" },
   { SIGTTOU, "TTOU" },
   { SIGVTALRM, "VTALRM" },
   { SIGPROF, "PROF" },
   { SIGXCPU, "XCPU" },
   { SIGXFSZ, "XFSZ" },
   { 0, NULL }
};




/********************************************************/
/*							*/
/*	Table definitions -- translation		*/
/*							*/
/********************************************************/


				/* character definitions */
#define SECCONTIN	0xFE	/* continue secondary str */
#define SECCMD		0xFC	/* start secondary string */
#define SECONEMORE	0xFA	/* ignore last char of def */
#define SECIGNORE	0xFB	/* ignore secondary string */
#define SGNDFLT 	0xFD	/* default signal action */
#define ABTCMD		0xFF	/* immediate abort	 */

#define MOUSE1		0xE0
#define MOUSE2		0xE1
#define MOUSE3		0xE2


static	BYTE	transtb[512];	/* translation table	*/

static	BOOL	sigverify[SIGTBLEN];	/* verify signal ? */
static	BYTE	sigvtbl[SIGTBLEN];	/* signal result*/

#define SECSIZE 	347	/* # secondary strings	*/
				/* must be suitable for double hashing */

#define SECESIZE	10	/* length of strings	*/
#define SECWIDTH	SECESIZE+2
#define SECVALUE	SECESIZE+1

static	BYTE	sectbl[SECSIZE][SECWIDTH];
static	INT	sectblen;

static	CHAR	secstr[SECESIZE*2]; /* sec string	 */

struct	termios saved_ttsts;
struct	termios init_ttsts;



/********************************************************************************/
/*										*/
/*	Table definitions -- curses						*/
/*										*/
/********************************************************************************/

#define CK(key) 	{ #key , KEY_ ## key }
#define CFK(i)		{ "F" #i, KEY_F(i) }

struct {
   char * curs_name;
   int curs_key;
} CURSTBL[] = {
   CK(BREAK),
   CK(DOWN),
   CK(UP),
   CK(LEFT),
   CK(RIGHT),
   CK(HOME),
   CK(BACKSPACE),
   CFK(0),
   CFK(1),
   CFK(2),
   CFK(3),
   CFK(4),
   CFK(5),
   CFK(6),
   CFK(7),
   CFK(8),
   CFK(9),
   CFK(10),
   CFK(11),
   CFK(12),
   CFK(13),
   CFK(14),
   CFK(15),
   CFK(16),
   CFK(17),
   CFK(18),
   CFK(19),
   CFK(20),
   CFK(21),
   CFK(22),
   CFK(23),
   CFK(24),
   CFK(25),
   CFK(26),
   CFK(27),
   CFK(28),
   CFK(29),
   CFK(30),
   CFK(31),
   CFK(32),
   CFK(33),
   CFK(34),
   CFK(35),
   CFK(36),
   CFK(37),
   CFK(38),
   CFK(39),
   CFK(40),
   CFK(41),
   CFK(42),
   CFK(43),
   CFK(44),
   CFK(45),
   CFK(46),
   CFK(47),
   CFK(48),
   CFK(49),
   CFK(50),
   CFK(51),
   CFK(52),
   CFK(53),
   CFK(54),
   CFK(55),
   CFK(56),
   CFK(57),
   CFK(58),
   CFK(59),
   CFK(60),
   CFK(61),
   CFK(62),
   CFK(63),
   CK(DL),
   CK(IL),
   CK(DC),
   CK(IC),
   CK(EIC),
   CK(CLEAR),
   CK(EOS),
   CK(EOL),
   CK(SF),
   CK(SR),
   CK(NPAGE),
   CK(PPAGE),
   CK(STAB),
   CK(CTAB),
   CK(CATAB),
   CK(ENTER),
   CK(SRESET),
   CK(RESET),
   CK(PRINT),
   CK(LL),
   CK(A1),
   CK(A3),
   CK(B2),
   CK(C1),
   CK(C3),
   CK(BTAB),
   CK(BEG),
   CK(CANCEL),
   CK(CLOSE),
   CK(COMMAND),
   CK(COPY),
   CK(CREATE),
   CK(END),
   CK(EXIT),
   CK(FIND),
   CK(HELP),
   CK(MARK),
   CK(MESSAGE),
   CK(MOVE),
   CK(NEXT),
   CK(OPEN),
   CK(OPTIONS),
   CK(PREVIOUS),
   CK(REDO),
   CK(REFERENCE),
   CK(REFRESH),
   CK(REPLACE),
   CK(RESTART),
   CK(RESUME),
   CK(SAVE),
   CK(SBEG),
   CK(SCANCEL),
   CK(SCOMMAND),
   CK(SCOPY),
   CK(SCREATE),
   CK(SDC),
   CK(SDL),
   CK(SELECT),
   CK(SEND),
   CK(SEOL),
   CK(SEXIT),
   CK(SFIND),
   CK(SHELP),
   CK(SHOME),
   CK(SIC),
   CK(SLEFT),
   CK(SMESSAGE),
   CK(SMOVE),
   CK(SNEXT),
   CK(SOPTIONS),
   CK(SPREVIOUS),
   CK(SPRINT),
   CK(SREDO),
   CK(SREPLACE),
   CK(SRIGHT),
   CK(SRSUME),
   CK(SSAVE),
   CK(SSUSPEND),
   CK(SUNDO),
   CK(SUSPEND),
   CK(UNDO),
   CK(MOUSE),
   { "MOUSE1", MOUSE1 },
   { "MOUSE2", MOUSE2 },
   { "MOUSE3", MOUSE3 },
   { NULL, 0 }
};



/********************************************************/
/*							*/
/*	INinit -- initialize input processing		*/
/*							*/
/********************************************************/


void INinit()
{
   tcgetattr(INFID,&init_ttsts);

   setupterminal();

   in_mouse = FALSE;
}




/********************************************************/
/*							*/
/*	INsetup -- setup for input processing		*/
/*							*/
/********************************************************/


int INsetup(term,flgs)
   STRING term;
   STRING flgs;
{
   register INT vl,sts,i;

   envok = FALSE;
   sts = 0;
   if (*term == 0) term = NULL;

   if (!termdefd || term != NULL)
    { definput(term);
      for (i = SIGFIRST; i < SIGLAST; ++i) setsig(i);
    };

   INecho = INlcnt = INcbreak = INraw = INnline = 0;

   if (flgs != 0)
      while (*flgs != 0)
       { vl = 1;
	 if (*flgs == '-') { vl = 2; ++flgs; };
	 switch (*flgs++)
	  { case 'e': INecho = vl; break;
	    case 'l': INlcnt = vl; break;
	    case 'c': INcbreak = vl; break;
	    case 'r': INraw = vl; break;
	    case 'n': INnline = vl; break;
	    default : sts = 1; break;
	  };
       };

   /* setupterminal(); */

   if (INecho == 1) echo();
   else noecho();
   if (INraw == 1) raw();
   else if (INcbreak == 1) cbreak();
   else { noraw(); nocbreak(); }

   sigchr = 0;
   unputch = 0;
   unputch1 = 0;
   unputch2 = 0;

   termdefd = TRUE;

   return sts;
}





/********************************************************/
/*							*/
/*	INchar -- get next logical input character	*/
/*							*/
/********************************************************/


int INchar()
{
   register INT ch,och,secfg,secct,onemorefg;			/* ddj */
   CHAR secbuf[SECESIZE+1];

   if ((ch = unputch) != 0) {
      unputch = unputch1;
      unputch1 = unputch2;
      unputch2 = 0;
      return ch & 0xFF;
    };

   secfg = FALSE;
   onemorefg = FALSE;						/* ddj */
   do {
      if (!sigfg && setjmp(env) != 0) ch = -1;
      else if ((ch = sigchr) != 0) {
	 sigchr = 0;
	 ch &= 0xFF;
	 if (ch == CM_KNOCK) return KNOCKDOWN(ch,nextchar());
	 else if (ch & 0x80) return ch;
       }
      else {
	 envok = TRUE;
	 ch = -1;
	 ch = nextchar();
       }

      envok = FALSE;
      och = ch;
      if (ch < 0) {
	 if (intrfg) { ch = 0; continue; }
	 else ch = '\04';
       }
      intrfg = FALSE;

      if (!secfg) {
	 ch = transtb[ch];
	 if (ch == SECCMD) {
	    ch = och;
	    secfg = TRUE;
	    secct = 0;
	  }
	 if (ch == ABTCMD) INabort();
	 if (ch == CM_INTR) kill(getpid(),SIGINT);
       }

      if (secfg && ch == 0) {
	 ILLCHR();
	 secfg = FALSE;
	 onemorefg = FALSE;
       }
      else if (secfg) {
	 secbuf[secct++] = ch;
	 ch = seclookup(secbuf,secct);
	 if (ch == SECONEMORE) onemorefg = TRUE;		/* ddj */
	 if (ch == SECCONTIN || ch == SECONEMORE) ch = 0;	/* ddj */
	 else if (ch == SECIGNORE) {
	    ch = 0;
	    secfg = FALSE;
	    onemorefg = FALSE;
	  }    /* ddj */
	 else secfg = FALSE;
       }

      if (!sigfg && setjmp(env) != 0) ch = 0;
      else {
	 envok = TRUE;
	 if (ch == CM_KNOCK) KNOCKDOWN(ch,nextchar());
       }
      envok = FALSE;
    }
     while (ch == 0);

   if (onemorefg && (sigfg || setjmp(env) == 0)) {
      envok = TRUE;
      nextchar();	/* ddj */
    };
   envok = FALSE;

   return ch;
}





/********************************************************/
/*							*/
/*	INcmdchar -- get command character input	*/
/*							*/
/********************************************************/


int INcmdchar()
{
   register INT ch;

   if ((ch = unputch) != 0) {
      unputch = unputch1;
      unputch1 = unputch2;
      unputch2 = 0;
      return ch & 0xFF;
    };

   if (PVAL(douse) && (ch = DOget()) != 0) return ch;

   ch = INchar();

   if (PVAL(dosave)) DOsave(ch);

   return ch;
}






/********************************************************/
/*							*/
/*	INline -- input a line from user		*/
/*							*/
/********************************************************/


void INline(buf,sz,tbl,slnt)
   STRING buf;			/* input buffer 	*/
   INT sz;			/* size of buffer	*/
   TEXT tbl[];			/* editing info table	*/
   INT slnt;			/* silent flag		*/
{
   register INT ln,ch,i,och;
   register TEXT *bp,*lp;
   register INT bs;
   TEXT last[LINESIZE];

   ln = 0;

   bs = transtb['\b'];
   if (tbl != NULL) {
      strcpy((char *) last,buf);
      for (i = 0; tbl[i] != 0; i += 2)
	 if (tbl[i] < CM_FIRST) tbl[i] = transtb[tbl[i]];
    };

   bp = (TEXT *) buf;

   for ( ; ; )
    { och = ch = INcmdchar();
      if (ch == bs) ch = ICH_BS;
	 else if (tbl != NULL)
	    for (i = 0; tbl[i] != 0; i += 2)
	       if (ch == tbl[i])
		{ ch = tbl[i+1];
		  break;
		};
      if (ch == ICH_IGNORE || ch == 0) continue;

      switch(ch)
       { default	:
		if (ch >= CM_FIRST) goto exit;
		och = ch;
		break;

	 case ICH_ENTER :
		if (ln == 0 || *(bp-1) == ICH_ENTER)
		 { och = '>'; break; }
		ch = ICH_DELIM;

	 case ICH_DELIM :
		och = '$';
		break;

	 case ICH_WARN	:
		if (ln == 0) {ch = och; goto exit; };
		if (!slnt) TRbells(1);
		ch = och = 0;
		break;

	 case ICH_END	:
		goto exit;

	 case ICH_BS	:
		if (ln == 0) { if (!slnt) TRbells(1); }
		 else { --bp; if (!slnt) TRtype(CHR_BS,FALSE); --ln; };
		ch = och = 0;
		break;

	 case ICH_LAST	:
		lp = last;
		while (*lp != 0 && ln < sz)
		 { *bp++ = *lp;
		   if (!slnt) TRtype(*lp++);
		   ++ln;
		 };
		*bp = 0;
		ch = och = 0;
		break;
       };

      if (ln >= sz-1 && (ch != 0 || och != 0)) { if (!slnt) TRbells(1); }
	 else
	  { if (ch != 0) {*bp++ = ch; *bp = 0; };
	    if (och != 0) {if (!slnt) TRtype(och,FALSE); ++ln; };
	  };
    };

exit:
   *bp++ = ch;
   *bp = 0;

   return;
}






/********************************************************/
/*							*/
/*	INunput -- unput one character			*/
/*							*/
/********************************************************/


void INunput(ch)
   INT ch;
{
   unputch2 = unputch1;
   unputch1 = unputch;
   unputch = ch;
}





/********************************************************/
/*							*/
/*	INdone -- finish input processing		*/
/*							*/
/********************************************************/


void INdone()
{
   register INT i;
   struct sigaction hdlr;
   struct termios ttsts;

   if (!termdefd) return;

   for (i = SIGFIRST; i < SIGLAST; ++i)
      signal(i,SIG_DFL);

/* ioctl(INFID,TCGETS,&ttsts);	*/
   tcgetattr(INFID,&ttsts);
   for (i = 0; i < NCCS; ++i) ttsts.c_cc[i] = saved_ttsts.c_cc[i];
   tcsetattr(INFID,TCSANOW,&ttsts);
/* ioctl(INFID,TCSETS,&ttsts);	*/

#ifdef SIGWINCH
   hdlr.sa_handler = (void (*)()) INsignal;
   sigemptyset(&hdlr.sa_mask);
   hdlr.sa_flags = 0;
   sigaction(SIGWINCH,&hdlr,NULL);		/* signal(SIGWINCH,INsignal) */
#endif
}





/********************************************************/
/*							*/
/*	INscan -- translate input character		*/
/*							*/
/********************************************************/


int INscan(chp)
   STRING *chp; 		/* character string	*/
{
   return getchr(chp);
}





/********************************************************/
/*							*/
/*	INsignal -- handle signal			*/
/*							*/
/********************************************************/


static void
INsignal(id)
   INT id;
{
   register INT alm,rfg;
   register BOOL okenv;

   okenv = envok;
   signal(id,SIG_IGN);
   alm = alarm(0);
   rfg = 0;
   sigfg = 1;

   if (!sigverify[id] || INVERIFY(id)) {
      if (sigvtbl[id] == ABTCMD) INabort();
      sigchr = sigvtbl[id];
      rfg = 1;
    }

   setsig(id);

   sigfg = 0;
   intrfg = TRUE;

   envok = FALSE;
   if (rfg && okenv) longjmp(env,1);
}





/********************************************************/
/*							*/
/*	INabort -- cleanly abort program		*/
/*							*/
/********************************************************/


static void
INabort()
{
   INdone();
   INCLEAN();
   exit(1);
}





/********************************************************/
/*							*/
/*	INabort -- cleanly abort program		*/
/*							*/
/********************************************************/


static void
setupterminal()
{
   struct termios ttsts;

   tcgetattr(INFID,&ttsts);
   saved_ttsts = ttsts;
   saved_ttsts.c_cc[VEOF] = init_ttsts.c_cc[VEOF];	/* these two are affected by */
   saved_ttsts.c_cc[VEOL] = init_ttsts.c_cc[VEOL];	/* non-canonical processing  */

   if (INcbreak == 1 || INraw == 1) {
      ttsts.c_cc[VINTR] = '\177';
      ttsts.c_cc[VQUIT] = '\034';
      ttsts.c_cc[VSTOP] = 0;
      ttsts.c_cc[VSUSP] = 0;
#ifdef VDSUSP
      ttsts.c_cc[VDSUSP] = 0;
#endif
#ifdef VSWTCH
      ttsts.c_cc[VSWTCH] = 0;
#endif
#ifdef VSWTC
      ttsts.c_cc[VSWTC] = 0;
#endif
      ttsts.c_cc[VERASE] = 0;
      ttsts.c_cc[VKILL] = 0;
      ttsts.c_cc[VEOF] = 0;
      ttsts.c_cc[VEOL] = 0;
      ttsts.c_cc[VEOL2] = 0;
      ttsts.c_cc[VSTART] = 0;
      ttsts.c_cc[VREPRINT] = 0;
      ttsts.c_cc[VDISCARD] = 0;
      ttsts.c_cc[VWERASE] = 0;
      ttsts.c_cc[VLNEXT] = 0;
#ifdef VTIME
      ttsts.c_cc[VTIME] = 0;
#endif
#ifdef VMIN
      ttsts.c_cc[VMIN] = 0;
#endif
    };
/* ioctl(INFID,TCSETS,&ttsts);	*/
   tcsetattr(INFID,TCSANOW,&ttsts);
}



/********************************************************/
/*							*/
/*	setsig -- set signal based on table entry	*/
/*							*/
/********************************************************/


static void
setsig(i)
   INT i;
{
   struct sigaction hdlr;

   sigemptyset(&hdlr.sa_mask);
   hdlr.sa_flags = 0;

   if (!sigverify[i] && sigvtbl[i] == SGNDFLT) hdlr.sa_handler = SIG_DFL;
   else if (!sigverify[i] && sigvtbl[i] == 0) hdlr.sa_handler = SIG_IGN;
   else hdlr.sa_handler = (void (*)()) INsignal;

   sigaction(i,&hdlr,NULL);
}





/********************************************************/
/*							*/
/*	seclookup -- lookup secondary string		*/
/*							*/
/********************************************************/


static int
seclookup(str,len)
   STRING str;
   INT len;
{
   register LONG i,j,h;

   str[len] = 0;
   h = sechash(str);
   i = h % SECSIZE;
   if (sectbl[i][0] != 0 && strcmp((char *) sectbl[i],str) != 0)
    { j = 1 + h%(SECSIZE-2);
      do
	 if ((i -= j) < 0) i += SECSIZE;
       while (sectbl[i][0] != 0 && strcmp((char *) sectbl[i],str) != 0);
    };

   if (sectbl[i][0] != 0) return sectbl[i][SECVALUE];

   ILLCHR();

   return SECIGNORE;
}





/********************************************************/
/*							*/
/*	definput -- define input processing tables	*/
/*							*/
/********************************************************/


static void
definput(dflt)
   STRING dflt;
{
   char buf[BUFLEN];
   register char * bufp;
   register int secondfg,onemorefg;			/* ddj */

   secondfg = FALSE;
   onemorefg = FALSE;					/* ddj */
   PSET(doset,FALSE);

   inittbls();
   findentry(dflt,buf);
   bufp = strchr(buf,':');
   if (bufp == NULL) badinput("No initial colon",buf);
   bufp++;

   while (*bufp != 0) {
      switch(*bufp) {
	 case '$' :
	    bufp = prcsignal(++bufp);
	    break;
	 case '%' :
	    onemorefg = TRUE;			/* ddj */
	    bufp = prcsecond(++bufp);
	    secondfg = TRUE;
	    break;
	 case '&' :
	    onemorefg = FALSE;
	    bufp = prcsecond(++bufp);
	    secondfg = TRUE;
	    break;
	 case '@' :
	    bufp = prccurses(++bufp);
	    break;
	 case ':' :
	 case ' ' :
	 case '\n':
	 case '\t':
	    ++bufp;
	    break;
	 default  :
	    if (secondfg) bufp = prcsecdef(bufp, onemorefg);
	    else bufp = prcdef(bufp);
	    break;
       }
    }

   finitbls();
}





/********************************************************/
/*							*/
/*	inittbls -- initialize tables			*/
/*							*/
/********************************************************/


static void
inittbls()
{
   register INT i;

   for (i = 0; i < 127; ++i) transtb[i] = i;
   transtb[127] = CM_BS;
   for (i = 128; i < 512; ++i) transtb[i] = 0;

   for (i = 0; i < SIGTBLEN; ++i) {
      sigverify[i] = FALSE;
      sigvtbl[i] = SGNDFLT;
    }

#ifdef SIGWINCH
   sigvtbl[SIGWINCH] = CM_RESET;
#endif

   sectblen = 0;
   for (i = 0; i < SECSIZE; ++i) sectbl[i][0] = 0;
}





/********************************************************/
/*							*/
/*	findentry -- find table entry for terminal	*/
/*							*/
/********************************************************/


static void
findentry(dflt,buf)
   STRING dflt,buf;
{
   register STRING dstr;
   register FILE *fid;
   register INT newfg,ln;
   CHAR buffer[BUFSIZ];

   if (dflt == NULL) dflt = getenv("BINP");
   if (dflt == NULL) dflt = getenv("TERM");
   if (dflt == NULL) dflt = DFLTTERM;

   dstr = getenv("BINPDEF");
   if (dstr != NULL && dstr[0] != '/' && term_name(dflt,dstr))
    { strcpy(buf,dstr);
      if ((dstr = getenv("BINPDEF2")) != NULL) strcat(buf,dstr);
      return;
    };

   fid = NULL;
   if (dstr != NULL && dstr[0] == '/') fid = fopen(dstr,"r");
   if (fid == NULL) fid = fopen(DEFFILE,"r");
   newfg = TRUE;
   setbuf(fid,buffer);

   while (fgets(buf,BUFLEN,fid) != NULL) {
      ln = strlen(buf);
      if (newfg) {
	 if (term_name(dflt,buf)) break;
	 else if (strchr(buf,':') != NULL) newfg = FALSE;
       }
      if (!newfg) {
	 if (ln < 2 || buf[ln-2] != '\\') newfg = TRUE;
       }
      buf[0] = 0;
    }

   if (buf[0] == 0) {
      DSerror("-dUnknown terminal: using default key definitions");
      rewind(fid);
      dflt = DFLTTERM;
      newfg = TRUE;
      setbuf(fid,buffer);

      while (fgets(buf,BUFLEN,fid) != NULL) {
	 ln = strlen(buf);
	 if (newfg) {
	    if (term_name(dflt,buf)) break;
	    else if (strchr(buf,':') != NULL) newfg = FALSE;
	  }
	 if (!newfg) {
	    if (ln < 2 || buf[ln-2] != '\\') newfg = TRUE;
	  }
	 buf[0] = 0;
       }

      if (buf[0] == 0) {
	 fprintf(stderr,"b: Can't find input entry for terminal\n");
	 exit(1);
       }
    }

   while(buf[ln-2] == '\\') {
      ln -= 2;
      buf[ln] = 0;
      if (fgets(&buf[ln],BUFLEN-ln,fid) == NULL) break;
      if (buf[ln] == '#') strcpy(&buf[ln],"\\\n");
      else ln = strlen(buf);
    }

   buf[ln-1] = 0;

   fclose(fid);
   setbuf(fid,NULL);
}




/********************************************************/
/*							*/
/*	prcdef -- process single character definition	*/
/*							*/
/********************************************************/


static STRING
prcdef(pt)
   STRING pt;
{
   CHAR frm[128],to[128];
   register INT i,lst,nxt,npr;

   npr = 0;
   lst = -1;

   do {
      nxt = getchr(&pt);
      if (lst != -1) {
	 frm[npr] = lst;
	 to[npr++] = nxt;
	 lst = -1;
       }
      else if (*pt == '-' && *(pt+1) != '=') {
	 lst = nxt;
	 ++pt;
       }
      else {
	 frm[npr] = to[npr] = nxt;
	 ++npr;
       }
    }
   while (*pt != '=' && *pt != 0);

   if (*pt++ == 0) badinput("Bad def",NULL);

   lst = getcdef(&pt,":");

   for (i = 0; i < npr; ++i) {
      for (nxt = frm[i]; nxt <= to[i]; ++nxt) {
	 transtb[nxt] = lst;
       }
    }

   return pt;
}





/********************************************************/
/*							*/
/*	prcsignal -- process signal entry		*/
/*							*/
/********************************************************/


static STRING
prcsignal(bp)
   STRING bp;
{
   register LONG sgn;
   CHAR buf[16];
   STRING p;
   INT i;

   p = buf;
   while (isalnum(*bp)) *p++ = *bp++;
   *p = 0;

   sgn = -1;
   for (i = 0; SIGTBL[i].sig_name != NULL; ++i) {
      if (strcmp(buf,SIGTBL[i].sig_name) == 0) {
	 sgn = SIGTBL[i].sig_num;
	 break;
       };
    };

   if (sgn < 0) badinput("Bad signal",bp);

   if (*bp++ != '=') badinput("No equal",bp);

   if (*bp == '?') {
      ++bp;
      sigverify[sgn] = TRUE;
    };
   sigvtbl[sgn] = getcdef(&bp,":");

   return bp;
}





/********************************************************/
/*							*/
/*	prcsecond -- process start of secondary table	*/
/*							*/
/********************************************************/


static STRING
prcsecond(bp)
   STRING bp;
{
   secpfxln = 0;

   while (*bp != ':' && *bp !=  0) {
      secstr[secpfxln++] = getchr(&bp);
      secstr[secpfxln] = 0;
      secenter(secstr,secpfxln,SECCONTIN);
    };

   return bp;
}




/********************************************************/
/*							*/
/*	prcsecdef -- process secondary definition	*/
/*							*/
/********************************************************/


static STRING
prcsecdef(bp,onemore)
   STRING bp;
   INT onemore;
{
   register INT ln,ch,i;

   ln = secpfxln;
   for (i = ln; i < SECESIZE; ++i) secstr[i] = 0;

   while (*bp != '=' && *bp != 0)
    { if (onemore) secenter(secstr,ln,SECONEMORE);		/* ddj */
	 else secenter(secstr,ln,SECCONTIN);			/* ddj */
      secstr[ln++] = getchr(&bp);
    };

   if (*bp++ == 0) badinput("Bad sec",NULL);

   ch = getcdef(&bp,":");
   secenter(secstr,ln,ch);

   return bp;
}





/********************************************************/
/*							*/
/*	prccurses -- process curses definition		*/
/*							*/
/********************************************************/


static STRING
prccurses(bp)
   STRING bp;
{
   register INT key,lst;
   CHAR buf[16];
   STRING p;
   INT i;

   if (*bp == '@') {                            /* @@ -> use curses for input */
      use_curses = TRUE;
      ++bp;
      while (*bp != ':' && *bp != 0) ++bp;
      return bp;
    }

   p = buf;
   while (isalnum(*bp)) *p++ = *bp++;
   *p = 0;

   key = -1;
   for (i = 0; CURSTBL[i].curs_name != NULL; ++i) {
      if (strcmp(buf,CURSTBL[i].curs_name) == 0) {
	 key = CURSTBL[i].curs_key;
	 break;
       }
    }

   if (key < 0) badinput("Bad curses name",bp);

   if (key == KEY_MOUSE) {
#ifdef NCURSES_MOUSE_VERSION
      mousemask(BUTTON1_CLICKED|BUTTON2_CLICKED|BUTTON3_CLICKED|
		   BUTTON1_PRESSED|BUTTON2_PRESSED|BUTTON3_PRESSED,
		NULL);
#else
      mouse_set(BUTTON1_CLICKED|BUTTON2_CLICKED|BUTTON3_CLICKED);
#endif
    }
   if (*bp++ != '=') badinput("No equal",bp);

   lst = getcdef(&bp,":");
   transtb[key] = lst;

   return bp;
}





/********************************************************/
/*							*/
/*	finitbls -- clean up table definitions		*/
/*							*/
/********************************************************/


static void
finitbls()
{
	;		/* nothing to do for now	*/
}





/********************************************************/
/*							*/
/*	getcdef -- get definition for translation	*/
/*							*/
/********************************************************/


static int
getcdef(ptp,trms)
   STRING *ptp;
   STRING  trms;
{
   register INT ch;
   STRING pt;

   pt = *ptp;

   switch(ch = *pt++) {
      case '\\':
      case '^':
	 --pt;
	 ch = getchr(&pt);
	 break;
      case '>':
	 ch = cmdtrans(&pt);
	 break;
      case '&':
	 ch = SECCMD;
	 break; 												P
      case '%':
	 ch = SECCMD;
	 break; 			    /* ddj */
      case '$':
	 ch = ABTCMD;
	 break;
      default :
	 if (strchr(trms,ch)) { --pt; ch = 0; };
	 break;
    }

   if (strchr(trms,*pt) == 0 && *pt != 0) badinput("bad def",pt);

   if (ch == CM_DOSET) PSET(doset,TRUE);

   *ptp = pt;

   return ch;
}





/********************************************************/
/*							*/
/*	cmdtrans -- translate command name		*/
/*							*/
/********************************************************/


static int
cmdtrans(ptp)
   STRING *ptp;
{
   register LONG idx;

   idx = srchtbl(ptp,CMDTBL,CMDTBLEN);

   return CMDVTBL[idx];
}





/********************************************************/
/*							*/
/*	getchr -- get single character			*/
/*							*/
/********************************************************/


static int
getchr(ptp)
   STRING *ptp;
{
   register STRING pt;
   register CHAR ch;
   STRING np;
   register INT j,k;
   static CHAR stba[] = "nbtrfE";
   static CHAR stbb[] = "\n\b\t\r\f\33";

   pt = *ptp;

   if ((ch = *pt++) == '\\')
    { if (isdigit(*pt))
       { ch = k = 0;
	 while (++k < 4 && isdigit(j = *pt++))
	    ch = (ch << 3) + (j - '0');
	 if (k < 4) --pt;
       }
      else if ((np = strchr(stba,*pt)) != 0)
       { ++pt;
	 ch = stbb[np-stba];
       }
      else ch = *pt++;
    }
   else if (ch == '^') ch = *pt++ & 0x1F;

   *ptp = pt;

   return ch;
}





/********************************************************/
/*							*/
/*	srchtbl -- do a search on name table		*/
/*							*/
/********************************************************/


static LONG
srchtbl(stp,tbl,tblen)
   STRING *stp;
   LONG tbl[];
   INT tblen;
{
   register STRING pt;
   register LONG vl, i;

   pt = *stp;
   vl = 0;
   i = 0;
   while (isalnum(*pt)) {
      if (i++ < 4) vl = (vl<<8)+ *pt++;
	 else ++pt;
    }

   for (i = 0; i < tblen; ++i) {
      if (vl == tbl[i]) break;
    }
   if (i >= tblen) badinput("Bad cmd",*stp);

   *stp = pt;

   return i;
}






/********************************************************/
/*							*/
/*	secenter -- put entry in secondary table	*/
/*							*/
/********************************************************/


static void
secenter(str,ln,vl)
   STRING str;
   INT ln,vl;
{
   register LONG i,j,h;

   if (ln > SECESIZE) badinput("esc seq too long",NULL);

   h = sechash(str);
   i = h % SECSIZE;
   if (sectbl[i][0] != 0 && strcmp((char *) sectbl[i],str) != 0)
    { j = 1 + h%(SECSIZE-2);
      do
	 if ((i -= j) < 0) i += SECSIZE;
       while (sectbl[i][0] != 0 && strcmp((char *) sectbl[i],str) != 0);
    };

   if (sectbl[i][0] != 0) {
      if (sectbl[i][SECVALUE] != vl) badinput("dup esc",NULL);
	 else return;
    }

   for (j = 0; j < SECWIDTH; ++j)
      sectbl[i][j] = j < ln ? str[j] : 0;

   sectbl[i][SECVALUE] = vl;

   if (++sectblen >= SECSIZE-1) badinput("too many esc",NULL);
}





/********************************************************/
/*							*/
/*	sechash -- hash a secondary string		*/
/*							*/
/********************************************************/


static LONG
sechash(str)
   STRING str;
{
   register LONG wd,sh;

   wd = 0;
   sh = 24;
   while (*str != 0)
    { wd ^= (*str++)<<sh;
      if ((sh -= 8) < 0) sh = 24;
    };

   wd &= 0x3FFFFFFF;

   return wd;
}






/********************************************************/
/*							*/
/*	term_name -- check for terminal name match	*/
/*							*/
/********************************************************/


static int
term_name(name,str)
   STRING name,str;
{
   register STRING pt,ept;
   register INT ln;

   ln = strlen(name);
   ept = strchr(str,':');
   if (ept == NULL) return FALSE;

   for (pt = str-1; pt++ != NULL && pt < ept;
	   pt = strchr(pt,'|'))
      if (strncmp(name,pt,ln) == 0 &&
	     ispunct(pt[ln])) return TRUE;

   return FALSE;
}





/********************************************************/
/*							*/
/*	badinput -- report bad input and abort		*/
/*							*/
/********************************************************/


static void
badinput(msg,ptr)
   STRING msg;
   STRING ptr;
{
   printf("\nError in input definitions file: %s\n",msg);

   if (ptr != NULL)
      printf("Before '%.40s'\n",ptr);

   exit(1);
}





/************************************************************************/
/*									*/
/*	nextchar -- read character from tty				*/
/*									*/
/************************************************************************/


static INT
nextchar()
{
   char buf[10];
   INT sts,chr;

retry:
   if (use_curses) {
      chr = getch();
      if (chr == ERR) return -1;
      if (chr == KEY_MOUSE) {
#ifdef NCURSES_MOUSE_VERSION
	 MEVENT evt;
	 if (getmouse(&evt) == OK) {
	    if (evt.bstate == REPORT_MOUSE_POSITION && in_mouse == FALSE) {
	       chr = MOUSE1;
	       in_mouse = TRUE;
	     }
	    else in_mouse = FALSE;
	    if ((evt.bstate & (BUTTON1_CLICKED|BUTTON1_PRESSED)) != 0) chr = MOUSE1;
	    else if ((evt.bstate & (BUTTON2_CLICKED|BUTTON2_PRESSED)) != 0) chr = MOUSE2;
	    else if ((evt.bstate & (BUTTON3_CLICKED|BUTTON3_PRESSED)) != 0) chr = MOUSE3;
	    if (chr != KEY_MOUSE) {
	       INunput(' ' + evt.y+1);
	       INunput(' ' + evt.x+1);
	     }
	    else goto retry;
	  }
#else
	 long evt = getmouse();
	 if ((evt & (BUTTON1_CLICKED|BUTTON1_PRESSED)) != 0) chr = MOUSE1;
	 else if ((evt & (BUTTON2_CLICKED|BUTTON2_PRESSED)) != 0) chr = MOUSE2;
	 else if ((evt & (BUTTON3_CLICKED|BUTTON3_PRESSED)) != 0) chr = MOUSE3;
	 if (chr != KEY_MOUSE) {
	    INunput(' ' + MOUSE_Y_POS);
	    INunput(' ' + MOUSE_X_POS);
	  }
	 else goto retry;
#endif
       }
      else in_mouse = FALSE;
    }
   else {
      sts = read(INFID,buf,1);
      if (sts != 1) return -1;
      chr = buf[0] & 0x7f;
    }

   return chr;
}





/* end of input.c */
