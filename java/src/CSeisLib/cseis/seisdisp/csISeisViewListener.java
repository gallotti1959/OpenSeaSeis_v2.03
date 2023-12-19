/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */


package cseis.seisdisp;
import java.awt.Dimension;

/**
 * Seismic view listener
 * @author Bjorn Olofsson
 */
public interface csISeisViewListener {
  public void changedSettings( csSeisDispSettings settings );
  public void vertScrollChanged( int scrollValue );
  public void horzScrollChanged( int scrollValue );
  public void sizeChanged( Dimension size );
}


