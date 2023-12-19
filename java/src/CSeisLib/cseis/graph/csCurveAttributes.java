/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */


package cseis.graph;

import java.awt.Color;

/**
 * Attributes defining style of 2D curve.
 * @author Bjorn Olofsson
 */
public class csCurveAttributes {
  public static final int POINT_TYPE_NONE   = -1;
  public static final int POINT_TYPE_CIRCLE = 0;
  public static final int POINT_TYPE_CROSS  = 1;
  public static final int POINT_TYPE_SQUARE = 2;
  public static final int LINE_TYPE_NONE   = -101;
  public static final int LINE_TYPE_SOLID   = 100;
  public static final int LINE_TYPE_DASH    = 101;
  public static final int LINE_TYPE_DOT     = 102;
  public static final int FILLED_TYPE_NONE  = -201;
  public static final int FILLED_TYPE_ZERO  = 200;
  public static final int FILLED_TYPE_MIN   = 201;

  public static final int Y_AXIS_LEFT   = 11;
  public static final int Y_AXIS_RIGHT  = 12;
  public static final int X_AXIS_TOP    = 13;
  public static final int X_AXIS_BOTTOM = 14;

  private static int staticCurveCounter = 1;

  public String name;
  public int pointType;
  public int lineType;
  public int filledType;
  public int pointSize;
  public int lineSize;
  public Color pointColor;
  public Color lineColor;
  public Color fillColor;
  public int axisXType;
  public int axisYType;

  private float myMinX;
  private float myMaxX;
  private float myMinY;
  private float myMaxY;
  private float myMaxTotal;
  private boolean myIsMinMaxSet;

  public csCurveAttributes() {
    this( "Curve #" + csCurveAttributes.staticCurveCounter );
  }
  public csCurveAttributes( String name_in ) {
    csCurveAttributes.staticCurveCounter += 1;
    name    = name_in;
//    pointType = POINT_TYPE_NONE;
//    filledType = FILLED_TYPE_NONE;
    pointType  = POINT_TYPE_SQUARE;
    lineType   = LINE_TYPE_SOLID;
    filledType = FILLED_TYPE_MIN;
    pointSize  = 10;
    lineSize   = 1;
    pointColor = Color.red;
    lineColor  = Color.black;
    fillColor  = Color.green;
    axisXType  = X_AXIS_BOTTOM;
    axisYType  = Y_AXIS_LEFT;

    // Set private members:
    myMinX = 0;
    myMaxX = 0;
    myMinY = 0;
    myMaxY = 0;
    myMaxTotal = 0;
    myIsMinMaxSet = false;
  }
  public boolean isMinMaxSet() {
    return myIsMinMaxSet;
  }
  public float maxTotal() { return myMaxTotal; }
  public float minX() { return myMinX; }
  public float maxX() { return myMaxX; }
  public float minY() { return myMinY; }
  public float maxY() { return myMaxY; }
  public void setMinMax( float minX, float maxX, float minY, float maxY ) {
    myIsMinMaxSet = true;
    myMinX = minX;
    myMaxX = maxX;
    myMinY = minY;
    myMaxY = maxY;
    myMaxTotal = Math.max(
        Math.max( Math.abs(myMaxX), Math.abs(myMaxY)),
        Math.max( Math.abs(myMinX), Math.abs(myMinY)) );
  }
}


