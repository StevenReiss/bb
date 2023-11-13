/********************************************************************************/
/*										*/
/*		SteamPekoeManager.java						*/
/*										*/
/*	Steam pattern manager							*/
/*										*/
/********************************************************************************/
/*	Copyright 1998 Brown University -- Steven P. Reiss		      */
/*********************************************************************************
 *  Copyright 1998, Brown University, Providence, RI.				 *
 *										 *
 *			  All Rights Reserved					 *
 *										 *
 *  Permission to use, copy, modify, and distribute this software and its	 *
 *  documentation for any purpose other than its incorporation into a		 *
 *  commercial product is hereby granted without fee, provided that the 	 *
 *  above copyright notice appear in all copies and that both that		 *
 *  copyright notice and this permission notice appear in supporting		 *
 *  documentation, and that the name of Brown University not be used in 	 *
 *  advertising or publicity pertaining to distribution of the software 	 *
 *  without specific, written prior permission. 				 *
 *										 *
 *  BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS		 *
 *  SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND		 *
 *  FITNESS FOR ANY PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY	 *
 *  BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY 	 *
 *  DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,		 *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS		 *
 *  ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 	 *
 *  OF THIS SOFTWARE.								 *
 *										 *
 ********************************************************************************/

/* RCS: $Header$ */


/*********************************************************************************
 *
 * $Log$
 *
 ********************************************************************************/



package tea.steam.pekoe;

import tea.steam.*;
import tea.prom.*;
import tea.pekoe.*;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.event.*;
import java.awt.*;

import java.io.File;
import java.util.Vector;
import java.util.Enumeration;




public class SteamPekoeManager extends JFrame implements SteamConstants
{


/****************************************************************************************/
/*											*/
/*	Constants									*/
/*											*/
/****************************************************************************************/


private static final int	VIEW_BY_NAME = 0;
private static final int	VIEW_BY_TYPE = 1;



/****************************************************************************************/
/*											*/
/*	Private Storage 								*/
/*											*/
/****************************************************************************************/

private PromProject	cur_project;
private int		view_by;
private MenuBar 	menu_bar;
private JList		pattern_list;
private JPanel		display_panel;




/****************************************************************************************/
/*											*/
/*	Main Program									*/
/*											*/
/****************************************************************************************/

public static void main(String args[])
{
   SteamSetup ss = new SteamSetup();
   SteamPekoeManager spm = new SteamPekoeManager(null);
   spm.addWindowListener(new SteamRootMonitor(true));
   spm.setVisible(true);
}



/****************************************************************************************/
/*											*/
/*	Constructors									*/
/*											*/
/****************************************************************************************/

public SteamPekoeManager(PromProject proj)
{
   super("TEA Pattern Manager");

   JRootPane jrp = getRootPane();
   jrp.setDoubleBuffered(true);

   cur_project = null;
   view_by = VIEW_BY_NAME;

   setupWindow();

   if (proj != null) setProject(proj);
}



private void setupWindow()
{
   menu_bar = new MenuBar(this);

   setSize(200,70);

   setJMenuBar(menu_bar);

   display_panel = new JPanel();
   display_panel.setDoubleBuffered(true);
   setContentPane(display_panel);

   display_panel.add(new JLabel("<No Project>"));
   display_panel.revalidate();
}



/****************************************************************************************/
/*											*/
/*	Access Methods									*/
/*											*/
/****************************************************************************************/


PromProject getProject()			{ return cur_project; }

int getViewBy() 				{ return view_by; }
void setViewBy(int vb)				{ view_by = vb; }



/****************************************************************************************/
/*											*/
/*	Methods for setting up a project						*/
/*											*/
/****************************************************************************************/

void updateProject()
{
   setProject(cur_project);
}



void setProject(PromProject proj)
{
   cur_project = proj;

   listPatterns();

   menu_bar.setProjectMenu();
}



private void listPatterns()
{
   PekoeControl pc = PekoeControl.getControl(cur_project);
   PekoeLibrary pl = pc.getLibrary();
   String [] patnms = pl.getElements();
   PatInstance [] patins = new PatInstance[patnms.length];
   PekoeInstance pi,pj;
   boolean fg;

   for (int i = 0; i < patnms.length; ++i) {
      patins[i] = new PatInstance(pl,patnms[i]);
    }

   for (int i = 0; i < patins.length; ++i) {
      for (int j = i+1; j < patins.length; ++j) {
	 if (patins[j].lessThan(patins[i],view_by)) {
	    PatInstance pt = patins[i];
	    patins[i] = patins[j];
	    patins[j] = pt;
	  }
       }
    }

   pattern_list = new JList(patins);
   pattern_list.addListSelectionListener(new ListManager());

   display_panel.removeAll();
   display_panel.add(new JScrollPane(pattern_list));

   setSize(getPreferredSize());
   display_panel.revalidate();
   setSize(getPreferredSize());
}




private class ListManager implements ListSelectionListener {

   public void valueChanged(ListSelectionEvent e) {
      Object [] selected = pattern_list.getSelectedValues();
      menu_bar.setPatternMenu(selected.length);
    }


}	// end of class ListManager




private static class PatInstance {

   private String file_name;
   private PekoeInstance pat_instance;

   PatInstance(PekoeLibrary pl,String nm) {
      if (nm.endsWith(".pat")) file_name = nm.substring(0,nm.length()-4);
      else file_name = nm;
      pat_instance = pl.findElement(file_name);
    }

   public boolean lessThan(PatInstance pi,int mode) {
      int i = 0;
      if (mode == VIEW_BY_TYPE) {
	 i = pat_instance.getPatternName().compareTo(pi.pat_instance.getPatternName());
       }
      if (i == 0) i = file_name.compareTo(pi.file_name);
      return i < 0;
    }

   public String toString() {      String s = file_name;      if (pat_instance != null) {
	 s += "   (" + pat_instance.getPatternName() + ")";      }      return s;
    }

   String getName()		{ return file_name; }
   PekoeInstance getInstance()	{ return pat_instance; }


}	// end of subclass PatInstance



/****************************************************************************************/
/*											*/
/*	Pattern editing methods 							*/
/*											*/
/****************************************************************************************/


void deletePatterns()
{
   Object [] selected = pattern_list.getSelectedValues();
   PekoeControl pc = PekoeControl.getControl(cur_project);
   PekoeLibrary pl = pc.getLibrary();

   for (int i = 0; i < selected.length; ++i) {
      PatInstance pi = (PatInstance) selected[i];
      pl.removeElement(pi.getName());
    }

   updateProject();
}



void renamePattern()
{
   PatInstance pi = (PatInstance) pattern_list.getSelectedValue();
   String pnm = pi.getName();

   String s = JOptionPane.showInputDialog(this,"Enter new name for " + pnm,
					     "Rename Input",
					     JOptionPane.QUESTION_MESSAGE);
   if (s != null) s = s.trim();

   if (s != null && s.length() > 0) {
      PekoeControl pc = PekoeControl.getControl(cur_project);
      PekoeLibrary pl = pc.getLibrary();
      pl.renameElement(pnm,s);
      updateProject();
    }
}



void duplicatePattern()
{
   PatInstance pi = (PatInstance) pattern_list.getSelectedValue();
   String pnm = pi.getName();

   String s = JOptionPane.showInputDialog(this,"Enter duplicate name for " + pnm,
					     "Duplicate Input",
					     JOptionPane.QUESTION_MESSAGE);
   if (s != null) s = s.trim();

   if (s != null && s.length() > 0) {
      PekoeControl pc = PekoeControl.getControl(cur_project);
      PekoeLibrary pl = pc.getLibrary();
      pl.addElement(s,pi.getInstance());
      updateProject();
    }
}



/****************************************************************************************/
/*											*/
/*	MenuBar methods and subclasses							*/
/*											*/
/****************************************************************************************/

private static class MenuBar extends SteamMenuBar implements ActionListener {

   private SteamPekoeManager the_manager;
   private JMenuItem find_btn;
   private JMenuItem create_btn;
   private JMenu edit_menu;
   private JMenuItem rename_btn;
   private JMenuItem duplicate_btn;
   private JMenuItem edit_btn;

   MenuBar(SteamPekoeManager pm) {
      the_manager = pm;

      JMenu filemenu = new JMenu("File");
      edit_menu = new JMenu("Edit");
      JMenu viewmenu = new JMenu("View");
      JMenuItem btn;

      btn = addButton(filemenu,"Set Project ...","Change the project being edited");
      find_btn = addButton(filemenu,"Find Pattern","Start the TEA Pattern Finder");
      find_btn.setEnabled(false);
      create_btn = addButton(filemenu,"Create Pattern","Start the TEA Pattern Creater");
      create_btn.setEnabled(false);
//    btn = addButton(filemenu,"Quit","Quit the project manager");
      add(filemenu);

      edit_btn = addButton(edit_menu,"Edit","Edit the selected pattern");
      edit_btn.setEnabled(false);
      btn = addButton(edit_menu,"Delete","Delete the selected pattern(s)");
      rename_btn = addButton(edit_menu,"Rename","Rename the selected pattern");
      duplicate_btn = addButton(edit_menu,"Duplicate","Duplicate the selected pattern");
      add(edit_menu);
      edit_menu.setEnabled(false);

      btn = addButton(viewmenu,"Update","Update the list of patterns");
      ButtonGroup grp = new ButtonGroup();
      btn = addRadioButton(viewmenu,grp,"By Name",the_manager.getViewBy() == VIEW_BY_NAME,
			      "Sort patterns by their names");
      btn = addRadioButton(viewmenu,grp,"By Type",the_manager.getViewBy() == VIEW_BY_TYPE,
			      "Sort patterns by pattern type");
      add(viewmenu);
    }


   void setProjectMenu() {
      find_btn.setEnabled(true);
//    create_btn.setEnabled(true);
    }

   void setPatternMenu(int ct) {
      if (ct == 0) edit_menu.setEnabled(false);
      else {
	 edit_menu.setEnabled(true);
//	 edit_btn.setEnabled(ct == 1);
	 rename_btn.setEnabled(ct == 1);
	 duplicate_btn.setEnabled(ct == 1);
       }
    }

   public void actionPerformed(ActionEvent e) {
      String btn = e.getActionCommand();
      if (btn.equals("Quit")) {
//	 the_manager.setVisible(false); 	    // this fails to work properly
//	 the_manager.dispose();
       }
      if (btn.equals("Find Pattern")) {
	 SteamPekoeFinder spf = new SteamPekoeFinder(the_manager.getProject());
	 spf.addWindowListener(new SteamRootMonitor());
	 spf.setVisible(true);
       }
      else if (btn.equals("Create Pattern")) { }
      else if (btn.equals("Edit")) { }
      else if (btn.equals("Delete")) {
	 the_manager.deletePatterns();
       }
      else if (btn.equals("Rename")) {
	 the_manager.renamePattern();
       }
      else if (btn.equals("Duplicate")) {
	 the_manager.duplicatePattern();
       }
      else if (btn.equals("Update")) {
	 the_manager.updateProject();
       }
      else if (btn.equals("By Name")) {
	 the_manager.setViewBy(VIEW_BY_NAME);
	 the_manager.updateProject();
       }
      else if (btn.equals("By Type")) {
	 the_manager.setViewBy(VIEW_BY_TYPE);
	 the_manager.updateProject();
       }
      else if (btn.equals("Set Project ...")) {
	 SetProjectAction spa = new SetProjectAction(the_manager);
	 SwingUtilities.invokeLater(spa);
       }
    }

}	// end of subclass MenuBar



/****************************************************************************************/
/*											*/
/*	SetProjectAction subclass							*/
/*											*/
/****************************************************************************************/

static private class SetProjectAction implements Runnable {

   private SteamPekoeManager the_manager;

   SetProjectAction(SteamPekoeManager pm)		{ the_manager = pm; }

   public void run() {
      SteamProjectDialog spd = new SteamProjectDialog(the_manager);
      PromProject pm = spd.selectProject(the_manager.getProject(),
					    "Manage Patterns for Project:");
      if (pm != null) the_manager.setProject(pm);
    }

}	// end of subclass SetProjectAction




}	// end of class SteamPekoeManager




/* end of SteamPekoeManager.java */
