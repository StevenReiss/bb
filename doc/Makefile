bstandard.t: bstandard.n
	met -t bstandard.n
	mv \#pipe \#bstandard.t

b.help: h19.t vt100.t iq140.t iq120.t ex.t tvi950.t

h19.t: h19.keys.n h19.ctl.n
	cmdsed h19.keys.n h19.ctl.n | met -t -
	mv \#pipe \#h19.t

nsun.i: nsun.keys.n nsun.ctl.n
	cmdsed nsun.keys.n nsun.ctl.n > /tmp/docbb
	csh /u/spr/commands/trip /tmp/docbb
	rm /tmp/docbb

vt100.t: vt100.keys.n vt100.ctl.n
	cmdsed vt100.keys.n vt100.ctl.n | met -t -
	mv \#pipe \#vt100.t

tvi950.t: tvi950.keys.n tvi950.ctl.n
	cmdsed tvi950.keys.n tvi950.ctl.n | met -t -
	mv \#pipe \#tvi950.t

tvi920.t: tvi920.keys.n tvi920.ctl.n
	cmdsed tvi920.keys.n tvi920.ctl.n | met -t -
	mv \#pipe \#tvi920.t

sun.t: sun.keys.n sun.ctl.n
	cmdsed sun.keys.n sun.ctl.n | met -t -
	mv \#pipe \#sun.t

teleray.t: teleray.keys.n teleray.ctl.n
	cmdsed teleray.keys.n teleray.ctl.n | met -t -
	mv \#pipe \#teleray.t

ibm.t: ibm.keys.n ibm.ctl.n
	cmdsed ibm.keys.n ibm.ctl.n | met -t -
	mv \#pipe \#ibm.t

iq140.t: iq140.keys.n iq140.ctl.n
	cmdsed iq140.keys.n iq140.ctl.n | met -t -
	mv \#pipe \#iq140.t

iq120.t: iq120.keys.n iq120.ctl.n
	cmdsed iq120.keys.n iq120.ctl.n | met -t -
	mv \#pipe \#iq120.t

ex.t: ex.keys.n ex.ctl.n
	cmdsed ex.keys.n ex.ctl.n | met -t -
	mv \#pipe \#ex.t
