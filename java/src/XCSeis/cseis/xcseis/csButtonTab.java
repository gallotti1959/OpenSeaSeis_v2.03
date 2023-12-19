/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;
 
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.event.*;
import javax.swing.*;
 
public class csButtonTab extends JPanel {
  private final JTabbedPane myParentPane;
  private final XCSeis myFrame;
 
  public csButtonTab( XCSeis frame, JTabbedPane parent ) {
    this( frame, parent, "Close tab" );
  }
  public csButtonTab( XCSeis frame, JTabbedPane parent, String toolTip ) {
    super( new BorderLayout() );
    myParentPane = parent;
    myFrame = frame;
    
    JButton button = new CloseButton(toolTip);
    JLabel label = new JLabel() {
      @Override
      public String getText() {
        int index = myParentPane.indexOfTabComponent( csButtonTab.this );
        if( index >= 0 ) {
          return myParentPane.getTitleAt(index);
        }
        return null;
      }
    };
    label.setBorder( BorderFactory.createEmptyBorder(0, 0, 0, 5) );
    
    setOpaque(false);

    add(label, BorderLayout.CENTER);
    add(button, BorderLayout.EAST);
  }
  private class CloseButton extends JButton implements ActionListener {
    public CloseButton( String toolTip ) {
      setText("x");
      setToolTipText(toolTip);
      setBorder(BorderFactory.createCompoundBorder(
              BorderFactory.createLineBorder(Color.lightGray),
              BorderFactory.createEmptyBorder(0,5,1,4)) );
      setBorderPainted(false);
      setContentAreaFilled(false);
      setFocusable(false);
      addMouseListener(theMouseListener);
      setRolloverEnabled(true);
      addActionListener(this);
    }
    public void actionPerformed(ActionEvent e) {
      myFrame.closeFlow( myParentPane.indexOfTabComponent(csButtonTab.this) );
    }
  }

  private final static MouseListener theMouseListener = new MouseAdapter() {
    public void mouseEntered(MouseEvent e) {
      Component comp = e.getComponent();
      if( comp instanceof AbstractButton ) {
        ((AbstractButton)comp).setBorderPainted(true);
      }
    }
    public void mouseExited(MouseEvent e) {
      Component comp = e.getComponent();
      if( comp instanceof AbstractButton ) {
        ((AbstractButton)comp).setBorderPainted(false);
      }
    }
  };
}


