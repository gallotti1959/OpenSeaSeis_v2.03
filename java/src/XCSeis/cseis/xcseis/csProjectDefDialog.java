/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;

import cseis.general.csStandard;
import java.awt.*;
import java.awt.Color;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import javax.swing.*;
import javax.swing.event.CaretEvent;
import javax.swing.event.CaretListener;

public class csProjectDefDialog extends JDialog {
  private static final int TEXT_MIN_WIDTH = 200;

  private Color myColorError = new Color(255,128,128);
  private csProjectDef myProjDef;
  private JTextField myTextProjDir;
  private JTextField myTextFlowDir;
  private JTextField myTextLogDir;
  private JButton myButtonProjDir;
  private XCSeis myFrame;

  private JButton myButtonClose;

  public csProjectDefDialog( XCSeis parent, csProjectDef projDef ) {
    super( parent, "Project setup" );
    setModal(false);

    myFrame = parent;
    myTextProjDir    = new JTextField( projDef.pathProj() );
    myTextFlowDir    = new JTextField( projDef.pathFlows() );
    myTextLogDir     = new JTextField( projDef.pathLogs() );
    myTextFlowDir.setEditable(false);
    myTextLogDir.setEditable(false);
    myButtonProjDir  = new JButton("Browse..");
    myProjDef = new csProjectDef( projDef.pathProj() );

    int height = myTextProjDir.getPreferredSize().height;
    myTextProjDir.setMinimumSize( new Dimension(TEXT_MIN_WIDTH,height) );
    myTextFlowDir.setMinimumSize( new Dimension(TEXT_MIN_WIDTH,height) );
    myTextLogDir.setMinimumSize( new Dimension(TEXT_MIN_WIDTH,height) );
    myTextProjDir.setPreferredSize( new Dimension(TEXT_MIN_WIDTH,height) );
    myTextFlowDir.setPreferredSize( new Dimension(TEXT_MIN_WIDTH,height) );
    myTextLogDir.setPreferredSize( new Dimension(TEXT_MIN_WIDTH,height) );
    
    //=============================================================================

    JPanel panelDir = new JPanel(new GridBagLayout());
    panelDir.setBorder( BorderFactory.createCompoundBorder(
        BorderFactory.createTitledBorder("Project directory"),
        csStandard.INNER_EMPTY_BORDER ) );

    int yp = 0;
    panelDir.add( myButtonProjDir, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 5, 0, 0 ), 0, 0 ) );
    panelDir.add( myTextProjDir, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 5, 0, 0 ), 0, 0 ) );
    panelDir.add( new JLabel("Flow dir:"), new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 5, 0, 0 ), 0, 0 ) );
    panelDir.add( myTextFlowDir, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 5, 0, 0 ), 0, 0 ) );
    panelDir.add( new JLabel("Log dir:"), new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 5, 0, 0 ), 0, 0 ) );
    panelDir.add( myTextLogDir, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 5, 0, 0 ), 0, 0 ) );
    panelDir.add( Box.createVerticalGlue(), new GridBagConstraints(
        0, yp++, 2, 1, 1.0, 1.0, GridBagConstraints.WEST,
        GridBagConstraints.BOTH, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    //  ----------------------------
    //  Button panel
    myButtonClose   = new JButton("Close");
    this.getRootPane().setDefaultButton(myButtonClose);

    JPanel panelButtons = new JPanel( new GridBagLayout() );
    int xp = 0;
    panelButtons.add( Box.createHorizontalGlue(), new GridBagConstraints(
        xp++, 0, 1, 1, 0.9, 1.0, GridBagConstraints.CENTER,
        GridBagConstraints.BOTH, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelButtons.add( myButtonClose, new GridBagConstraints(
        xp++, 0, 1, 1, 0.0, 0.0, GridBagConstraints.SOUTHWEST,
        GridBagConstraints.BOTH, new Insets( 11, 0, 0, 0 ), 0, 0 ) );
    panelButtons.add( Box.createHorizontalGlue(), new GridBagConstraints(
        xp++, 0, 1, 1, 0.1, 1.0, GridBagConstraints.CENTER,
        GridBagConstraints.BOTH, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    JPanel panelAll = new JPanel(new BorderLayout());
    panelAll.setBorder( csStandard.DIALOG_BORDER );

    yp = 0;

    panelAll.add(panelDir,BorderLayout.CENTER);
    panelAll.add(panelButtons,BorderLayout.SOUTH);
    getContentPane().add(panelAll);

    pack();
    setLocationRelativeTo( myFrame );

    //------------------------------------------------------------
    //
    myButtonClose.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        apply();
        dispose();
      }
    });
    myTextProjDir.addActionListener(new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myTextProjDir.setBackground(Color.white);
        if( !apply() ) {
          myTextProjDir.setBackground(myColorError);
        }
      }
    });
    myTextProjDir.addCaretListener( new CaretListener() {
      public void caretUpdate(CaretEvent e) {
        myTextProjDir.setBackground(Color.white);
      }
    });
    myButtonProjDir.addActionListener(new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myTextProjDir.setBackground(Color.white);
        selectProjDir();
      }
    });

    this.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        cancel();
      }
    });
  }
  private boolean apply() {
    String newProjDir = myTextProjDir.getText();
    csProjectDef projDef = new csProjectDef(newProjDir); 
//    if( newProjDir.compareTo(myProjDef.pathProj()) == 0 ) {
      // No change, just return
//      return true;
//    }
    File newProjDirFile = new File(newProjDir);
    boolean createDirs = false;
    if( !newProjDirFile.exists() ) {
      int option = JOptionPane.showConfirmDialog(this, "Create new directory \n" + newProjDir + " ?",
              "Create new directory", JOptionPane.YES_NO_OPTION);
      if( option != JOptionPane.OK_OPTION ) {
        myTextProjDir.setBackground(myColorError);
        return false;
      }
      createDirs = true;
    }
    else {
      projDef.set(newProjDir);
      if( !projDef.checkDirs() ) {
        int option = JOptionPane.showConfirmDialog(this, "Create sub-directories " +
                csProjectDef.SUBDIR_FLOWS + " and " + csProjectDef.SUBDIR_LOGS + " under \n" +
                projDef.pathProj() + " ?",
              "Create new directories", JOptionPane.YES_NO_OPTION);
        if( option != JOptionPane.OK_OPTION ) {
          myTextProjDir.setBackground(myColorError);
          return false;
        }
        createDirs = true;
      }
    }
    if( createDirs ) {
      try {
        projDef.createDirs();
      }
      catch (Exception ex) {
        JOptionPane.showMessageDialog(this, ex.getMessage(), "Error occurred", JOptionPane.ERROR_MESSAGE);
        myTextProjDir.setBackground(myColorError);
        return false;
      }
      JOptionPane.showMessageDialog(this, "Successfully created project directory\n" + projDef.pathProj() +
              "\nincluding subdirectories " + csProjectDef.SUBDIR_FLOWS + " and " + csProjectDef.SUBDIR_LOGS,
              "Directories created", JOptionPane.INFORMATION_MESSAGE);
    }
    myProjDef.set(newProjDir);
    myFrame.setProjectDir( newProjDir );
    refresh();
    return true;
  }
  private void cancel() {
    dispose();
  }
  private void selectProjDir() {
    JFileChooser fs = new JFileChooser( myTextProjDir.getText() );
    fs.setFileSelectionMode( JFileChooser.DIRECTORIES_ONLY );
    int option = fs.showOpenDialog(this);
    if( option == JFileChooser.APPROVE_OPTION ) {
      String path = fs.getSelectedFile().getAbsolutePath();
      myTextProjDir.setText(path);
      apply();
    }
  }
  private void refresh() {
    myTextProjDir.setBackground(Color.white);
    myTextProjDir.setText(myProjDef.pathProj());
    myTextFlowDir.setText(myProjDef.pathFlows());
    myTextLogDir.setText(myProjDef.pathLogs());
  }
}


