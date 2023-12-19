/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */


package cseis.seaview;

import java.awt.*;
import javax.swing.*;
import cseis.general.csStandard;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

/**
 * Dialog window providing picking functionality in seismic view.<br>
 * <br>
 * Provides only rudimentary functionality as of yet.
 * @author 2011 Bjorn Olofsson
 */
public class csPickingDialog extends JDialog {
  private JFileChooser myFileChooser;

  public csPickingDialog( JFrame frame ) {
    super(frame,"Picking dialog",true);
    myFileChooser = new JFileChooser();
    super.setResizable(false);

    JTextArea area = new JTextArea();
    area.setEditable(false);
    area.setWrapStyleWord(true);
    area.setLineWrap(true);
    area.append("NOTE: Picking functionality is not implemented yet.\n");
    area.append("Click left mouse button to dump rudimentary sample information at the current mouse location to standard output (terminal from where SeaView was launched).\n");
    area.append("\nYou need to close this window before proceeding.\n");

    JPanel panelArea = new JPanel(new BorderLayout());
    panelArea.setBorder(BorderFactory.createLineBorder(Color.black));
    panelArea.add(area);

    JPanel panelInfo = new JPanel(new GridBagLayout());
    panelInfo.setBorder( BorderFactory.createCompoundBorder(
        BorderFactory.createTitledBorder("Info"),
        csStandard.INNER_EMPTY_BORDER ) );

    panelInfo.add( panelArea, new GridBagConstraints(
        0, 0, 1, 1, 1.0, 1.0, GridBagConstraints.WEST,
        GridBagConstraints.BOTH, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
//    panelInfo.add( Box.createVerticalGlue(), new GridBagConstraints(
//        0, 1, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
//        GridBagConstraints.BOTH, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    JButton bClose = new JButton("Close");

    getRootPane().setDefaultButton(bClose);

    JPanel panelButtons = new JPanel( new GridBagLayout() );
    int xp = 0;
    panelButtons.add( Box.createHorizontalGlue(), new GridBagConstraints(
        xp++, 0, 1, 1, 0.9, 0.0, GridBagConstraints.CENTER,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelButtons.add( bClose, new GridBagConstraints(
        xp++, 0, 1, 1, 0.0, 0.0, GridBagConstraints.SOUTHWEST,
        GridBagConstraints.HORIZONTAL, new Insets( 11, 0, 0, 0 ), 0, 0 ) );
    panelButtons.add( Box.createHorizontalGlue(), new GridBagConstraints(
        xp++, 0, 1, 1, 0.1, 0.0, GridBagConstraints.CENTER,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    JPanel panelAll = new JPanel(new BorderLayout());
    panelAll.setBorder( csStandard.DIALOG_BORDER );
    panelAll.add(panelInfo,BorderLayout.CENTER);
    panelAll.add(panelButtons,BorderLayout.SOUTH);

    bClose.addActionListener(new ActionListener() {
      public void actionPerformed( ActionEvent e ) {
        dispose();
      }
    });
    
    getContentPane().add(panelAll);
    setSize( new Dimension(500,250) );
    setLocationRelativeTo(frame);
  }
  public void writePick() {

  }
}


