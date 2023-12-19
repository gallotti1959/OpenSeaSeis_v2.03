/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;

import cseis.general.csAbstractPreference;
import java.awt.Font;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.util.ArrayList;
import javax.swing.filechooser.FileSystemView;

/**
 * Preferences specific to XCSeis
 *
 */
public class csPreferences extends csAbstractPreference {
  private boolean DEBUG = false;
  public static final String FILE_HEADER = "### XCSEIS v" + XCSeis.VERSION + " preferences file ###";

  public static final String NODE_DIR_PROJ       = "<proj_dir>";
  public static final String NODE_DIR_PROPERTIES = "<properties_dir>";
  public static final String NODE_END            = "<end>";
  public static final String NODE_RECENT_FILES   = "<recent_files>";
  public static final String NODE_FONT   = "<font>";

  private String myProjDirectoryPath;
  private String myPropertiesDirectoryPath;
  private java.util.List<String> myRecentFileList = null;
  private XCSeis myFrame;

  public csPreferences( XCSeis xcseis ) {
    super( "xcseis" );
    myFrame = xcseis;
    myProjDirectoryPath = FileSystemView.getFileSystemView().getHomeDirectory().getAbsolutePath();
    myPropertiesDirectoryPath = csAbstractPreference.getCseisDirectory().getAbsolutePath();
  }
  public String getProjDirectory() {
    return myProjDirectoryPath;
  }
  public void setProjDirectoryPath( String dir ) {
    myProjDirectoryPath = dir;
  }
  public String getPropertiesDirectory() {
    return myPropertiesDirectoryPath;
  }
  public java.util.List<String> getRecentFileList() {
    return myRecentFileList;
  }
  public void setPropertiesDirectoryPath( String dir ) {
    myPropertiesDirectoryPath = dir;
  }
  public void setRecentFileList( java.util.List<String> list ) {
    myRecentFileList = list;
  }
//----------------------------------------------
  protected void readPreferences(BufferedReader reader) throws IOException {
    String line;
    myRecentFileList = new ArrayList<String>();
    while( (line = reader.readLine()) != null ) {
      if( DEBUG ) System.out.println("Preference reading...");
      if( line.equalsIgnoreCase(NODE_DIR_PROJ) ) {
        while( (line = reader.readLine()) != null && !line.trim().equalsIgnoreCase(NODE_END)) {
          myProjDirectoryPath = line.trim();
        }
        if( DEBUG ) System.out.println("Preference reading: " + NODE_DIR_PROJ + " project dir: " + myProjDirectoryPath);
      }
      else if( line.equalsIgnoreCase(NODE_DIR_PROPERTIES) ) {
        if( DEBUG ) System.out.println("Preference reading: " + NODE_DIR_PROPERTIES);
        while( (line = reader.readLine()) != null && !line.trim().equalsIgnoreCase(NODE_END)) {
          myPropertiesDirectoryPath = line.trim();
        }
      }
      else if( line.equalsIgnoreCase(NODE_RECENT_FILES) ) {
        myRecentFileList = new ArrayList<String>();
        if( DEBUG ) System.out.println("Preference reading: " + NODE_DIR_PROPERTIES);
        while( (line = reader.readLine()) != null && !line.trim().equalsIgnoreCase(NODE_END)) {
          myRecentFileList.add( line.trim() );
        }
      }
      else if( line.equalsIgnoreCase(NODE_FONT) ) {
        if( DEBUG ) System.out.println("Preference reading: " + NODE_FONT);
        Font font = myFrame.getSystemFont();
        int fontSize = font.getSize();
        while( (line = reader.readLine()) != null && !line.trim().equalsIgnoreCase(NODE_END)) {
          try {
            fontSize = Integer.parseInt( line.trim() );
          }
          catch( NumberFormatException e ) {
            // nothing
          }
        }
        myFrame.setSystemFont( new Font(font.getName(),font.getStyle(),fontSize) );
      }

    }
  }
  protected void writePreferences( BufferedWriter writer ) throws IOException {
    writer.write(FILE_HEADER);
    writer.newLine();
    writer.newLine();
    writer.write(NODE_DIR_PROJ);
    writer.newLine();
    String dir = myProjDirectoryPath;
    if( dir != null ) {
      writer.write( dir );
      writer.newLine();
    }
    writer.write(NODE_END);
    writer.newLine();

    writer.write(NODE_DIR_PROPERTIES);
    writer.newLine();
    dir = myPropertiesDirectoryPath;
    if( dir != null ) {
      writer.write( dir );
      writer.newLine();
    }
    writer.write(NODE_END);
    writer.newLine();

    writer.write(NODE_RECENT_FILES);
    writer.newLine();
    if( myRecentFileList != null ) {
      for( int i = 0; i < myRecentFileList.size(); i++ ) {
        writer.write( (String)myRecentFileList.get(i) );
        writer.newLine();
      }
    }
    writer.write(NODE_END);
    writer.newLine();

    writer.write(NODE_FONT);
    writer.newLine();
    Font font = myFrame.getSystemFont();
  //  writer.write( font.getName() );
  //  writer.newLine();
  //  writer.write( font.getStyle() );
  //  writer.newLine();
    writer.write( new Integer(font.getSize()).toString() );
    writer.newLine();
    writer.write(NODE_END);
    writer.newLine();
    writer.newLine();
  }
}


