/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Font;
import java.awt.GridLayout;
import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.JPanel;

public class csSubPanel extends JPanel {
  public static final int STATUS_NONE    = 0;
  public static final int STATUS_OK      = 1;
  public static final int STATUS_PENDING = 2;
  public static final int STATUS_ERROR   = 3;
  public static Color[] STATUS_COLORS = {
    new JLabel("").getBackground(),
    new Color(128,255,128),
    new Color(255,175,50),
    new Color(255,128,128),
  };
  
  private JPanel myTitleBar;
  private JLabel myTitle;
  private JLabel myInfoLabel;

  public csSubPanel( String title ) {
    super( new BorderLayout() );
    myTitle = new JLabel(title);
    myInfoLabel = new JLabel(" ",JLabel.CENTER);
    java.awt.Font font = myInfoLabel.getFont();
    myInfoLabel.setFont( new Font("SansSerif",Font.PLAIN,font.getSize()-2) );
    myInfoLabel.setOpaque(true);
    myTitleBar = new JPanel( new GridLayout(1, 2) );
    myTitleBar.add(myTitle);
    myTitleBar.add(myInfoLabel);
    
    add( myTitleBar, BorderLayout.NORTH );
  }
  public void setInfo( String info ) {
    setInfo( info, csSubPanel.STATUS_NONE );
  }
  public void setInfo( String info, int status ) {
    if( info != null && info.length() > 0 ) {
      myInfoLabel.setBorder( BorderFactory.createCompoundBorder(
              BorderFactory.createLineBorder(getBackground()), BorderFactory.createLineBorder(Color.darkGray)));
      myInfoLabel.setText( info );
      myInfoLabel.setToolTipText( info );
    }
    else {
      myInfoLabel.setBorder( null );
      myInfoLabel.setText(" ");
      myInfoLabel.setToolTipText(null);
    }
    myInfoLabel.setBackground(STATUS_COLORS[status]);
  }
}


