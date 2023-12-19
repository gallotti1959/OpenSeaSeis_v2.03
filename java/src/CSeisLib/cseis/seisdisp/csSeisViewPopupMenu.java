/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */


package cseis.seisdisp;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

/**
 * Popup menu used by csSeisView.
 * @author Bjorn Olofsson
 */
@SuppressWarnings("serial")
public class csSeisViewPopupMenu extends JPopupMenu {
  protected csShowDispSettingsAction myShowDispSettingsAction;
  protected csSeisView mySeisView;
  
  public csSeisViewPopupMenu( csSeisView seisview ) {
    super();
    mySeisView = seisview;
    myShowDispSettingsAction = new csShowDispSettingsAction();
    JMenuItem itemTmp1 = new JMenuItem("Dummy1...");
    JMenuItem itemTmp2 = new JMenuItem("Dummy2...");
    itemTmp1.setEnabled(false);
    itemTmp2.setEnabled(false);

    add( itemTmp1 );
    add( itemTmp2 );
    addSeparator();
    add( myShowDispSettingsAction );
  }
  public csShowDispSettingsAction getShowDispSettingsAction() {
    return myShowDispSettingsAction;
  }
  class csShowDispSettingsAction extends AbstractAction {
    public csShowDispSettingsAction() {
      super( "Display settings..." );
      putValue( AbstractAction.SHORT_DESCRIPTION, "Set seismic display settings" );
    }
    @Override
    public void actionPerformed( ActionEvent e ) {
      mySeisView.showDispSettingsDialog();
    }
  }
}


