/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;

import cseis.swing.csAboutDialog;
import cseis.swing.csRecentFileMenu;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.InputEvent;
import java.awt.event.KeyEvent;
import javax.swing.*;

/**
 * Menu bar for XCSeis
 */
@SuppressWarnings("serial")
public class csMenuBar extends JMenuBar {
  private final static int MAX_RECENT_FILES = 10;
  public final static int NONE                  = 100;
  public final static int EXISTING_FILE         = 101;
  public final static int EXISTING_CHANGED_FILE = 102;
  public final static int NEW_FILE              = 103;
  public final static int NEW_CHANGED_FILE      = 104;
  private XCSeis myXCSeis;

  private JMenu myMenuFile;
  private JMenu myMenuView;
  private JMenu myMenuRun;
  private JMenu myMenuHelp;

  private JMenuItem myMenuOpen;
  private JMenuItem myMenuNew;
  private JMenuItem myMenuOpenLog;
  private JMenuItem myMenuExit;
  private JMenuItem myMenuRefresh;
  private JMenuItem myMenuClose;
  private JMenuItem myMenuSetProj;

  private JMenuItem myMenuSave;
  private JMenuItem myMenuSaveAs;
  private JMenuItem myMenuSubmit;

  private JMenuItem myMenuTools;
  private JMenuItem myMenuSettings;
  
  private JMenuItem myMenuAbout;


  private csRecentFileMenu myMenuRecentFiles;

  public csMenuBar( XCSeis xcseis ) {
    myXCSeis = xcseis;

    myMenuRecentFiles = new csRecentFileMenu("Recently opened files", MAX_RECENT_FILES);
    myMenuRecentFiles.addFileMenuListener(myXCSeis);

    myMenuFile  = new JMenu("File");
    myMenuView  = new JMenu("View");
    myMenuRun   = new JMenu("Run");
    myMenuTools = new JMenu("Tools");
    myMenuHelp  = new JMenu("Help");
    myMenuSubmit = new JMenuItem("Submit");

    myMenuOpen    = new JMenuItem("Open...");
    myMenuOpen.setAccelerator( KeyStroke.getKeyStroke(KeyEvent.VK_O, InputEvent.CTRL_MASK) );
    myMenuNew    = new JMenuItem("New");
    myMenuOpenLog    = new JMenuItem("Open log file");
    myMenuSaveAs   = new JMenuItem("Save as...");
    myMenuSaveAs.setAccelerator( KeyStroke.getKeyStroke(KeyEvent.VK_S, InputEvent.SHIFT_MASK | InputEvent.CTRL_MASK) );
    myMenuSave   = new JMenuItem("Save");
    myMenuSave.setAccelerator( KeyStroke.getKeyStroke(KeyEvent.VK_S, InputEvent.CTRL_MASK) );
    myMenuSetProj = new JMenuItem("Project setup...");
    myMenuExit    = new JMenuItem("Exit");
    myMenuExit.setAccelerator( KeyStroke.getKeyStroke(KeyEvent.VK_Q, InputEvent.CTRL_MASK) );

    myMenuRefresh   = new JMenuItem("Refresh");
    myMenuClose   = new JMenuItem("Close");

    myMenuSettings  = new JMenuItem("Settings...");
    
    myMenuAbout   = new JMenuItem("About");

    myMenuFile.add(myMenuOpen);
    myMenuFile.add(myMenuRecentFiles);
    myMenuFile.add(myMenuNew);
    myMenuFile.addSeparator();
    myMenuFile.add(myMenuSave);
    myMenuFile.add(myMenuSaveAs);

    myMenuFile.addSeparator();
    myMenuFile.add(myMenuOpenLog);
    myMenuFile.add(myMenuRefresh);
    myMenuFile.add(myMenuClose);
    myMenuFile.addSeparator();
    myMenuFile.add(myMenuSetProj);
    myMenuFile.addSeparator();
    myMenuFile.add(myMenuExit);

    myMenuRun.add( myMenuSubmit );

    myMenuTools.add( myMenuSettings );

    myMenuHelp.add(myMenuAbout);

    add(myMenuFile);
//    add(myMenuView);
    add(myMenuRun);
    add(myMenuTools);
    add(myMenuHelp);

    setToolTips();

    setMenusEnabled( csMenuBar.NONE );

    myMenuExit.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
      String[] buttonLabels = {"Yes", "No"};
      int selectedOption = JOptionPane.showOptionDialog( myXCSeis,
          "Are you sure you want to quit?\n",
          "Confirm exit",
          JOptionPane.YES_NO_OPTION,
          JOptionPane.QUESTION_MESSAGE,
          null, buttonLabels, buttonLabels[1] );
        if( selectedOption == JOptionPane.YES_OPTION ) {
          myXCSeis.exitApplication();
        }
      }
    });

    myMenuSubmit.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myXCSeis.submitFlow();
      }
    });
    myMenuSetProj.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myXCSeis.projectSetup();
      }
    });

    myMenuNew.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myXCSeis.newFlow();
      }
    });
    myMenuOpen.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        if( myXCSeis.openFlow() ) {
          setMenusEnabled(csMenuBar.EXISTING_FILE);
        }
      }
    });
    myMenuOpenLog.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myXCSeis.openRecentLog();
      }
    });
    myMenuRefresh.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myXCSeis.refreshFile();
      }
    });
    myMenuClose.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myXCSeis.closeFlow();
      }
    });
    myMenuSave.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myXCSeis.saveFlow();
      }
    });
    myMenuSaveAs.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myXCSeis.saveFlowAs();
      }
    });

    myMenuSettings.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myXCSeis.openSettingsDialog();
      }
    });

    myMenuAbout.addActionListener( new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        csAboutDialog dialog = new csAboutDialog( myXCSeis, "XCSeis" );
        dialog.setVersionString( XCSeis.VERSION );
        dialog.setContact("john@dix.mines.edu  ");
        dialog.setDate("2012  ");
        dialog.setAdditionalComments("Copyright (c) Colorado School of Mines, 2013\n" +
                "All rights reserved.\n" +
                "Based on SeaSeis, developed by Bjorn Olofsson.");
//        dialog.setLogo( cseis.resources.csResources.getIcon("xcseis_icon_smooth.png") );
//        dialog.setAdditionalComments("Free software.");
        dialog.setVisible(true);
      }
    });
  }
  public void setMenusEnabled( int flag ) {
    myMenuSave.setEnabled( flag == csMenuBar.EXISTING_CHANGED_FILE || flag == csMenuBar.NEW_CHANGED_FILE );
    myMenuRefresh.setEnabled( flag == csMenuBar.EXISTING_FILE || flag == csMenuBar.EXISTING_CHANGED_FILE  );
    myMenuOpenLog.setEnabled( flag == csMenuBar.EXISTING_FILE || flag == csMenuBar.EXISTING_CHANGED_FILE );
    myMenuSaveAs.setEnabled( flag != csMenuBar.NONE );
    myMenuClose.setEnabled( flag != csMenuBar.NONE );
  }
  public void setSubmissionEnabled( boolean doEnable ) {
    myMenuSubmit.setEnabled(doEnable);
  }
  private void setToolTips() {
    myMenuNew      .setToolTipText("New flow");
    myMenuOpen     .setToolTipText("Open flow");
    myMenuSave     .setToolTipText("Save changes to current flow file");
    myMenuSaveAs   .setToolTipText("Save flow to different file");
    myMenuSetProj  .setToolTipText("Set project directory etc");
    myMenuExit     .setToolTipText("Exit");
    myMenuAbout    .setToolTipText("About");
    myMenuOpenLog  .setToolTipText("Open recent log file of current flow");
    myMenuRefresh  .setToolTipText("Refresh/Re-open current flow");
    myMenuClose    .setToolTipText("Close current flow");
  }
  public void removeRecentFile( String filename ) {
    myMenuRecentFiles.removeFile( filename );
  }
  public void addRecentFile( String filename ) {
    myMenuRecentFiles.addFile( filename );
  }
  public java.util.List<String> getRecentFileList() {
    return myMenuRecentFiles.getFileList();
  }
}


