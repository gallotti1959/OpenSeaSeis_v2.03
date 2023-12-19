/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

package cseis.xcseis;

import cseis.general.csAbstractPreference;
import java.io.File;
import javax.swing.filechooser.FileSystemView;

public class csProjectDef {
  public static final String SUBDIR_FLOWS = "flows";
  public static final String SUBDIR_LOGS  = "logs";
  private String myPathLogs;
  private String myPathFlows;
  private String myPathRoot;
  private String myName;
  private String mySeparator;
  
  public csProjectDef() {
    init( FileSystemView.getFileSystemView().getHomeDirectory().getAbsolutePath(), "Default project" );
  }
  public csProjectDef( String pathProject ) {
    init( pathProject, "Default project" );
  }
  private void init( String pathProject, String name ) {
    if( csAbstractPreference.IS_WINDOWS ) {
      mySeparator = "\\";
    }
    else {
      mySeparator = "/";
    }
    myPathRoot  = pathProject;
    myPathLogs  = myPathRoot + mySeparator + SUBDIR_LOGS;
    myPathFlows = myPathRoot + mySeparator + SUBDIR_FLOWS;
    myName = name;
  }
  /**
   * Set new project root path
   * @param pathProject
   */
  public void set( String pathProject ) {
    init( pathProject, myName );
  }
  public boolean checkDirs() {
    boolean dirsExist = false;
    File dirFlows = new File(myPathFlows);
    File dirLogs = new File(myPathLogs);
    dirsExist = ( dirFlows.exists() && dirLogs.exists() );
    return dirsExist;
  }

  /**
   * Create flow & log directories
   */
  public void createDirs() throws Exception {
    File dirFlows = new File(myPathFlows);
    if( !dirFlows.exists() ) {
      if( !dirFlows.mkdirs() ) {
        throw( new Exception("Unable to create directory '" + dirFlows + "'.\nIs system write protected?") );
      }
    }
    File dirLogs = new File(myPathLogs);
    if( !dirLogs.exists() ) {
      if( !dirLogs.mkdirs() ) {
        throw( new Exception("Unable to create directory '" + dirLogs + "'.\nIs system write protected?") );
      }
    }
  }
  public final String projName() {
    return myName;
  }
  public final String pathFlows() {
    return myPathFlows;
  }
  public final String pathLogs() {
    return myPathLogs;
  }
  public final String pathProj() {
    return myPathRoot;
  }
}


