/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.util.Timer;
import java.util.TimerTask;
import javax.swing.BorderFactory;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.border.BevelBorder;

public class csStatusBar extends JPanel {
  private static final String DEFAULT_MESSAGE = " --- ";
  private Timer myTimer;
  private JLabel myLabel;
  private long myTimerMilliseconds = 3000;
  
  public csStatusBar() {
    super( new BorderLayout() );

    myLabel = new JLabel( DEFAULT_MESSAGE, JLabel.LEFT );

    setPreferredSize( new Dimension(100, (int)(myLabel.getPreferredSize().height*1.2)) );
    setBorder( BorderFactory.createBevelBorder( BevelBorder.LOWERED ) );
    myLabel.setText("");

    add( myLabel, BorderLayout.CENTER );
  }
  public void setTimer( int milliseconds ) {
    myTimerMilliseconds = milliseconds;
  }
  public void setMessage( String message ) {
    myLabel.setText(message);
    myLabel.repaint();
    repaint();
    if( myTimer != null ) {
      myTimer.cancel();
    }
    myTimer = new Timer();
    myTimer.schedule(new RefreshStatusTask(), myTimerMilliseconds );
  }
  class RefreshStatusTask extends TimerTask {
    public void run() {
      myLabel.setText(" ");
      myLabel.repaint();
      repaint();
      myTimer.cancel(); //Terminate the timer thread
    }
  }
}


