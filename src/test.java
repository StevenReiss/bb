void test {

   if( si_state == 1 && state == STATE_ASCII ) {
      switch (c) {
	 case ' ':
	    return si_write();
	 case 'q':
	    return si_write('-');      // horizontal line
	 case 'x':
	    return si_write('|');      // vertical line
	 case 'l':
	    return si_write('+');      // top left corner
	 case 'k':
	    return si_write('+');      // top right corner
	 case 'm':
	    return si_write('+');      // bottom left corner
	 case 'j':
	    return si_write('+');      // bottom right corner
	 case '\r':
	    terminal.setCursorPos(terminal.getCursorRow() + 1, 0);
	    return true;
       }
