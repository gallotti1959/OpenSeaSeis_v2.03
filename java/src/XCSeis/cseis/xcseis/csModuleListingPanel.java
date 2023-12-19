/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;

import javax.swing.*;
import javax.swing.text.*;
import javax.swing.text.html.*;
import cseis.jni.csNativeModuleHelp;

public class csModuleListingPanel extends JScrollPane {
  private JTextPane myTextPane;

  public csModuleListingPanel() {
    super( JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS );
    myTextPane = new JTextPane();
    myTextPane.setEditable(false);
    myTextPane.setEditorKit(new HTMLEditorKit());
    csNativeModuleHelp help = new csNativeModuleHelp();
    myTextPane.setText("<html>" + help.moduleHtmlListing() + "</html>");
    MutableAttributeSet attrs = myTextPane.getInputAttributes();
    int size = StyleConstants.getFontSize(attrs);
    StyleConstants.setFontSize(attrs, size - 1);
    StyledDocument doc = myTextPane.getStyledDocument();
    doc.setCharacterAttributes(0, doc.getLength() + 1, attrs, false);
    myTextPane.setCaretPosition(0);
    super.setViewportView( myTextPane );
  }
}


