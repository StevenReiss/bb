/********************************************************/
/*							*/
/*		PARMS.H   (from parms.hm)		*/
/*							*/
/*	This file contains the definitions for using	*/
/*   parameters as parameters.	It is not particularly	*/
/*   specific (see parmdef.h), but does everything	*/
/*   that is needed by external modules 		*/
/*							*/
/********************************************************/
/*	Copyright 1984 Brown University 				*/



#define PMXSIZE 1024		/* maximum string space / parm	*/


#define PFILECT 45 	/* number of file parameters	*/
#define PCTRLCT 9 	/* number of control parameters */
#define PFILEST 0+FNSIZE+16+FNSIZE+16+FNSIZE+16	/* total file string size	*/
#define PCTRLST 0+FNSIZE+FNSIZE+512+512+512+FNSIZE+512	/* total control string size	*/



/* end of parameters */











/********************************************************/
/*							*/
/*	Type definitions				*/
/*							*/
/********************************************************/


typedef struct _PARM * PARM;
typedef struct _PARM   BPARM;


typedef enum {		/* parmeter types		*/
   PMTYNUM,		/*    numeric parameter 	*/
   PMTYSTR,		/*    string parameter		*/
   PMTYFLAG,		/*    boolean parameter 	*/
   PMTYBUF,		/*    buffer name parameter	*/
   PMTYBFCT,		/*    buffer/count parameter	*/
   PMTYIGN		/*    ignore argument		*/
 }
PARMTYPE;


typedef LONG	PARMFLGS;	/* parameter flags	*/

#define PMFnone 	0x0
#define PMFfile 	0x1	/*    change file	*/
#define PMFreset	0x2	/*    reset file	*/
#define PMFctrl 	0x4	/*    part of control	*/
#define PMFndisp	0x8	/*    dont display	*/
#define PMFcmd		0x10	/*    command parameter */
#define PMFfprop	0x20	/*    file property	*/
#define PMFdisp 	0x40	/*    rewrite display	*/
#define PMFenv		0x80	/*    take from environ */
#define PMFfldflt	0x100	/*    reset to default after file set	*/
#define PMFninit	0x200	/*    don't initialize  */
#define PMFset		0x400	/*    parameter has been set	*/
#define PMFprev 	0x800	/*    previous file parm	*/
#define PMFaltf 	0x1000	/*    alternate file parm	*/
#define PMFdelay	0x2000	/*    file delayed parm 	*/
#define PMFstkd 	0x4000	/*    file stack parm		*/
#define PMFnhist	0x8000	/*    dont save as history	*/
#define PMFfdset	0x10000 /*    file delayed set parm	*/
#define PMFnouser	0x20000 /*    cant be set by user	*/
#define PMFdinit	0x40000 /*    complete reinitialize disp*/
#define PMFnoflip	0x80000 /*    set parm without flip	*/


typedef char *	PARMVAL;	/* parameter value	*/





/********************************************************/
/*							*/
/*	Global routines 				*/
/*							*/
/********************************************************/


extern	void	Pinit();	/* initialize parms	*/
extern	void	Parm(); 	/* set parm from string */
extern	void	Pgetctrl();	/* get control area	*/
extern	void	Psetctrl();	/* save control area	*/
extern	void	Parmdo();	/* act on parm settings */
extern	int	Pset(); 	/* internal parm set	*/
extern	int	Pcmdarg();	/* set command arg parm */
extern	int	Pfini();	/* finish with parms	*/
extern	void	Pwriteall();	/* write all parms to file	*/







/********************************************************/
/*							*/
/*	Data structures -- parameter blocks		*/
/*							*/
/********************************************************/


struct	_PARM	{		/* parameter descriptor */
   PARMTYPE	PMtype; 	/*    type of parm	*/
   char *	PMname; 	/*    parameter name	*/
   char 	PMnmln; 	/*    min name match sz */
   PARMFLGS	PMflags;	/*    flags field	*/
   int		(*PMcall)();	/*    post process fct	*/
   PARMVAL	PMvalue;	/*    value		*/
   PARMVAL	PMdflt; 	/*    default value	*/
   PARMVAL	PMsdflt;	/*    set default value */
   long 	PMfluse;	/*    parm to use after */
				/*	  file set	*/
   long 	PMlbd;		/*    lower bound	*/
   long 	PMubd;		/*    upper bound	*/
 };
#define PMsize	PMlbd		/*    max string size	*/
#define PMtable PMlbd		/*    table pointer	*/






/********************************************************/
/*							*/
/*	Parameter value access functions		*/
/*							*/
/********************************************************/


#define BFCCOUNT(x)   ((INT)(((DATA)x)>>8))	/* buffer/count values	*/
#define BFCBUF(x)     ((INT)(((DATA)x) & 0xFF))
#define BFCMAKE(b,c)  ((((DATA)c)<<8)|(b&0xFF))
#define BADBUF	      0xFF




/********************************************************/
/*							*/
/*	access functions				*/
/*							*/
/********************************************************/


#ifndef __STDC__
#define PVAL(name)     ((PINT) PARMS[P_/**/name].PMvalue)
#define PDVAL(name)    ((DATA) PARMS[P_/**/name].PMvalue)
#define PSVAL(name)    ((STRING) PARMS[P_/**/name].PMvalue)
#define PSET(name,val)	Pset(P_/**/name,(PARMVAL)val)
#define PSETI(name,val)  Pset(P_/**/name,(PARMVAL)((PINT) val))
#else
#define PVAL(name)     ((PINT) PARMS[P_ ## name].PMvalue)
#define PDVAL(name)    ((DATA) PARMS[P_ ## name].PMvalue)
#define PSVAL(name)    ((STRING) PARMS[P_ ## name].PMvalue)
#define PSET(name,val)	Pset(P_ ## name,(PARMVAL)val)
#define PSETI(name,val)  Pset(P_ ## name,(PARMVAL)((PINT) val))
#endif




/********************************************************/
/*							*/
/*	access definitions				*/
/*							*/
/********************************************************/


extern	BPARM	PARMS[];


#define P_copy 0

#define P_savetemp 1

#define P_bufinfile 2

#define P_bufoutfile 3

#define P_share 4

#define P_useshare 5

#define P_huge 6

#define P_filename 7

#define P_curcrypt 8

#define P_curstream 9

#define P_curtabin 10

#define P_curtabout 11

#define P_curcrlf 12

#define P_curindent 13

#define P_curline 14

#define P_curchar 15

#define P_curscreen 16

#define P_curmargin 17

#define P_curwindow 18

#define P_curreadonly 19

#define P_curlanguage 20

#define P_fileid 21

#define P_curcopy 22

#define P_Pfilename 23

#define P_Pcrypt 24

#define P_Pstream 25

#define P_Ptabin 26

#define P_Ptabout 27

#define P_Pcrlf 28

#define P_Pindent 29

#define P_Pline 30

#define P_Pchar 31

#define P_Pscreen 32

#define P_Pmargin 33

#define P_Pwindow 34

#define P_Preadonly 35

#define P_Planguage 36

#define P_Pfileid 37

#define P_Qfilename 38

#define P_Qcrypt 39

#define P_Qstream 40

#define P_Qtabin 41

#define P_Qtabout 42

#define P_Qcrlf 43

#define P_Qindent 44

#define P_Qline 45

#define P_Qchar 46

#define P_Qscreen 47

#define P_Qmargin 48

#define P_Qwindow 49

#define P_Qreadonly 50

#define P_Qlanguage 51

#define P_Qfileid 52

#define P_Xfilename 53

#define P_Xcrypt 54

#define P_Xstream 55

#define P_Xtabin 56

#define P_Xtabout 57

#define P_Xcrlf 58

#define P_Xindent 59

#define P_Xcurline 60

#define P_Xcurchar 61

#define P_Xcurscreen 62

#define P_Xcurmargin 63

#define P_Xcurwindow 64

#define P_Xreadonly 65

#define P_Xlanguage 66

#define P_Xfileid 67

#define P_terminal 68

#define P_interminal 69

#define P_backup 70

#define P_slow 71

#define P_rowarn 72

#define P_savecount 73

#define P_doboth 74

#define P_doset 75

#define P_nlock 76

#define P_filetail 77

#define P_directory 78

#define P_tdirect 79

#define P_crypt 80

#define P_stream 81

#define P_indent 82

#define P_readonly 83

#define P_language 84

#define P_tabin 85

#define P_tabout 86

#define P_crlf 87

#define P_newfile 88

#define P_nextfile 89

#define P_afterfile 90

#define P_editor 91

#define P_nonewfile 92

#define P_curwidth 93

#define P_curlength 94

#define P_width 95

#define P_length 96

#define P_termwidth 97

#define P_termlength 98

#define P_silent 99

#define P_filesize 100

#define P_line 101

#define P_character 102

#define P_screen 103

#define P_margin 104

#define P_window 105

#define P_winmark 106

#define P_brief 107

#define P_home 108

#define P_shell 109

#define P_localcontrol 110

#define P_nocontrol 111

#define P_nochange 112

#define P_savequery 113

#define P_history 114

#define P_bells 115

#define P_errordelay 116

#define P_minustop 117

#define P_search 118

#define P_endclear 119

#define P_rhsoption 120

#define P_priority 121

#define P_backspace 122

#define P_tabtypein 123

#define P_movewindow 124

#define P_eolspace 125

#define P_suspend 126

#define P_ateof 127

#define P_eofmark 128

#define P_autolang 129

#define P_mesg 130

#define P_mail 131

#define P_motd 132

#define P_marking 133

#define P_markline 134

#define P_markchar 135

#define P_mcntline 136

#define P_mcntchar 137

#define P_copmode 138

#define P_lopmode 139

#define P_dosave 140

#define P_douse 141

#define P_srching 142

#define P_find 143

#define P_ioerr 144

#define P_alternate 145

#define P_Apage 146

#define P_Aline 147

#define P_Aslide 148

#define P_Agoto 149

#define P_Agotoend 150

#define P_Agotobeg 151

#define P_Asavpos 152

#define P_Asearch 153

#define P_Arsearch 154

#define P_Afsearch 155

#define P_Absearch 156

#define P_Acedit 157

#define P_Aledit 158

#define P_A1edit 159

#define P_A2edit 160

#define P_A3edit 161

#define P_A4edit 162

#define P_A5edit 163

#define P_Aexit 164

#define P_Asystem 165

#define P_Acommand 166

#define P_Abfsave 167

#define P_Ado 168

#define P_Afile 169

#define P_Ajoin 170

#define P_Aindent 171

#define P_Ashow 172

#define P_Aundo 173

#define P_L0 174

#define P_L1 175

#define P_L2 176

#define P_L3 177

#define P_L4 178

#define P_L5 179

#define P_L6 180

#define P_L7 181





#define PM_LAST 0x1	/* flag indicating last parm	*/

#define NUMPARM 182 /* number of parameters	*/





/* end of parms.h */
