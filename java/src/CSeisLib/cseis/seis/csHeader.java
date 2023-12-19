/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */


package cseis.seis;

import cseis.jni.csJNIDef;

/**
 * Header information
 * @author Bjorn Olofsson
 */
public class csHeader {
  private Number myNumber = null;
  private String myText   = null;
  private int myType;
  
  public csHeader() {
    myType   = csJNIDef.TYPE_DOUBLE;
    myNumber = new Double(0);
    myText   = "";
  }
  public csHeader( csHeader hdr ) {
    myType   = hdr.myType;
    switch( hdr.myType ) {
      case csJNIDef.TYPE_INT:
        myNumber = new Integer(hdr.intValue());
        break;
      case csJNIDef.TYPE_LONG:
        myNumber = new Long(hdr.longValue());
        break;
      case csJNIDef.TYPE_FLOAT:
        myNumber = new Double(hdr.floatValue());
        break;
      case csJNIDef.TYPE_DOUBLE:
        myNumber = new Double(hdr.doubleValue());
        break;
      default:
        myNumber = new Integer(0);
    }
    myText   = hdr.myText;
  }
  public csHeader( int value ) {
    myType   = csJNIDef.TYPE_INT;
    myNumber = new Integer(value);
  }
  public csHeader( long value ) {
    myType   = csJNIDef.TYPE_LONG;
    myNumber = new Long(value);
  }
  public csHeader( String value ) {
    myType   = csJNIDef.TYPE_STRING;
    myText   = value;
  }
  public void setValue( Number value ) {
    myNumber = value;
  }
  public void setValue( int value ) {
    myNumber = new Integer(value);
    myType   = csJNIDef.TYPE_INT;
    myText   = "";
  }
  public void setValue( long value ) {
    myNumber = new Long(value);
    myType   = csJNIDef.TYPE_LONG;
    myText   = "";
  }
  public void setValue( float value ) {
    myNumber = new Float(value);
    myType   = csJNIDef.TYPE_FLOAT;
    myText   = "";
  }
  public void setValue( double value ) {
    myNumber = new Double(value);
    myType   = csJNIDef.TYPE_DOUBLE;
    myText   = "";
  }
  public void setValue( String value ) {
    myText = value;
    myType = csJNIDef.TYPE_STRING;
    myNumber = new Double(0);
  }

  public Object value() {
    if( myType == csJNIDef.TYPE_STRING ) {
      return myText;
    }
    else {
      return myNumber;
    }
  }
  public int intValue() {
    return myNumber.intValue();
  }
  public long longValue() {
    return myNumber.longValue();
  }
  public float floatValue() {
    return myNumber.floatValue();
  }
  public double doubleValue() {
    return myNumber.doubleValue();
  }
  
  public String stringValue() {
	  return myText;
  }
  public int type() {
    return myType;
  }
  @Override
  public boolean equals( Object obj ) {
    return( obj instanceof csHeader && ((csHeader)obj).value().equals(myNumber) );
  }
  @Override
  public String toString() {
    if( myType == csJNIDef.TYPE_STRING ) {
      return myText;
    }
    else {
      return myNumber.toString();
    }
  }
}


