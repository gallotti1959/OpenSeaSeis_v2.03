/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;

import javax.swing.*;

public class csModuleParamPanel extends JScrollPane {
  private JTextPane myTextPane;

  public csModuleParamPanel() {
    super( JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS );
    myTextPane     = new JTextPane();
    myTextPane.setEditable(false);
    super.setViewportView( myTextPane );
  }
  public void updateModule( String moduleName ) {
    myTextPane.setText("Example for " + moduleName);
  }
  public void updateModuleExample( String text ) {
    myTextPane.setText(text + "\n ...place holder...");
    myTextPane.setCaretPosition(0);
  }
}


