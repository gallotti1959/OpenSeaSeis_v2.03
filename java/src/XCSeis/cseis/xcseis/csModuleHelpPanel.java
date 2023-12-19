/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;

import javax.swing.*;
import javax.swing.text.*;
import javax.swing.text.html.*;

public class csModuleHelpPanel extends JScrollPane {
  private JTextPane myTextPane;

  public csModuleHelpPanel() {
    super( JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS );
    myTextPane     = new JTextPane();
    myTextPane.setEditable(false);
    myTextPane.setEditorKit(new HTMLEditorKit());
    super.setViewportView( myTextPane );
  }
  public void updateModule( String moduleName ) {
    myTextPane.setText("<html> <h2>Documentation for " + moduleName + "</h2> <p> ... <p> ... <p> ...</html>");
  }
  public void updateModuleHelp( String text ) {
    myTextPane.setText("<html>" + text + "</html>");
    MutableAttributeSet attrs = myTextPane.getInputAttributes();
    int size = StyleConstants.getFontSize(attrs);
    StyleConstants.setFontSize(attrs, size - 1);
    StyledDocument doc = myTextPane.getStyledDocument();
    doc.setCharacterAttributes(0, doc.getLength() + 1, attrs, false);
    myTextPane.setCaretPosition(0);
  }
}


