/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;

import java.awt.Color;
import java.util.Hashtable;
import javax.swing.event.DocumentEvent;
import javax.swing.text.*;

public class csFlowDocument extends DefaultStyledDocument {

	private Element myRootElement;
	private boolean myIsMultiLineComment;
	private MutableAttributeSet myStyleNormal;
	private MutableAttributeSet myStyleKeyword;
	private MutableAttributeSet myStyleComment;
	private MutableAttributeSet myStyleQuote;

	private MutableAttributeSet myStyleModule;
	private MutableAttributeSet myStyleNumber;
	private MutableAttributeSet myStyleValue;
	private MutableAttributeSet myStyleParam;
  private Hashtable<String,Object> myKeywords;
 
	public csFlowDocument() {
// 		doc = this;
		myRootElement = getDefaultRootElement();
		putProperty(DefaultEditorKit.EndOfLineStringProperty, "\n");

    myStyleNormal = new SimpleAttributeSet();
    StyleConstants.setFontFamily(myStyleNormal,"Monospaced");
    StyleConstants.setItalic(myStyleNormal, false);
    StyleConstants.setBold(myStyleNormal, false);
    StyleConstants.setForeground(myStyleNormal, Color.black);

    myStyleComment = new SimpleAttributeSet();
    StyleConstants.setFontFamily(myStyleComment,"Monospaced");
    StyleConstants.setItalic(myStyleComment, true);
    StyleConstants.setBold(myStyleComment, false);
    StyleConstants.setForeground(myStyleComment, new Color(153,153,153) );

    myStyleKeyword = new SimpleAttributeSet();
    StyleConstants.setFontFamily(myStyleKeyword,"Monospaced");
    StyleConstants.setItalic(myStyleKeyword, false);
    StyleConstants.setBold(myStyleKeyword, true);
    StyleConstants.setForeground(myStyleKeyword, new Color(0,0,0));
		
    myStyleQuote = new SimpleAttributeSet();
    StyleConstants.setFontFamily(myStyleQuote,"Monospaced");
    StyleConstants.setItalic(myStyleQuote, false);
    StyleConstants.setBold(myStyleQuote, false);
    StyleConstants.setForeground(myStyleQuote, Color.red);

    myStyleModule = new SimpleAttributeSet();
    StyleConstants.setFontFamily(myStyleModule,"Monospaced");
    StyleConstants.setItalic(myStyleModule, false);
    StyleConstants.setBold(myStyleModule, true);
    StyleConstants.setForeground(myStyleModule, new Color(0,0,153));

    myStyleParam = new SimpleAttributeSet();
    StyleConstants.setFontFamily(myStyleParam,"Monospaced");
    StyleConstants.setItalic(myStyleParam, false);
    StyleConstants.setBold(myStyleParam, true);
    StyleConstants.setForeground(myStyleParam, new Color(136,68,34) );

    myStyleValue = new SimpleAttributeSet();
    StyleConstants.setFontFamily(myStyleValue,"Monospaced");
    StyleConstants.setItalic(myStyleValue, false);
    StyleConstants.setBold(myStyleValue, false);
    StyleConstants.setForeground(myStyleValue, Color.black );

    myStyleNumber = new SimpleAttributeSet();
    StyleConstants.setFontFamily(myStyleNumber,"Monospaced");
    StyleConstants.setItalic(myStyleNumber, false);
    StyleConstants.setBold(myStyleNumber, false);
    StyleConstants.setForeground(myStyleNumber, new Color(00,153,00) );
  
    Object dummyObject = new Object();
    myKeywords = new Hashtable<String,Object>();
		myKeywords.put("&define", dummyObject);
		myKeywords.put("&table", dummyObject);
  }
 
	public void insertString(int offset, String str, AttributeSet a) throws BadLocationException {
		if( str.equals("(") ) {
			str = addMatchingBrace(offset);
    }
		super.insertString(offset, str, a);
		processChangedLines(offset, str.length());
	}
 
	public void remove(int offset, int length) throws BadLocationException {
		super.remove(offset, length);
		processChangedLines(offset, 0);
	}
 
	/*
	 * Determine how many lines have been changed,
	 * then apply highlighting to each line
	 */
	private void processChangedLines(int offset, int length) throws BadLocationException {
		String content = getText(0, getLength());
		// The lines affected by the latest document update
		int startLine = myRootElement.getElementIndex(offset);
		int endLine = myRootElement.getElementIndex(offset + length);
		// Make sure all comment lines prior to the start line are commented
		// and determine if the start line is still in a multi line comment
		// setMultiLineComment(commentLinesBefore(content, startLine));
		// Do the actual highlighting
		for( int i = startLine; i <= endLine; i++ ) {
			applyHighlighting(content, i);
    }
		// Resolve highlighting to the next end multi line delimiter
		if( isMultiLineComment() ) {
			commentLinesAfter(content, endLine);
    }
    else {
			highlightLinesAfter(content, endLine);
    }
	}
 
	/*
	 * Highlight lines when a multi line comment is still 'open'
	 * (ie. matching end delimiter has not yet been encountered)
	 */
	private boolean commentLinesBefore(String content, int line) {
		int offset = myRootElement.getElement(line).getStartOffset();
		// Start of comment not found, nothing to do
		int startDelimiter = lastIndexOf(content, getStartDelimiter(), offset - 2);
		if( startDelimiter < 0 ) return false;
		// Matching start/end of comment found, nothing to do
		int endDelimiter = indexOf(content, getEndDelimiter(), startDelimiter);
		if( endDelimiter < offset & endDelimiter != -1 ) return false;
		// End of comment not found, highlight the lines
		setCharacterAttributes(startDelimiter, offset - startDelimiter + 1, myStyleComment, false);
		return true;
	}
 
	/*
	 * Highlight comment lines to matching end delimiter
	 */
	private void commentLinesAfter(String content, int line) {
		int offset = myRootElement.getElement(line).getEndOffset();
		// End of comment not found, nothing to do
		int endDelimiter = indexOf(content, getEndDelimiter(), offset);
		if( endDelimiter < 0 ) return;
		// Matching start/end of comment found, comment the lines
		int startDelimiter = lastIndexOf(content, getStartDelimiter(), endDelimiter);
		if( startDelimiter < 0 || startDelimiter <= offset ) {
			setCharacterAttributes(offset, endDelimiter - offset + 1, myStyleComment, false);
		}
	}
 
	/*
	 * Highlight lines to start or end delimiter
	 */
	private void highlightLinesAfter(String content, int line) throws BadLocationException {
		int offset = myRootElement.getElement(line).getEndOffset();
		// Start/End delimiter not found, nothing to do
		int startDelimiter = indexOf(content, getStartDelimiter(), offset);
		int endDelimiter = indexOf(content, getEndDelimiter(), offset);
		if( startDelimiter < 0 ) startDelimiter = content.length();
		if( endDelimiter < 0 ) endDelimiter = content.length();
		int delimiter = Math.min(startDelimiter, endDelimiter);
		if( delimiter < offset ) return;
		// Start/End delimiter found, reapply highlighting
		int endLine = myRootElement.getElementIndex(delimiter);
		for( int i = line + 1; i < endLine; i++ ) {
			Element branch = myRootElement.getElement(i);
			Element leaf = getCharacterElement(branch.getStartOffset());
			AttributeSet as = leaf.getAttributes();
			if( as.isEqual(myStyleComment) ) {
				applyHighlighting(content, i);
      }
		}
	}
 
	/*
	 * Parse the line to determine the appropriate highlighting
	 */
	private void applyHighlighting(String content, int line) throws BadLocationException {
		int startOffset = myRootElement.getElement(line).getStartOffset();
		int endOffset = myRootElement.getElement(line).getEndOffset() - 1;
		int lineLength = endOffset - startOffset;
		int contentLength = content.length();
		if (endOffset >= contentLength)
			endOffset = contentLength - 1;
		// check for multi line comments
		// (always set the comment attribute for the entire line)
/*
    if (endingMultiLineComment(content, startOffset, endOffset)
				|| isMultiLineComment()
				|| startingMultiLineComment(content, startOffset, endOffset)) {
			doc.setCharacterAttributes(startOffset,
					endOffset - startOffset + 1, comment, false);
			return;
		}
    */
		// set normal attributes for the line
		setCharacterAttributes(startOffset, lineLength, myStyleNormal, true);
		// check for single line comment
		int index = content.indexOf(getSingleLineDelimiter(), startOffset);
		if( (index > -1) && (index < endOffset) ) {
			setCharacterAttributes(index, endOffset - index + 1, myStyleComment, false);
			endOffset = index - 1;
		}
    formatText(content, startOffset, endOffset);
		// check for tokens
		checkForTokens(content, startOffset, endOffset);
	}
 
	/*
	 * Does this line contain the start delimiter
	 */
	private boolean startingMultiLineComment(String content, int startOffset, int endOffset) throws BadLocationException {
		int index = indexOf(content, getStartDelimiter(), startOffset);
		if( (index < 0) || (index > endOffset) ) {
			return false;
    }
		else {
			setMultiLineComment(true);
			return true;
		}
	}
 
	/*
	 * Does this line contain the end delimiter
	 */
	private boolean endingMultiLineComment(String content, int startOffset, int endOffset) throws BadLocationException {
		int index = indexOf(content, getEndDelimiter(), startOffset);
		if( (index < 0) || (index > endOffset) ) {
			return false;
    }
		else {
			setMultiLineComment(false);
			return true;
		}
	}
 
	/*
	 * We have found a start delimiter
	 * and are still searching for the end delimiter
	 */
	private boolean isMultiLineComment() {
		return myIsMultiLineComment;
	}
 
	private void setMultiLineComment(boolean value) {
		myIsMultiLineComment = value;
	}
 
	/*
	 * Parse the line for tokens to highlight
	 */
	private void checkForTokens(String content, int startOffset, int endOffset) {
		while( startOffset <= endOffset ) {
			// skip the delimiters to find the start of a new token
			while( isDelimiter(content.substring(startOffset, startOffset + 1)) ) {
				if( startOffset < endOffset ) {
					startOffset++;
        }
        else {
					return;
        }
			}
			// Extract and process the entire token
			if( isQuoteDelimiter(content.substring(startOffset, startOffset + 1)) ) {
				startOffset = getQuoteToken(content, startOffset, endOffset);
      }
      else {
				startOffset = getOtherToken(content, startOffset, endOffset);
      }
		}
	}  
  
	/*
	 * Parse the line to get the quotes and highlight it
	 */
	private int getQuoteToken(String content, int startOffset, int endOffset) {
		String quoteDelimiter = content.substring(startOffset, startOffset + 1);
		String escapeString = getEscapeString(quoteDelimiter);
		int index;
		int endOfQuote = startOffset;
		// skip over the escape quotes in this quote
		index = content.indexOf(escapeString, endOfQuote + 1);
		while( (index > -1) && (index < endOffset) ) {
			endOfQuote = index + 1;
			index = content.indexOf(escapeString, endOfQuote);
		}
		// now find the matching delimiter
		index = content.indexOf(quoteDelimiter, endOfQuote + 1);
		if( (index < 0) || (index > endOffset) ) {
			endOfQuote = endOffset;
    }
    else {
			endOfQuote = index;
    }
		setCharacterAttributes(startOffset, endOfQuote - startOffset + 1, myStyleQuote, false);
		return endOfQuote + 1;
	}
 
	private int getOtherToken(String content, int startOffset, int endOffset) {
		int endOfToken = startOffset + 1;
		while( endOfToken <= endOffset ) {
			if( isDelimiter(content.substring(endOfToken, endOfToken + 1)) ) break;
			endOfToken++;
		}
		String token = content.substring(startOffset, endOfToken);
		if( isKeyword(token) ) {
			setCharacterAttributes(startOffset, endOfToken - startOffset,	myStyleKeyword, false);
    }
		return endOfToken + 1;
	}
 
//-----------------------------------------------------------------------------
  public void formatText( String text, int startOffset, int endOffset ) {
    if( startOffset > endOffset || text.length() < endOffset ) return;
    int counter = startOffset;
    while( counter < endOffset ) {
      int offset = counter;
      // Skip leading spaces
      while( counter < endOffset && text.charAt(counter) == ' ' ) {
        counter += 1;
      }
      char letter = text.charAt(counter);
      if( letter == '$' ) {
        while( counter < endOffset && text.charAt(counter) != '\n' ) {
          counter += 1;
        }
        setCharacterAttributes(offset, counter-offset, myStyleModule, true );
      }
      else if( letter == '#' ) {
        while( counter < endOffset && text.charAt(counter) != '\n' ) {
          counter += 1;
        }
        setCharacterAttributes(offset, counter-offset, myStyleComment, true );
      }
      else if( letter == '\n' ) {
        counter += 1;
        setCharacterAttributes(offset, counter-offset, myStyleNormal, true );
      }
      else {
        while( counter < endOffset && text.charAt(counter) != ' ' ) {
          counter += 1;
        }
        setCharacterAttributes(offset, counter-offset, myStyleParam, true );
        offset = counter;
        if( counter < endOffset ) {
          while( counter <= endOffset && text.charAt(counter) != '\n' ) {
            counter += 1;
          }
          setCharacterAttributes(offset, counter-offset, myStyleValue, true );
        }
      }
    } // END while
    counter = startOffset;
    boolean isNumber = false;
    boolean isPreviousNumberDelimiter = false;
    int numberStart = 0;
    while( counter <= endOffset ) {
      char letter = text.charAt(counter);
      if( (letter == '-' && counter == startOffset) || Character.isDigit(letter) || (isNumber && letter == '.') ) {
        if( !isNumber && isPreviousNumberDelimiter ) {
          numberStart = counter;
          isNumber = true;
        }
      }
      else if( isNumber ) {
        setCharacterAttributes(numberStart, counter-numberStart, myStyleNumber, true );
        isNumber = false;
      }
      isPreviousNumberDelimiter = isNumberDelimiter(letter+"");
      counter += 1;
    } // END while
  }  
  
  
	/*
	 * This updates the colored text and prepares for undo event
	 */
	protected void fireInsertUpdate(DocumentEvent evt) {
 		super.fireInsertUpdate(evt);
 		try {
			processChangedLines(evt.getOffset(), evt.getLength());
		}
    catch (BadLocationException ex) {
			System.out.println("" + ex);
		}
	}
 
	/*
	 * This updates the colored text and does the undo operation
	 */
	protected void fireRemoveUpdate(DocumentEvent evt) {
 		super.fireRemoveUpdate(evt);
 		try {
			processChangedLines(evt.getOffset(), evt.getLength());
		}
    catch (BadLocationException ex) {
			System.out.println("" + ex);
		}
	}
 
	/*
	 * Assume the needle will the found at the start/end of the line
	 */
	private int indexOf(String content, String needle, int offset) {
		int index;
		while( (index = content.indexOf(needle, offset)) != -1 ) {
			String text = getLine(content, index).trim();
			if( text.startsWith(needle) || text.endsWith(needle) ) {
				break;
      }
      else {
				offset = index + 1;
      }
		}
		return index;
	}
 
	/*
	 * Assume the needle will the found at the start/end of the line
	 */
	private int lastIndexOf(String content, String needle, int offset) {
		int index;
		while( (index = content.lastIndexOf(needle, offset)) != -1 ) {
			String text = getLine(content, index).trim();
			if( text.startsWith(needle) || text.endsWith(needle) ) {
				break;
      }
      else {
				offset = index - 1;
      }
		}
		return index;
	}
 
	private String getLine(String content, int offset) {
		int line = myRootElement.getElementIndex(offset);
		Element lineElement = myRootElement.getElement(line);
		int start = lineElement.getStartOffset();
		int end = lineElement.getEndOffset();
		return content.substring(start, end - 1);
	}
 
	/*
	 * Override for other languages
	 */
	protected boolean isDelimiter(String character) {
		String operands = ";:{}()[]+-/%<=>!|^~*";
		if( Character.isWhitespace(character.charAt(0)) || operands.indexOf(character) != -1 ) {
			return true;
    }
    else {
			return false;
    }
	}

	protected boolean isNumberDelimiter(String character) {
		String operands = "-,;:{}()[]/%<=>!|^\"'\n";
		if( Character.isWhitespace(character.charAt(0)) || operands.indexOf(character) != -1 ) {
			return true;
    }
    else {
			return false;
    }
	}
  
	protected boolean isQuoteDelimiter(String character) {
		String quoteDelimiters = "\"'";
		if( quoteDelimiters.indexOf(character) < 0 ) {
			return false;
    }
    else {
			return true;
    }
	}
 
	protected boolean isKeyword(String token) {
		Object o = myKeywords.get(token.toUpperCase());
		return( o == null ? false : true );
	}
 
	protected String getStartDelimiter() {
		return "/*";
	}
 
	protected String getEndDelimiter() {
		return "*/";
	}
 
	protected String getSingleLineDelimiter() {
		return "#";
	}
 
	protected String getEscapeString(String quoteDelimiter) {
		return "\\" + quoteDelimiter;
	}
 
	protected String addMatchingBrace(int offset) throws BadLocationException {
		StringBuffer whiteSpace = new StringBuffer();
		int line = myRootElement.getElementIndex(offset);
		int i = myRootElement.getElement(line).getStartOffset();
		while( true ) {
			String temp = getText(i, 1);
			if( temp.equals(" ") || temp.equals("\t") ) {
				whiteSpace.append(temp);
				i++;
			}
      else {
				break;
      }
		}
		return "(\n" + whiteSpace.toString() + whiteSpace.toString() + "\n" + whiteSpace.toString() + ")";
	}
}


