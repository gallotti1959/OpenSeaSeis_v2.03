/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */


package cseis.graph;

import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

import javax.swing.*;

import cseis.general.csStandard;

/**
 * Dialog where graph settings can be changed.
 * @author Bjorn Olofsson
 */
 // ...work in progress
public class csGraphDispDialog extends JDialog {
  private static final int TEXT_MIN_WIDTH = 80;
  
  protected csGraphAttributes myAttr;
  
  // Text fields
  private JTextField myTextInsetLeft;
  private JTextField myTextInsetTop;
  private JTextField myTextInsetRight;
  private JTextField myTextInsetBottom;
  private JTextField myTextMinAxisX;
  private JTextField myTextMaxAxisX;
  private JTextField myTextMinAxisY;
  private JTextField myTextMaxAxisY;

  private JTextField myBorderSize;
  private JTextField myBorderPadding;
  private JTextField myInnerBorderSize;
  private JTextField myInnerBorderPadding;

  private JTextField myTextGridIncX;
  private JTextField myTextGridIncY;
  private JTextField myTextMinorRatioX;
  private JTextField myTextMinorRatioY;

  private JTextField myGraphTitle;
  private JTextField myXAxisLabel;
  private JTextField myYAxisLabel;
  private JTextField myTextRefValueY;

  private JCheckBox myAutoScaleAxes;
  private JCheckBox myCentreAxisX;
  private JCheckBox myCentreAxisY;

  private JCheckBox myShowBorder;
  private JCheckBox myShowInnerBorder;
  private JCheckBox myShowGrid;
  private JCheckBox myShowAxisX;
  private JCheckBox myShowAxisY;
  private JCheckBox myShowAxisZero;
  private JCheckBox myShowXTics;
  private JCheckBox myShowYTics;
  private JCheckBox myShowAnnotationX;
  private JCheckBox myShowAnnotationY;

  private JRadioButton myButtonYLinearScale;
  private JRadioButton myButtonYLogScale;
  private JRadioButton myButtonXLinearScale;
  private JRadioButton myButtonXLogScale;
  private JLabel myLabelXAxisScale;
  private JLabel myLabelYAxisScale;

  private JCheckBox myIsTrueScale;
  private JCheckBox myUseRefValueY;

  private JLabel myLabelInsetLeft;
  private JLabel myLabelInsetTop;
  private JLabel myLabelInsetRight;
  private JLabel myLabelInsetBottom;
  private JLabel myLabelMinAxisX;
  private JLabel myLabelMaxAxisX;
  private JLabel myLabelMinAxisY;
  private JLabel myLabelMaxAxisY;
  private JLabel myLabelGridIncX;
  private JLabel myLabelGridIncY;
  private JLabel myLabelMinorRatioX;
  private JLabel myLabelMinorRatioY;

  private JLabel myLabelBorderSize;
  private JLabel myLabelBorderPadding;
  private JLabel myLabelInnerBorderSize;
  private JLabel myLabelInnerBorderPadding;

  private JLabel myLabelGraphTitle;
  private JLabel myLabelXAxisLabel;
  private JLabel myLabelYAxisLabel;
  
  private csGraph2D myGraph;
  private boolean myIsUpdating;

  //----------------------
  private JButton myButtonOK;
  private JButton myButtonApply;
  private JButton myButtonCancel;
  
  public csGraphDispDialog( csGraph2D graph ) {
    super.setTitle( "Graph settings" );
    super.setModal( false );
    
    myGraph = graph;
    myAttr = myGraph.getGraphAttributes();
    myIsUpdating = false;
    
    // Text fields
    myTextInsetLeft = new JTextField(" ");
    myTextGridIncX = new JTextField(" ");
    myTextInsetTop = new JTextField(" ");
    myTextInsetRight = new JTextField(" ");
    myTextInsetBottom = new JTextField(" ");
    myTextMinAxisX = new JTextField(" ");
    myTextMaxAxisX = new JTextField(" ");
    myTextMinAxisY = new JTextField(" ");
    myTextMaxAxisY = new JTextField(" ");

    myTextGridIncY = new JTextField(" ");
    myTextMinorRatioX = new JTextField(" ");
    myTextMinorRatioY = new JTextField(" ");

    myInnerBorderSize = new JTextField(" ");
    myInnerBorderPadding = new JTextField(" ");
    myBorderSize = new JTextField(" ");
    myBorderPadding = new JTextField(" ");

    myGraphTitle = new JTextField(" ");
    myXAxisLabel = new JTextField(" ");
    myYAxisLabel = new JTextField(" ");
    myTextRefValueY = new JTextField(" ");

    int height = myTextInsetLeft.getPreferredSize().height;
    myTextInsetLeft.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myTextGridIncX.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );

    myTextInsetTop.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myTextInsetRight.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myTextInsetBottom.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myTextMinAxisX.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myTextMinAxisY.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myTextMaxAxisX.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myTextMaxAxisY.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myTextGridIncY.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myTextMinorRatioY.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myTextMinorRatioX.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myBorderSize.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myBorderPadding.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myInnerBorderSize.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myInnerBorderPadding.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myGraphTitle.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myXAxisLabel.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myYAxisLabel.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );
    myTextRefValueY.setPreferredSize( new Dimension( TEXT_MIN_WIDTH, height ) );

    myAutoScaleAxes   = new JCheckBox("Auto scale",myAttr.autoScaleAxes);
    myCentreAxisX     = new JCheckBox("Centre X axis",myAttr.centreAxisX);
    myCentreAxisY     = new JCheckBox("Centre Y axis",myAttr.centreAxisY);
    myShowAxisX       = new JCheckBox("Show X axis",myAttr.showAxisX);
    myShowAxisY       = new JCheckBox("Show Y axis",myAttr.showAxisY);
    myShowAxisZero    = new JCheckBox("Show zero axis",myAttr.showZeroAxis);
    myShowXTics       = new JCheckBox("Show X tics",myAttr.showXTics);
    myShowYTics       = new JCheckBox("Show Y tics",myAttr.showYTics);
    myShowAnnotationX = new JCheckBox("Show X axis annotation",myAttr.showAxisXAnnotation);
    myShowAnnotationY = new JCheckBox("Show Y axis annotation",myAttr.showAxisYAnnotation);
    myShowBorder      = new JCheckBox("Show border",myAttr.showBorder);
    myShowInnerBorder = new JCheckBox("Show inner border",myAttr.showInnerBorder);
    myShowGrid        = new JCheckBox("Show grid",myAttr.showGrid);

    myButtonYLinearScale  = new JRadioButton("Linear",myAttr.axisScaleY == csGraphAttributes.AXIS_SCALE_LINEAR);
    myButtonYLogScale  = new JRadioButton("Logarithmic",myAttr.axisScaleY == csGraphAttributes.AXIS_SCALE_LOG);
    myButtonXLinearScale  = new JRadioButton("Linear",myAttr.axisScaleX == csGraphAttributes.AXIS_SCALE_LINEAR);
    myButtonXLogScale  = new JRadioButton("Logarithmic",myAttr.axisScaleY == csGraphAttributes.AXIS_SCALE_LOG);
    myLabelXAxisScale = new JLabel("X axis");
    myLabelYAxisScale = new JLabel("Y axis");
    ButtonGroup groupXAxis = new ButtonGroup();
    groupXAxis.add(myButtonXLinearScale);
    groupXAxis.add(myButtonXLogScale);
    ButtonGroup groupYAxis = new ButtonGroup();
    groupYAxis.add(myButtonYLinearScale);
    groupYAxis.add(myButtonYLogScale);

    myIsTrueScale = new JCheckBox("True scale",myAttr.isTrueScale);
    myUseRefValueY = new JCheckBox("Use ref value",myAttr.useRefValueY);

    // Labels
    myLabelInsetLeft   = new JLabel("Left inset");
    myLabelInsetTop    = new JLabel("Top inset");
    myLabelInsetRight  = new JLabel("Right inset");
    myLabelInsetBottom = new JLabel("Bottom inset");
    myLabelMinAxisX = new JLabel("Min X value");
    myLabelMaxAxisX = new JLabel("Max X value");
    myLabelMinAxisY = new JLabel("Min Y value");
    myLabelMaxAxisY = new JLabel("Max Y value");

    myLabelGridIncX = new JLabel("Grid inc X (pixels)");
    myLabelGridIncY = new JLabel("Grid inc Y (pixels)");
    myLabelMinorRatioX = new JLabel("Min/maj tic ratio X");
    myLabelMinorRatioY = new JLabel("Min/maj tic ratio Y");

    myLabelBorderSize = new JLabel("Border size");
    myLabelBorderPadding = new JLabel("Border padding");
    myLabelInnerBorderSize = new JLabel("Inner border size");
    myLabelInnerBorderPadding = new JLabel("Inner border padding");

    myLabelGraphTitle = new JLabel("Title");
    myLabelXAxisLabel = new JLabel("X label");
    myLabelYAxisLabel = new JLabel("Y label");

    //=============================================================================
    // Creating panels
    //

    JPanel panelCheck = new JPanel(new GridBagLayout());
    panelCheck.setBorder( BorderFactory.createCompoundBorder(
        BorderFactory.createTitledBorder("Settings 1"),
        csStandard.INNER_EMPTY_BORDER ) );

    JPanel panelText = new JPanel(new GridBagLayout());
    panelText.setBorder( BorderFactory.createCompoundBorder(
        BorderFactory.createTitledBorder("Settings 2"),
        csStandard.INNER_EMPTY_BORDER ) );

    JPanel panelOther = new JPanel(new GridBagLayout());
    panelOther.setBorder( BorderFactory.createCompoundBorder(
        BorderFactory.createTitledBorder("Settings 3"),
        csStandard.INNER_EMPTY_BORDER ) );

    //--------------------------------------------------------------
    int yp = 0;

    panelText.add( myLabelInsetLeft, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextInsetLeft, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelInsetRight, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextInsetRight, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelInsetTop, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextInsetTop, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelInsetBottom, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextInsetBottom, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
/*
    panelText.add( myLabelMinAxisX, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextMinAxisX, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelMaxAxisX, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextMaxAxisX, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelMinAxisY, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextMinAxisY, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelMaxAxisY, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextMaxAxisY, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
*/
    panelText.add( myLabelGridIncX, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextGridIncX, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelGridIncY, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextGridIncY, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );


    panelText.add( myLabelMinorRatioX, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextMinorRatioX, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelMinorRatioY, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myTextMinorRatioY, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelBorderSize, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myBorderSize, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelBorderPadding, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myBorderPadding, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelInnerBorderSize, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myInnerBorderSize, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelInnerBorderPadding, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myInnerBorderPadding, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelGraphTitle, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myGraphTitle, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelXAxisLabel, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myXAxisLabel, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( myLabelYAxisLabel, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelText.add( myYAxisLabel, new GridBagConstraints(
        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelText.add( Box.createVerticalGlue(), new GridBagConstraints(
        0, yp++, 2, 1, 0.0, 1.0, GridBagConstraints.WEST,
        GridBagConstraints.VERTICAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

//----------------------------------------------------------------------
    yp = 0;
    panelCheck.add( myShowAxisX, new GridBagConstraints(
        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelCheck.add( myShowAxisY, new GridBagConstraints(
        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelCheck.add( myShowXTics, new GridBagConstraints(
        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelCheck.add( myShowYTics, new GridBagConstraints(
        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelCheck.add( myShowAxisZero, new GridBagConstraints(
        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelCheck.add( myShowAnnotationX, new GridBagConstraints(
        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelCheck.add( myShowAnnotationY, new GridBagConstraints(
        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelCheck.add( myShowBorder, new GridBagConstraints(
        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelCheck.add( myShowInnerBorder, new GridBagConstraints(
        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelCheck.add( myShowGrid, new GridBagConstraints(
        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

//    panelCheck.add( myAutoScaleAxes, new GridBagConstraints(
//        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
//        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    //    panelCheck.add( myIsTrueScale, new GridBagConstraints(
//        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
//        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
//    panelCheck.add( myCentreAxisX, new GridBagConstraints(
//        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
//        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
//    panelCheck.add( myCentreAxisY, new GridBagConstraints(
//        0, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
//        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    panelCheck.add( Box.createVerticalGlue(), new GridBagConstraints(
        0, yp++, 1, 1, 1.0, 1.0, GridBagConstraints.WEST,
        GridBagConstraints.VERTICAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    yp = 0;

    panelOther.add( myLabelXAxisScale, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelOther.add( myButtonXLinearScale, new GridBagConstraints(
        1, yp, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelOther.add( myButtonXLogScale, new GridBagConstraints(
        2, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelOther.add( myLabelYAxisScale, new GridBagConstraints(
        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.NONE, new Insets( 0, 0, 0, 5 ), 0, 0 ) );
    panelOther.add( myButtonYLinearScale, new GridBagConstraints(
        1, yp, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
    panelOther.add( myButtonYLogScale, new GridBagConstraints(
        2, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

//    panelOther.add( myUseRefValueY, new GridBagConstraints(
//        0, yp, 1, 1, 0.0, 0.0, GridBagConstraints.WEST,
//        GridBagConstraints.NONE, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
//    panelOther.add( myTextRefValueY, new GridBagConstraints(
//        1, yp++, 1, 1, 1.0, 0.0, GridBagConstraints.WEST,
//        GridBagConstraints.HORIZONTAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

    //  ----------------------------
    //  Button panel
     myButtonOK = new JButton("OK");
     myButtonApply = new JButton("Apply");
     myButtonCancel = new JButton("Cancel");
     this.getRootPane().setDefaultButton(myButtonApply);

     //  ----------------------------
     int xp = 0;
     yp = 0;
     JPanel panelUpper = new JPanel(new GridBagLayout());
     panelUpper.add( panelCheck, new GridBagConstraints(
         0, 0, 1, 1, 0.0, 1.0, GridBagConstraints.NORTH,
         GridBagConstraints.VERTICAL, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
     panelUpper.add( panelText, new GridBagConstraints(
         1, 0, 1, 1, 1.0, 1.0, GridBagConstraints.NORTH,
         GridBagConstraints.BOTH, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
//     panelUpper.add( panelOther, new GridBagConstraints(
//         1, 1, 1, 1, 1.0, 1.0, GridBagConstraints.NORTH,
//         GridBagConstraints.BOTH, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

     JPanel panelButtons = new JPanel( new GridBagLayout() );
     xp = 0;
     panelButtons.add( Box.createHorizontalGlue(), new GridBagConstraints(
         xp++, 0, 1, 1, 0.35, 1.0, GridBagConstraints.CENTER,
         GridBagConstraints.BOTH, new Insets( 0, 0, 0, 0 ), 0, 0 ) );
     panelButtons.add( myButtonOK, new GridBagConstraints(
         xp++, 0, 1, 1, 0.1, 0.0, GridBagConstraints.SOUTH,
         GridBagConstraints.BOTH, new Insets( 11, 0, 0, 0 ), 0, 0 ) );
     panelButtons.add( myButtonApply, new GridBagConstraints(
         xp++, 0, 1, 1, 0.1, 0.0, GridBagConstraints.SOUTHWEST,
         GridBagConstraints.BOTH, new Insets( 11, 0, 0, 0 ), 0, 0 ) );
     panelButtons.add( myButtonCancel, new GridBagConstraints(
         xp++, 0, 1, 1, 0.1, 0.0, GridBagConstraints.SOUTHWEST,
         GridBagConstraints.BOTH, new Insets( 11, 0, 0, 0 ), 0, 0 ) );
     panelButtons.add( Box.createHorizontalGlue(), new GridBagConstraints(
         xp++, 0, 1, 1, 0.35, 1.0, GridBagConstraints.CENTER,
         GridBagConstraints.BOTH, new Insets( 0, 0, 0, 0 ), 0, 0 ) );

     JPanel panelAll = new JPanel(new BorderLayout());
     panelAll.setBorder( csStandard.DIALOG_BORDER );

     yp = 0;

     panelAll.add(panelUpper,BorderLayout.CENTER);
     panelAll.add(panelButtons,BorderLayout.SOUTH);
     this.getContentPane().add(panelAll);

     this.changedSettings( myAttr );

     // Other settings
     myTextRefValueY.setEnabled( myUseRefValueY.isSelected() );
     
     this.pack();

     //------------------------------------------------------------
     //
     myButtonOK.addActionListener(new ActionListener() {
       public void actionPerformed(ActionEvent e) {
         apply();
         dispose();
       }
     });
     myButtonApply.addActionListener(new ActionListener() {
       public void actionPerformed(ActionEvent e) {
         apply();
       }
     });
     myButtonCancel.addActionListener(new ActionListener() {
       public void actionPerformed(ActionEvent e) {
         cancel();
       }
     });

     myUseRefValueY.addActionListener(new ActionListener() {
       public void actionPerformed(ActionEvent e) {
         myTextRefValueY.setEnabled( myUseRefValueY.isSelected() );
         apply();
       }
     });

      myAutoScaleAxes.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myCentreAxisX.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myCentreAxisY.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myShowAxisX.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myShowAxisY.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myShowAxisZero.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myShowXTics.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myShowYTics.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myShowAnnotationX.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myShowAnnotationY.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myShowBorder.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myShowInnerBorder.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myShowGrid.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myIsTrueScale.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myButtonYLinearScale.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myButtonYLogScale.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myButtonXLinearScale.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });
      myButtonXLogScale.addActionListener(new ActionListener() {
        @Override
        public void actionPerformed(ActionEvent e) {
          apply();
        }
      });

     this.addWindowListener(new WindowAdapter() {
       public void windowClosing(WindowEvent e) {
         cancel();
       }
     });
  }

  //*****************************************************************************************
  //
  //
  private void cancel() {
    dispose();
  }
  public void apply() {
    if( myGraph == null ) return;
    csGraphAttributes ga = new csGraphAttributes( myAttr );  // Copy current settings

    // Text fields
    try {
      String text;
      text = myTextInsetLeft.getText();
      ga.insetLeft = Integer.parseInt( text );
      text = myTextInsetTop.getText();
      ga.insetTop = Integer.parseInt( text );
      text = myTextInsetRight.getText();
      ga.insetRight = Integer.parseInt( text );
      text = myTextInsetBottom.getText();
      ga.insetBottom = Integer.parseInt( text );
      text = myTextMinAxisX.getText();
      ga.minAxisX = Float.parseFloat( text );
      text = myTextMaxAxisX.getText();
      ga.maxAxisX = Float.parseFloat( text );
      text = myTextMinAxisY.getText();
      ga.minAxisY = Float.parseFloat( text );
      text = myTextMaxAxisY.getText();
      ga.maxAxisY = Float.parseFloat( text );
      text = myTextGridIncX.getText();
      ga.gridXPixelInc = Integer.parseInt( text );
      text = myTextGridIncY.getText();
      ga.gridYPixelInc = Integer.parseInt( text );
      text = myTextMinorRatioX.getText();
      ga.minorXTicRatio = Integer.parseInt( text );
      text = myTextMinorRatioY.getText();
      ga.minorYTicRatio = Integer.parseInt( text );

      text = myBorderSize.getText();
      ga.borderSize = Integer.parseInt( text );
      text = myBorderPadding.getText();
      ga.borderPadding = Integer.parseInt( text );
      text = myInnerBorderSize.getText();
      ga.innerBorderSize = Integer.parseInt( text );
      text = myInnerBorderPadding.getText();
      ga.innerBorderPadding = Integer.parseInt( text );
      
      ga.title  = myGraphTitle.getText();
      ga.xLabel = myXAxisLabel.getText();
      ga.yLabel = myYAxisLabel.getText();

      text = myTextRefValueY.getText();
      ga.maxRefValueY = Float.parseFloat( text );
    }
    catch( NumberFormatException e ) {
//      System.out.println("Error in text fields...");
      return;
    }
    ga.autoScaleAxes = myAutoScaleAxes.isSelected();
    ga.showAxisX = myShowAxisX.isSelected();
    ga.showAxisY = myShowAxisY.isSelected();
    ga.showXTics = myShowXTics.isSelected();
    ga.showYTics = myShowYTics.isSelected();
    ga.showAxisXAnnotation = myShowAnnotationX.isSelected();
    ga.showAxisYAnnotation = myShowAnnotationY.isSelected();
    ga.showZeroAxis = myShowAxisZero.isSelected();
    ga.centreAxisX = myCentreAxisX.isSelected();
    ga.centreAxisY = myCentreAxisY.isSelected();
    ga.showBorder = myShowBorder.isSelected();
    ga.showInnerBorder = myShowInnerBorder.isSelected();
    ga.showGrid = myShowGrid.isSelected();
    ga.isTrueScale = myIsTrueScale.isSelected();
    ga.useRefValueY = myUseRefValueY.isSelected();
    ga.axisScaleX = myButtonXLinearScale.isSelected() ? csGraphAttributes.AXIS_SCALE_LINEAR : csGraphAttributes.AXIS_SCALE_LOG;
    ga.axisScaleY = myButtonYLinearScale.isSelected() ? csGraphAttributes.AXIS_SCALE_LINEAR : csGraphAttributes.AXIS_SCALE_LOG;

    myAttr = ga;
    if( myGraph != null ) {
      myIsUpdating = true;
      myGraph.setGraphAttributes( myAttr );
      myIsUpdating = false;
    }
  }

  //-------------------------------------------------------------------------------
  //
  //
  public void changedSettings( csGraphAttributes ga ) {
    if( myIsUpdating ) return;

    myAttr = ga;

    myTextInsetLeft.setText(""+ga.insetLeft);
    myTextInsetTop.setText(""+ga.insetTop);
    myTextInsetRight.setText(""+ga.insetRight);
    myTextInsetBottom.setText(""+ga.insetBottom);
    myTextMinAxisX.setText(""+ga.minAxisX);
    myTextMaxAxisX.setText(""+ga.maxAxisX);
    myTextMinAxisY.setText(""+ga.minAxisY);
    myTextMaxAxisY.setText(""+ga.maxAxisY);
    myTextGridIncX.setText(""+ga.gridXPixelInc);
    myTextGridIncY.setText(""+ga.gridYPixelInc);
    myTextMinorRatioX.setText(""+ga.minorXTicRatio);
    myTextMinorRatioY.setText(""+ga.minorYTicRatio);

    myBorderSize.setText(""+ga.borderSize);
    myBorderPadding.setText(""+ga.borderPadding);
    myInnerBorderSize.setText(""+ga.innerBorderSize);
    myInnerBorderPadding.setText(""+ga.innerBorderPadding);

    myGraphTitle.setText(""+ga.title);
    myXAxisLabel.setText(""+ga.xLabel);
    myYAxisLabel.setText(""+ga.yLabel);
    myTextRefValueY.setText(""+ga.maxRefValueY);

    myAutoScaleAxes.setSelected(ga.autoScaleAxes);
    myShowAxisX.setSelected(ga.showAxisX);
    myShowAxisY.setSelected(ga.showAxisY);
    myShowXTics.setSelected(ga.showXTics);
    myShowYTics.setSelected(ga.showYTics);
    myShowAnnotationX.setSelected(ga.showAxisXAnnotation);
    myShowAnnotationY.setSelected(ga.showAxisYAnnotation);
    myShowAxisZero.setSelected(ga.showZeroAxis);
    myShowBorder.setSelected(ga.showBorder);
    myShowGrid.setSelected(ga.showGrid);
    myCentreAxisX.setSelected(ga.centreAxisX);
    myCentreAxisY.setSelected(ga.centreAxisY);
    myIsTrueScale.setSelected(ga.isTrueScale);
    myUseRefValueY.setSelected(ga.useRefValueY);
    myButtonXLinearScale.setSelected(ga.axisScaleX == csGraphAttributes.AXIS_SCALE_LINEAR);
    myButtonXLogScale.setSelected(ga.axisScaleX == csGraphAttributes.AXIS_SCALE_LOG);
    myButtonYLinearScale.setSelected(ga.axisScaleY == csGraphAttributes.AXIS_SCALE_LINEAR);
    myButtonYLogScale.setSelected(ga.axisScaleY == csGraphAttributes.AXIS_SCALE_LOG);
  }
  //--------------------------
  //
  //
  
  public static void main( String[] args ) {
    try {
//      UIManager.setLookAndFeel( "com.sun.java.swing.plaf.windows.WindowsLookAndFeel" );
//      UIManager.setLookAndFeel( new MetalLookAndFeel() );
    }
    catch( Exception e ) {
      e.printStackTrace();
    }
    
    csGraph2D graph = new csGraph2D();
    csGraphDispDialog dialog = new csGraphDispDialog( graph );
    dialog.setVisible(true);
    
    dialog.addWindowListener(new WindowAdapter() {
      public void windowClosing( WindowEvent e ) {
        System.exit( 0 );
      }
    });    
  }
}


