/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;

import cseis.general.csStandard;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.*;
import javax.swing.event.CaretEvent;
import javax.swing.event.CaretListener;

public class csSettingsDialog extends JDialog {
  private static final int TEXT_MIN_WIDTH = 200;

  private Color myColorError = new Color(255,128,128);
  private JTextField myTextFontSize;
  private XCSeis myFrame;

  private JButton myButtonClose;

  public csSettingsDialog( XCSeis parent ) {
    super( parent, "Settings" );
    setModal(true);

    myFrame = parent;
    
    myTextFontSize   = new JTextField( "" + myFrame.getSystemFont().getSize() );
    int height = myTextFontSize.getPreferredSize().height;
    myTextFontSize.setPreferredSize( new Dimension(TEXT_MIN_WIDTH,height) );
    myTextFontSize.setMinimumSize( new Dimension(TEXT_MIN_WIDTH,height) );
    
    //=============================================================================

    JPanel panelFont = new JPanel(new GridBagLayout());
    panelFont.setBorder( BorderFactory.createCompoundBorder(
        BorderFactory.createTitledBorder("View settings"),
        csStandard.INNER_EMPTY_BORDER ) );

    int yp = 0;
    panelFont.add( new JLabel("Font size:"), new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 5, 0, 0 ), 0, 0 ) );
    panelFont.add( myTextFontSize, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 5, 0, 0 ), 0, 0 ) );
    panelFont.add( Box.createVerticalGlue(), new GridBagConstraints(
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

    panelAll.add(panelFont,BorderLayout.CENTER);
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
    myTextFontSize.addActionListener(new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        myTextFontSize.setBackground(Color.white);
        if( !apply() ) {
          myTextFontSize.setBackground(myColorError);
        }
      }
    });
    myTextFontSize.addCaretListener( new CaretListener() {
      public void caretUpdate(CaretEvent e) {
        myTextFontSize.setBackground(Color.white);
      }
    });

    this.addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        cancel();
      }
    });
  }
  private boolean apply() {
    try {
      int newFontSize = Integer.parseInt(myTextFontSize.getText());
      Font font = myFrame.getSystemFont();
      myFrame.setSystemFont( new Font( font.getName(), font.getStyle(), newFontSize ) );
    }
    catch( NumberFormatException e ) {
      return false;
    }
    refresh();
    return true;
  }
  private void cancel() {
    dispose();
  }
  private void refresh() {
    myTextFontSize.setBackground(Color.white);
  }
}


