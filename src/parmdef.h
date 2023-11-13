/********************************************************/
/*							*/
/*		PARMDEF.H  (from parmdef.hm)		*/
/*							*/
/*	This file contains the definitions for using	*/
/*   parameters within the parameter processing module. */
/*   Functions that use parameters externally should	*/
/*   include parms.h rather than this file.		*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University					*/



#define PMXSIZE 1024		/* maximum string space / parm	*/


#define PFILECT 45 	/* number of file parameters	*/
#define PCTRLCT 9 	/* number of control parameters */
#define PFILEST 0+FNSIZE+16+FNSIZE+16+FNSIZE+16	/* total file string size	*/
#define PCTRLST 0+FNSIZE+FNSIZE+512+512+512+FNSIZE+512	/* total control string size	*/



/* end of parameters */











/********************************************************/
/*							*/
/*	Parameter definitions				*/
/*							*/
/********************************************************/


				/* parameter routines	*/

static	int	PRbuffer ();


static	int	PRflname ();


static	int	PRscrchk ();


static	int	PRwindow ();


static	int	PRlanguage ();


static	int	PRdisplay ();


static	int	PRdirchk ();


static	int	PRchange ();


static	int	PRmesg ();





				/* string buffers	*/

static TEXT PSbufinfile[FNSIZE] = "";

static TEXT PSbufoutfile[FNSIZE] = "";

static TEXT PSuseshare[FNSIZE] = "";

static TEXT PSfilename[FNSIZE] = "";

static TEXT PScurlanguage[16] = "";

static TEXT PSPfilename[FNSIZE] = "/cs/lib/b.help";

static TEXT PSPlanguage[16] = "";

static TEXT PSQfilename[FNSIZE] = "/cs/lib/b.help";

static TEXT PSQlanguage[16] = "";

static TEXT PSXfilename[FNSIZE] = "/cs/lib/b.help";

static TEXT PSXlanguage[16] = "";

static TEXT PSterminal[16] = "TERM";

static TEXT PSinterminal[16] = "INTERM";

static TEXT PSfiletail[FNSIZE] = "";

static TEXT PSdirectory[FNSIZE] = "";

static TEXT PStdirect[24] = "/tmp";

static TEXT PSlanguage[16] = "";

static TEXT PSnextfile[FNSIZE] = "/cs/lib/b.help";

static TEXT PSafterfile[1024] = "";

static TEXT PSeditor[FNSIZE] = "/cs/bin/bb";

static TEXT PSwinmark[2] = "-";

static TEXT PShome[256] = "HOME";

static TEXT PSshell[64] = "SHELL";

static TEXT PSmail[FNSIZE] = "MAILBOX";

static TEXT PSfind[FNSIZE] = "";

static TEXT PSAsearch[512] = "";

static TEXT PSArsearch[512] = "";

static TEXT PSAfsearch[512] = "";

static TEXT PSAexit[FNSIZE] = "";

static TEXT PSAsystem[FNSIZE] = "";

static TEXT PSAcommand[512] = "";

static TEXT PSAbfsave[512] = "";

static TEXT PSAfile[FNSIZE] = "";

static TEXT PSAshow[FNSIZE] = "";

static TEXT PSL0[FNSIZE] = "";

static TEXT PSL1[FNSIZE] = "";

static TEXT PSL2[FNSIZE] = "";

static TEXT PSL3[64] = "";





BPARM	PARMS[] = {		/* parameters		*/


   { PMTYFLAG,"copy",3,PMFreset,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"savetemp",5,PMFctrl,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYSTR,"bufinfile",4,PMFctrl,NULL,(PARMVAL) PSbufinfile,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYSTR,"bufoutfile",4,PMFctrl,NULL,(PARMVAL) PSbufoutfile,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYFLAG,"share",2,PMFndisp,PRbuffer,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYSTR,"useshare",5,PMFndisp,NULL,(PARMVAL) PSuseshare,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYFLAG,"huge",2,PMFctrl,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYSTR,"filename",4,PMFfprop,PRflname,(PARMVAL) PSfilename,(PARMVAL)"",(PARMVAL)"/cs/lib/b.help",
	 -1,FNSIZE,0 },

   { PMTYFLAG,"curcrypt",5,PMFndisp|PMFfprop|PMFfldflt|PMFfdset,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_crypt,0,0 },

   { PMTYFLAG,"curstream",5,PMFndisp|PMFfprop|PMFfdset|PMFfldflt,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_stream,0,0 },

   { PMTYFLAG,"curtabin",4,PMFndisp|PMFfdset|PMFreset|PMFfprop|PMFfldflt,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_tabin,0,0 },

   { PMTYFLAG,"curtabout",2,PMFndisp|PMFfdset|PMFfprop|PMFfldflt,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_tabout,0,0 },

   { PMTYFLAG,"curcrlf",4,PMFndisp|PMFfdset|PMFreset|PMFfprop|PMFfldflt,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_crlf,0,0 },

   { PMTYFLAG,"curindent",5,PMFfprop|PMFfdset|PMFndisp,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_indent,0,0 },

   { PMTYNUM,"curline",5,PMFfdset|PMFfprop|PMFninit|PMFfldflt|PMFnhist,PRscrchk,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_line,0,1000000000 },

   { PMTYNUM,"curchar",5,PMFfdset|PMFfprop|PMFninit|PMFfldflt|PMFnhist,PRscrchk,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_character,0,LINESIZE },

   { PMTYNUM,"curscreen",5,PMFfdset|PMFdisp|PMFfprop|PMFninit|PMFfldflt,PRscrchk,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_screen,0,1000000000 },

   { PMTYNUM,"curmargin",5,PMFfdset|PMFdisp|PMFfprop|PMFninit|PMFfldflt,PRscrchk,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_margin,0,LINESIZE },

   { PMTYNUM,"curwindow",6,PMFfprop|PMFninit|PMFdisp,PRwindow,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,2 },

   { PMTYFLAG,"curreadonly",5,PMFfprop|PMFndisp|PMFfldflt|PMFfdset,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_readonly,0,0 },

   { PMTYSTR,"curlanguage",5,PMFfprop|PMFndisp|PMFfldflt|PMFfdset,PRlanguage,(PARMVAL) PScurlanguage,(PARMVAL)"",(PARMVAL)"",
	 P_language,16,0 },

   { PMTYNUM,"fileid",6,PMFfprop,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,32767 },

   { PMTYFLAG,"curcopy",7,PMFfdset|PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_copy,0,0 },

   { PMTYSTR,"Pfilename",3,PMFndisp|PMFfprop|PMFprev,NULL,(PARMVAL) PSPfilename,(PARMVAL)"/cs/lib/b.help",(PARMVAL)"",
	 P_filename,FNSIZE,0 },

   { PMTYFLAG,"Pcrypt",3,PMFndisp|PMFfprop|PMFprev|PMFfldflt,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curcrypt,0,0 },

   { PMTYFLAG,"Pstream",3,PMFndisp|PMFfprop|PMFprev,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curstream,0,0 },

   { PMTYFLAG,"Ptabin",5,PMFndisp|PMFfprop|PMFprev,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curtabin,0,0 },

   { PMTYFLAG,"Ptabout",3,PMFndisp|PMFfprop|PMFprev,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_curtabout,0,0 },

   { PMTYFLAG,"Pcrlf",5,PMFndisp|PMFfprop|PMFprev,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_curcrlf,0,0 },

   { PMTYFLAG,"Pindent",3,PMFndisp|PMFfprop|PMFprev,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_curindent,0,0 },

   { PMTYNUM,"Pline",3,PMFndisp|PMFfprop|PMFprev|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curline,0,16777215 },

   { PMTYNUM,"Pchar",3,PMFndisp|PMFfprop|PMFprev|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curchar,0,LINESIZE },

   { PMTYNUM,"Pscreen",3,PMFndisp|PMFfprop|PMFprev|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curscreen,0,16777215 },

   { PMTYNUM,"Pmargin",3,PMFndisp|PMFfprop|PMFprev|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curmargin,0,LINESIZE },

   { PMTYNUM,"Pwindow",3,PMFndisp|PMFfprop|PMFprev|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curwindow,0,2 },

   { PMTYFLAG,"Preadonly",2,PMFndisp|PMFfprop|PMFprev,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curreadonly,0,0 },

   { PMTYSTR,"Planguage",3,PMFndisp|PMFfprop|PMFprev,NULL,(PARMVAL) PSPlanguage,(PARMVAL)"",(PARMVAL)"",
	 P_curlanguage,16,0 },

   { PMTYNUM,"Pfileid",5,PMFndisp|PMFfprop|PMFprev,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_fileid,0,32767 },

   { PMTYSTR,"Qfilename",3,PMFndisp|PMFfprop|PMFaltf,NULL,(PARMVAL) PSQfilename,(PARMVAL)"/cs/lib/b.help",(PARMVAL)"",
	 P_filename,FNSIZE,0 },

   { PMTYFLAG,"Qcrypt",3,PMFndisp|PMFfprop|PMFaltf|PMFfldflt,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curcrypt,0,0 },

   { PMTYFLAG,"Qstream",3,PMFndisp|PMFfprop|PMFaltf,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curstream,0,0 },

   { PMTYFLAG,"Qtabin",5,PMFndisp|PMFfprop|PMFaltf,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curtabin,0,0 },

   { PMTYFLAG,"Qtabout",3,PMFndisp|PMFfprop|PMFaltf,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_curtabout,0,0 },

   { PMTYFLAG,"Qcrlf",5,PMFndisp|PMFfprop|PMFaltf,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_curcrlf,0,0 },

   { PMTYFLAG,"Qindent",3,PMFndisp|PMFfprop|PMFaltf,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_curindent,0,0 },

   { PMTYNUM,"Qline",3,PMFndisp|PMFfprop|PMFaltf|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curline,0,16777215 },

   { PMTYNUM,"Qchar",3,PMFndisp|PMFfprop|PMFaltf|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curchar,0,LINESIZE },

   { PMTYNUM,"Qscreen",3,PMFndisp|PMFfprop|PMFaltf|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curscreen,0,16777215 },

   { PMTYNUM,"Qmargin",3,PMFndisp|PMFfprop|PMFaltf|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curmargin,0,LINESIZE },

   { PMTYNUM,"Qwindow",3,PMFndisp|PMFfprop|PMFaltf|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curwindow,0,2 },

   { PMTYFLAG,"Qreadonly",2,PMFndisp|PMFfprop|PMFaltf,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curreadonly,0,0 },

   { PMTYSTR,"Qlanguage",3,PMFndisp|PMFfprop|PMFaltf,NULL,(PARMVAL) PSQlanguage,(PARMVAL)"",(PARMVAL)"",
	 P_curlanguage,16,0 },

   { PMTYNUM,"Qfileid",5,PMFndisp|PMFfprop|PMFaltf,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_fileid,0,32767 },

   { PMTYSTR,"Xfilename",9,PMFndisp|PMFstkd,NULL,(PARMVAL) PSXfilename,(PARMVAL)"/cs/lib/b.help",(PARMVAL)"",
	 P_filename,FNSIZE,0 },

   { PMTYFLAG,"Xcrypt",6,PMFndisp|PMFstkd|PMFfldflt,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curcrypt,0,0 },

   { PMTYFLAG,"Xstream",7,PMFndisp|PMFstkd|PMFfldflt,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curstream,0,0 },

   { PMTYFLAG,"Xtabin",6,PMFndisp|PMFstkd,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curtabin,0,0 },

   { PMTYFLAG,"Xtabout",7,PMFndisp|PMFstkd,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_curtabout,0,0 },

   { PMTYFLAG,"Xcrlf",6,PMFndisp|PMFstkd,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_curcrlf,0,0 },

   { PMTYFLAG,"Xindent",7,PMFndisp|PMFstkd,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_curindent,0,0 },

   { PMTYNUM,"Xcurline",8,PMFndisp|PMFstkd|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curline,0,16777215 },

   { PMTYNUM,"Xcurchar",8,PMFndisp|PMFstkd|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curchar,0,LINESIZE },

   { PMTYNUM,"Xcurscreen",10,PMFndisp|PMFstkd|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curscreen,0,16777215 },

   { PMTYNUM,"Xcurmargin",10,PMFndisp|PMFstkd|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curmargin,0,LINESIZE },

   { PMTYNUM,"Xcurwindow",6,PMFndisp|PMFstkd|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_curwindow,0,2 },

   { PMTYFLAG,"Xreadonly",9,PMFndisp|PMFstkd,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	P_curreadonly,0,0 },

   { PMTYSTR,"Xlanguage",9,PMFndisp|PMFstkd|PMFfldflt,NULL,(PARMVAL) PSXlanguage,(PARMVAL)"",(PARMVAL)"",
	 P_curlanguage,16,0 },

   { PMTYNUM,"Xfileid",7,PMFndisp|PMFstkd,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	P_fileid,0,32767 },

   { PMTYSTR,"terminal",1,PMFenv|PMFdisp,PRdisplay,(PARMVAL) PSterminal,(PARMVAL)"TERM",(PARMVAL)"ap",
	 -1,16,0 },

   { PMTYSTR,"interminal",3,PMFenv|PMFdisp,PRdisplay,(PARMVAL) PSinterminal,(PARMVAL)"INTERM",(PARMVAL)"ap",
	 -1,16,0 },

   { PMTYNUM,"backup",1,PMFnone,NULL,(PARMVAL)1,(PARMVAL)1,(PARMVAL)0,
	-1,0,9 },

   { PMTYFLAG,"slow",2,PMFnone|PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"rowarn",3,0,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYNUM,"savecount",2,PMFnone,NULL,(PARMVAL)20,(PARMVAL)20,(PARMVAL)0,
	-1,0,32767 },

   { PMTYFLAG,"doboth",2,0,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYFLAG,"doset",0,PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"nlock",2,0,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYSTR,"filetail",5,PMFndisp|PMFnone,PRflname,(PARMVAL) PSfiletail,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYSTR,"directory",2,PMFndisp|PMFnone,PRflname,(PARMVAL) PSdirectory,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYSTR,"tdirect",7,PMFndisp,PRdirchk,(PARMVAL) PStdirect,(PARMVAL)"/tmp",(PARMVAL)"/usr/tmp",
	 -1,24,0 },

   { PMTYFLAG,"crypt",1,PMFreset|PMFnoflip,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYFLAG,"stream",1,PMFreset|PMFnoflip,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYFLAG,"indent",1,PMFreset|PMFnoflip,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"readonly",1,PMFreset|PMFnoflip,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYSTR,"language",2,PMFreset,NULL,(PARMVAL) PSlanguage,(PARMVAL)"",(PARMVAL)"",
	 -1,16,0 },

   { PMTYFLAG,"tabin",4,PMFreset|PMFnoflip,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"tabout",2,PMFdelay|PMFnoflip,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	P_curtabout,0,0 },

   { PMTYFLAG,"crlf",4,PMFreset|PMFnoflip,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYFLAG,"newfile",1,PMFndisp|PMFfile,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYSTR,"nextfile",4,PMFfile|PMFndisp|PMFnhist,NULL,(PARMVAL) PSnextfile,(PARMVAL)"/cs/lib/b.help",(PARMVAL)"/cs/lib/b.help",
	 -1,FNSIZE,0 },

   { PMTYSTR,"afterfile",6,0,NULL,(PARMVAL) PSafterfile,(PARMVAL)"",(PARMVAL)"",
	 -1,1024,0 },

   { PMTYSTR,"editor",0,PMFndisp,NULL,(PARMVAL) PSeditor,(PARMVAL)"/cs/bin/bb",(PARMVAL)"/cs/bin/bb",
	 -1,FNSIZE,0 },

   { PMTYFLAG,"nonewfile",5,PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYNUM,"curwidth",0,PMFndisp|PMFdisp|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,16,LINESIZE },

   { PMTYNUM,"curlength",0,PMFndisp|PMFdisp|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,6,64 },

   { PMTYNUM,"width",5,PMFdisp|PMFdinit,PRdisplay,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,LINESIZE },

   { PMTYNUM,"length",2,PMFdisp|PMFdinit,PRdisplay,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,64 },

   { PMTYNUM,"termwidth",7,PMFdisp|PMFndisp,PRdisplay,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,LINESIZE },

   { PMTYNUM,"termlength",7,PMFdisp|PMFndisp,PRdisplay,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,64 },

   { PMTYFLAG,"silent",0,PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYNUM,"filesize",0,PMFndisp|PMFninit,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,32767 },

   { PMTYNUM,"line",2,PMFreset|PMFndisp,PRscrchk,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,16777215 },

   { PMTYNUM,"character",2,PMFreset|PMFndisp,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,LINESIZE },

   { PMTYNUM,"screen",2,PMFreset|PMFndisp,PRscrchk,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,16777215 },

   { PMTYNUM,"margin",2,PMFreset|PMFndisp,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,LINESIZE },

   { PMTYNUM,"window",2,PMFdisp,PRwindow,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,2 },

   { PMTYSTR,"winmark",4,PMFdisp,NULL,(PARMVAL) PSwinmark,(PARMVAL)"-",(PARMVAL)" ",
	 -1,2,0 },

   { PMTYNUM,"brief",2,PMFdisp,PRdisplay,(PARMVAL)1,(PARMVAL)1,(PARMVAL)0,
	-1,0,64 },

   { PMTYSTR,"home",0,PMFndisp|PMFenv,NULL,(PARMVAL) PShome,(PARMVAL)"HOME",(PARMVAL)"",
	 -1,256,0 },

   { PMTYSTR,"shell",0,PMFenv,NULL,(PARMVAL) PSshell,(PARMVAL)"SHELL",(PARMVAL)"",
	 -1,64,0 },

   { PMTYFLAG,"localcontrol",2,0,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"nocontrol",4,0,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"nochange",3,0,PRchange,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYFLAG,"savequery",9,0,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYNUM,"history",2,0,NULL,(PARMVAL)20,(PARMVAL)20,(PARMVAL)0,
	-1,0,16777215 },

   { PMTYNUM,"bells",4,0,NULL,(PARMVAL)2,(PARMVAL)2,(PARMVAL)0,
	-1,0,5 },

   { PMTYNUM,"errordelay",3,0,NULL,(PARMVAL)2,(PARMVAL)2,(PARMVAL)1,
	-1,0,10 },

   { PMTYFLAG,"minustop",6,PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"search",3,0,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"endclear",5,0,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYNUM,"rhsoption",5,0,NULL,(PARMVAL)1,(PARMVAL)1,(PARMVAL)0,
	-1,0,2 },

   { PMTYNUM,"priority",8,PMFndisp|PMFninit,NULL,(PARMVAL)19,(PARMVAL)19,(PARMVAL)19,
	-1,0,40 },

   { PMTYFLAG,"backspace",5,0,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"tabtypein",5,0,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"movewindow",5,0,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"eolspace",5,0,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"suspend",4,0,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYFLAG,"ateof",0,PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYNUM,"eofmark",3,PMFdisp,NULL,(PARMVAL)1,(PARMVAL)1,(PARMVAL)0,
	-1,0,2 },

   { PMTYFLAG,"autolang",5,0,NULL,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYFLAG,"mesg",4,PMFninit,PRmesg,(PARMVAL)TRUE,(PARMVAL)TRUE,(PARMVAL)FALSE,
	-1,0,0 },

   { PMTYSTR,"mail",4,PMFenv,NULL,(PARMVAL) PSmail,(PARMVAL)"MAILBOX",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYFLAG,"motd",4,0,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"marking",0,PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYNUM,"markline",8,PMFndisp,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,16777215 },

   { PMTYNUM,"markchar",8,PMFndisp,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,LINESIZE },

   { PMTYNUM,"mcntline",0,PMFndisp,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,16777215 },

   { PMTYNUM,"mcntchar",0,PMFndisp,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,LINESIZE },

   { PMTYFLAG,"copmode",0,PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"lopmode",0,PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"dosave",0,PMFndisp|PMFnhist,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"douse",0,PMFndisp|PMFnhist,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"srching",0,PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYSTR,"find",2,PMFndisp,NULL,(PARMVAL) PSfind,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYFLAG,"ioerr",0,PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYFLAG,"alternate",0,PMFndisp,NULL,(PARMVAL)FALSE,(PARMVAL)FALSE,(PARMVAL)TRUE,
	-1,0,0 },

   { PMTYNUM,"Apage",3,PMFcmd,NULL,(PARMVAL)1,(PARMVAL)1,(PARMVAL)1,
	-1,0,32767 },

   { PMTYNUM,"Aline",3,PMFcmd,NULL,(PARMVAL)7,(PARMVAL)7,(PARMVAL)7,
	-1,0,32767 },

   { PMTYNUM,"Aslide",3,PMFcmd,NULL,(PARMVAL)16,(PARMVAL)16,(PARMVAL)16,
	-1,0,32767 },

   { PMTYBFCT,"Agoto",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(BADBUF,0),(PARMVAL)BFCMAKE(BADBUF,0),(PARMVAL)0,
	-1,0,32767 },

   { PMTYBFCT,"Agotoend",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(BADBUF,0),(PARMVAL)BFCMAKE(BADBUF,0),(PARMVAL)0,
	-1,0,32767 },

   { PMTYBFCT,"Agotobeg",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(BADBUF,0),(PARMVAL)BFCMAKE(BADBUF,0),(PARMVAL)0,
	-1,0,32767 },

   { PMTYBUF,"Asavpos",3,PMFcmd,NULL,(PARMVAL)BADBUF,(PARMVAL)BADBUF,(PARMVAL)0,
	-1,0,32767 },

   { PMTYSTR,"Asearch",3,PMFctrl|PMFcmd,NULL,(PARMVAL) PSAsearch,(PARMVAL)"",(PARMVAL)"",
	 -1,512,0 },

   { PMTYSTR,"Arsearch",3,PMFctrl|PMFcmd,NULL,(PARMVAL) PSArsearch,(PARMVAL)"",(PARMVAL)"",
	 -1,512,0 },

   { PMTYSTR,"Afsearch",3,PMFctrl|PMFcmd,NULL,(PARMVAL) PSAfsearch,(PARMVAL)"",(PARMVAL)"",
	 -1,512,0 },

   { PMTYBFCT,"Absearch",3,0,NULL,(PARMVAL)BFCMAKE(BF_A,1),(PARMVAL)BFCMAKE(BF_A,1),(PARMVAL)PMFcmd,
	-1,0,32767 },

   { PMTYBFCT,"Acedit",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(23,1),(PARMVAL)BFCMAKE(23,1),(PARMVAL)BFCMAKE(23,1),
	-1,0,32767 },

   { PMTYBFCT,"Aledit",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(24,1),(PARMVAL)BFCMAKE(24,1),(PARMVAL)BFCMAKE(24,1),
	-1,0,32767 },

   { PMTYBFCT,"A1edit",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(BF_1,1),(PARMVAL)BFCMAKE(BF_1,1),(PARMVAL)BFCMAKE(BF_1,1),
	-1,0,32767 },

   { PMTYBFCT,"A2edit",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(BF_2,1),(PARMVAL)BFCMAKE(BF_2,1),(PARMVAL)BFCMAKE(BF_2,1),
	-1,0,32767 },

   { PMTYBFCT,"A3edit",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(BF_3,1),(PARMVAL)BFCMAKE(BF_3,1),(PARMVAL)BFCMAKE(BF_3,1),
	-1,0,32767 },

   { PMTYBFCT,"A4edit",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(BF_4,1),(PARMVAL)BFCMAKE(BF_4,1),(PARMVAL)BFCMAKE(BF_4,1),
	-1,0,32767 },

   { PMTYBFCT,"A5edit",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(BF_5,1),(PARMVAL)BFCMAKE(BF_5,1),(PARMVAL)BFCMAKE(BF_5,1),
	-1,0,32767 },

   { PMTYSTR,"Aexit",3,PMFctrl|PMFcmd,NULL,(PARMVAL) PSAexit,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYSTR,"Asystem",3,PMFcmd,NULL,(PARMVAL) PSAsystem,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYSTR,"Acommand",3,PMFcmd,NULL,(PARMVAL) PSAcommand,(PARMVAL)"",(PARMVAL)"",
	 -1,512,0 },

   { PMTYSTR,"Abfsave",3,PMFctrl|PMFcmd,NULL,(PARMVAL) PSAbfsave,(PARMVAL)"",(PARMVAL)"",
	 -1,512,0 },

   { PMTYBFCT,"Ado",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(BF_DO,1),(PARMVAL)BFCMAKE(BF_DO,1),(PARMVAL)BFCMAKE(BF_DO,-1),
	-1,0,32767 },

   { PMTYSTR,"Afile",3,PMFcmd,NULL,(PARMVAL) PSAfile,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYBFCT,"Ajoin",3,PMFcmd,NULL,(PARMVAL)BFCMAKE(BF_SINK,1),(PARMVAL)BFCMAKE(BF_SINK,1),(PARMVAL)BFCMAKE(BF_SINK,1),
	-1,0,32767 },

   { PMTYNUM,"Aindent",3,PMFcmd,NULL,(PARMVAL)1,(PARMVAL)1,(PARMVAL)1,
	-1,0,32767 },

   { PMTYSTR,"Ashow",3,PMFcmd,NULL,(PARMVAL) PSAshow,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYNUM,"Aundo",3,PMFcmd,NULL,(PARMVAL)1,(PARMVAL)1,(PARMVAL)1,
	-1,0,32767 },

   { PMTYSTR,"L0",2,PMFcmd,NULL,(PARMVAL) PSL0,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYSTR,"L1",2,PMFcmd,NULL,(PARMVAL) PSL1,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYSTR,"L2",2,PMFcmd,NULL,(PARMVAL) PSL2,(PARMVAL)"",(PARMVAL)"",
	 -1,FNSIZE,0 },

   { PMTYSTR,"L3",2,PMFcmd,NULL,(PARMVAL) PSL3,(PARMVAL)"",(PARMVAL)"",
	 -1,64,0 },

   { PMTYNUM,"L4",2,PMFcmd,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,32767 },

   { PMTYNUM,"L5",2,PMFcmd,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,32767 },

   { PMTYNUM,"L6",2,PMFcmd,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,32767 },

   { PMTYNUM,"L7",2,PMFcmd,NULL,(PARMVAL)0,(PARMVAL)0,(PARMVAL)0,
	-1,0,32767 },


};








/* end of parmdef.h */
