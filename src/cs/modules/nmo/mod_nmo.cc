/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include "csFlexNumber.h"
#include "csNMOCorrection.h"
#include "csTimeFunction.h"
#include "csTableManagerNew.h"
#include "csTableAll.h"
#include "csTableNew.h"
#include "csTableValueList.h"
#include <cmath>

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: NMO
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_nmo {
  struct VariableStruct {
    float* time_sec;  // [s]
    float* velocities;  // [m/s]
    csNMOCorrection* nmo;
    int numTimes;
    int hdrId_offset;
    int mode;
    bool dump;

    csTableNew* table;
    csTableManagerNew* oldTableManager;
    int* hdrId_keys;
  };
}
using mod_nmo::VariableStruct;

//*************************************************************************************************
// Init phase
//
//
//
//*************************************************************************************************
void init_mod_nmo_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csExecPhaseDef*   edef = env->execPhaseDef;
  csSuperHeader*    shdr = env->superHeader;
  csTraceHeaderDef* hdef = env->headerDef;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  edef->setExecType( EXEC_TYPE_SINGLETRACE );

//---------------------------------------------
//
  vars->numTimes       = 0;
  vars->time_sec       = NULL;
  vars->velocities     = NULL;
  vars->nmo            = NULL;
  vars->table          = NULL;
  vars->oldTableManager   = NULL;
  vars->hdrId_keys     = NULL;
  vars->dump           = false; 

  csVector<std::string> valueList;
  
  //-------------------------------------
  std::string text;
  int mode_nmo = csNMOCorrection::PP_NMO;
  float offsetApex_m = 4000;
  float zeroOffsetDamping = 5;
  if( param->exists("wave_mode") ) {
    param->getString( "wave_mode", &text );
    if( !text.compare("pp_iso") ) {
      mode_nmo = csNMOCorrection::PP_NMO;
    }
    else if( !text.compare("ps_iso") ) {
      mode_nmo = csNMOCorrection::PS_NMO;
    }
    else {
      log->error("Option not recognized: %s.", text.c_str());
    }
  }
  if( param->exists("empirical") ) {
    mode_nmo = csNMOCorrection::EMPIRICAL_NMO;
    param->getFloat("empirical", &offsetApex_m, 0 );
    param->getFloat("empirical", &zeroOffsetDamping, 1 );
  }
  if( param->exists("output_vel") ) {
    param->getString( "output_vel", &text );
    if( !text.compare("no") ) {
      // Do nothing, keep NMO mode set earlier
    }
    else if( !text.compare("yes") ) {
      mode_nmo = csNMOCorrection::OUTPUT_VEL;
    }
    else {
      log->error("Option not recognized: %s.", text.c_str());
    }
  }

  vars->mode = csNMOCorrection::NMO_APPLY;
  if( param->exists("mode") ) {
    param->getString( "mode", &text );
    if( !text.compare("apply") ) {
      vars->mode = csNMOCorrection::NMO_APPLY;
    }
    else if( !text.compare("remove") ) {
      vars->mode = csNMOCorrection::NMO_REMOVE;
    }
    else {
      log->error("Option not recognized: %s.", text.c_str());
    }
  }

  if( param->exists("dump") ) {
    param->getString( "dump", &text );
    if( !text.compare("yes") ) {
      vars->dump = true;
    }
    else if( !text.compare("no") ) {
      vars->dump = false;
    }
    else {
      log->error("Option not recognized: %s.", text.c_str());
    }
  }
//---------------------------------------------
// Retrieve velocity table
//
  if( param->exists("table_old") ) {
    param->getString("table_old", &text );
    try {
      vars->oldTableManager = new csTableManagerNew( text, csTableAll::TABLE_TYPE_TIME_FUNCTION, hdef );
    }
    catch( csException& exc ) {
      log->error("Error when initializing input table '%s': %s\n", text.c_str(), exc.getMessage() );
    }
    if( edef->isDebug() ) {
      vars->oldTableManager->dump();
    }
  }

  if( param->exists("table") ) {
    if( vars->oldTableManager != NULL ) log->error("Specify either user parameter 'table' or 'table_old', not both");
    std::string tableFilename;
    int colTime;
    int colVel;
    param->getString("table", &tableFilename );
    param->getInt("table_col",&colTime,0);
    param->getInt("table_col",&colVel,1);
    if( colTime < 1 || colVel < 1 ) log->error("Column numbers in table ('table_col') must be larger than 0");
    vars->table = new csTableNew( csTableNew::TABLE_TYPE_TIME_FUNCTION, colTime-1 );
    int numKeys = param->getNumLines("table_key");
    if( numKeys > 0 ) {
      int col;
      vars->hdrId_keys  = new int[numKeys];
      for( int ikey = 0; ikey < numKeys; ikey++ ) {
	std::string headerName;
	bool interpolate = false;
	param->getStringAtLine( "table_key", &headerName, ikey, 0 );
	param->getIntAtLine( "table_key", &col, ikey, 1 );
	if( param->getNumValues( "table_key", ikey ) > 2 ) {
	  param->getStringAtLine( "table_key", &text, ikey, 2 );
	  if( !text.compare("yes") ) {
	    interpolate = true;
	  }
	  else if( !text.compare("no") ) {
	    interpolate = false;
	  }
	  else {
	    log->error("Unknown option: %s", text.c_str() );
	  }
	}
	vars->table->addKey( col-1, interpolate );  // -1 to convert from 'user' column to 'C' column
	if( !hdef->headerExists( headerName ) ) {
	  log->error("No matching trace header found for table key '%s'", headerName.c_str() );
	}
	vars->hdrId_keys[ikey] = hdef->headerIndex( headerName );
      } // END for ikey
    }

    vars->table->addValue( colVel-1 );  // -1 to convert from 'user' column to 'C' column

    bool sortTable = false;
    try {
      vars->table->initialize( tableFilename, sortTable );
    }
    catch( csException& exc ) {
      log->error("Error when initializing input table '%s': %s\n", text.c_str(), exc.getMessage() );
    }
  }
  
//---------------------------------------------
// Retrieve velocity-time pairs
//
  if( vars->table == NULL && vars->oldTableManager == NULL ) {
    if( param->exists( "time" ) ) {
      param->getAll( "time", &valueList );
      if( valueList.size() < 1 ){
        log->error("No times specified in user parameter 'time'!");
      }
    }
    vars->numTimes = valueList.size();

    if( vars->numTimes > 0 ) {
      vars->time_sec = new float[vars->numTimes];
      for( int i = 0; i < vars->numTimes; i++ ) {
        vars->time_sec[i] = atof( valueList.at(i).c_str() ) / 1000.0;  // Convert to seconds
      }
    }
    else {
      vars->numTimes = 1;
      vars->time_sec = new float[vars->numTimes];
      vars->time_sec[0] = 0;
    }

    vars->velocities = new float[vars->numTimes];

    valueList.clear();
    param->getAll( "velocity", &valueList );

    if( valueList.size() < 1 ){
      log->error("Missing user parameter 'velocity'!");
    }
    else if( valueList.size() != vars->numTimes ) {
      log->error("Unequal number of velocities(%d) and times(%d)", valueList.size(), vars->numTimes );
    }
    csFlexNumber number;
    for( int i = 0; i < vars->numTimes; i++ ) {
      if( !number.convertToNumber( valueList.at(i) ) ) {
        log->error("Specified velocity is not a valid number: '%s'", valueList.at(i).c_str() );
      }
      vars->velocities[i] = number.floatValue();
      if( edef->isDebug() ) log->line("Velocity #%d: '%s' --> %f", i, valueList.at(i).c_str(), vars->velocities[i] );
    }
  }

  //---------------------------------------------
  // Set NMO object
  //
  vars->nmo = new csNMOCorrection( shdr->sampleInt, shdr->numSamples, mode_nmo );
  vars->nmo->setModeOfApplication( vars->mode );
  if( mode_nmo == csNMOCorrection::EMPIRICAL_NMO ) {
    vars->nmo->setEmpiricalNMO( offsetApex_m, zeroOffsetDamping );
  }

  if( param->exists("horizon_nmo") ) {
    param->getString( "horizon_nmo", &text );
    if( !text.compare("yes") ) {
      int horMethod = csNMOCorrection::HORIZON_METHOD_LINEAR;
      if( param->getNumValues("horizon_nmo") > 1 ) {
        param->getString("horizon_nmo", &text, 1);
        if( !text.compare("linear") ) {
          horMethod = csNMOCorrection::HORIZON_METHOD_LINEAR;
        }
        else if( !text.compare("quadratic") ) {
          horMethod = csNMOCorrection::HORIZON_METHOD_QUAD;
        }
        else {
          log->error("Option not recognized: %s.", text.c_str());
        }
      }
      vars->nmo->setHorizonBasedNMO( true, horMethod );
    }
    else if( !text.compare("no") ) {
      vars->nmo->setHorizonBasedNMO( false );
    }
    else {
      log->error("Option not recognized: %s.", text.c_str());
    }
  }
  
  if( !hdef->headerExists( "offset" ) ) {
    log->error("Trace header 'offset' does not exist. Cannot perform NMO correction.");
  }
  else if( hdef->headerType( "offset" ) != TYPE_FLOAT && hdef->headerType( "offset" ) != TYPE_DOUBLE ) {
    log->error("Trace header 'offset' exists but has the wrong number type. Should be FLOAT.");
  }

  vars->hdrId_offset = hdef->headerIndex( "offset" );
}

//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************
bool exec_mod_nmo_( csTrace* trace, int* port, csExecPhaseEnv* env, csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>( env->execPhaseDef->variables() );
  csExecPhaseDef* edef = env->execPhaseDef;

  if( edef->isCleanup() ) {
    if( vars->time_sec != NULL ) {
      delete [] vars->time_sec;
      vars->time_sec = NULL;
    }
    if( vars->velocities != NULL ) {
      delete [] vars->velocities;
      vars->velocities = NULL;
    }
    if( vars->nmo != NULL ) {
      delete vars->nmo;
      vars->nmo = NULL;
    }
    if( vars->table != NULL ) {
      delete vars->table;
      vars->table = NULL;
    }
    if( vars->oldTableManager != NULL ) {
      delete vars->oldTableManager;
      vars->oldTableManager = NULL;
    }
    if( vars->hdrId_keys != NULL ) {
      delete [] vars->hdrId_keys;
      vars->hdrId_keys = NULL;
    }
    delete vars; vars = NULL;
    return true;
  }

  float* samples = trace->getTraceSamples();
  double offset  = trace->getTraceHeader()->doubleValue( vars->hdrId_offset );

  if( vars->table != NULL ) {
    if( vars->table->numKeys() > 0 ) {
      double* keyValueBuffer = new double[vars->table->numKeys()];
      for( int ikey = 0; ikey < vars->table->numKeys(); ikey++ ) {
	keyValueBuffer[ikey] = trace->getTraceHeader()->doubleValue( vars->hdrId_keys[ikey] );
      }
      csTimeFunction<double> const* timeFunc = vars->table->getFunction( keyValueBuffer, vars->dump );
      vars->nmo->perform_nmo( timeFunc, offset, samples );
      delete [] keyValueBuffer;
    }
    else {
      csTimeFunction<double> const* timeFunc = vars->table->getFunction( NULL, vars->dump );
      vars->nmo->perform_nmo( timeFunc, offset, samples );
    }
  }
  else if( vars->oldTableManager != NULL ) {
    csTimeFunction<double> const* timeFunc = vars->oldTableManager->getFunction( trace->getTraceHeader() );
    vars->nmo->perform_nmo( timeFunc, offset, samples );
  }
  else {
    vars->nmo->perform_nmo( vars->numTimes, vars->time_sec, vars->velocities, offset, samples );
  }

  return true;
}

//*************************************************************************************************
// Parameter definition
//
//
//*************************************************************************************************
void params_mod_nmo_( csParamDef* pdef ) {
  pdef->setModule( "NMO", "Perform NMO (normal moveout) correction" );

  pdef->addDoc("When reading in the velocity field from an ASCII table, velocity-time functions are interpolated at the trace locations based on the");
  pdef->addDoc("specified 'table_key' key trace headers. The interpolation assumes a regular cartesian grid, oriented W-E/N-S.");
  pdef->addDoc("This will give incorrect results if the grid has an oblique orientation and if XY coordinates are used as keys. If inline/crossline numbers");
  pdef->addDoc("are used as keys, the interpolation will work correctly regardless of the grid's orientation.");
  pdef->addDoc("If XY coordinates must be used as keys, the work-around is to re-grid the input velocity field to a W-E/N-S grid.");

  pdef->addParam( "wave_mode", "Mode of wave to model", NUM_VALUES_FIXED );
  pdef->addValue( "pp_iso", VALTYPE_OPTION );
  pdef->addOption( "pp_iso", "Isotropic PP mode (downgoing P, upgoing P)" );
  pdef->addOption( "ps_iso", "Isotropic PS mode (downgoing P, upgoing S)" );

  pdef->addParam( "time", "List of time value [ms]", NUM_VALUES_VARIABLE, "Time knee points at which specified velocities apply" );
  pdef->addValue( "", VALTYPE_NUMBER, "List of time values [ms]." );

  pdef->addParam( "velocity", "List of velocities [m/s]", NUM_VALUES_VARIABLE,
      "Velocities values at specified time knee points. In between time knee points, velocities will be linearly interpolated." );
  pdef->addValue( "", VALTYPE_NUMBER, "List of velocities [m/s]..." );

  pdef->addParam( "table", "Velocity table", NUM_VALUES_FIXED,
                  "Velocity table format: The ASCII file should contain only numbers, no text. Required are two columns containing time and velocity pairs. Optional: Up to 2 key values. Lines starting with '#' are considered comment lines." );
  pdef->addValue( "", VALTYPE_STRING, "Filename containing velocity table.");

  pdef->addParam( "table_col", "Table columns containing time in [ms] and velocity in [m/s]", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_NUMBER, "Column number in input table containing time [ms]" );
  pdef->addValue( "", VALTYPE_NUMBER, "Column number in input table containing velocity [m/s]" );

  pdef->addParam( "table_key", "Key trace header used to match values found in specified table columns", NUM_VALUES_VARIABLE,
                  "Specify the 'key' parameter for each key in the velocity file" );
  pdef->addValue( "", VALTYPE_STRING, "Trace header name of key header" );
  pdef->addValue( "", VALTYPE_NUMBER, "Column number in input table" );
  pdef->addValue( "no", VALTYPE_OPTION, "Interpolate based to this key?" );
  pdef->addOption( "yes", "Use this key for interpolation of value" );
  pdef->addOption( "no", "Do not use this key for interpolation", "The input table is expected to contain the exact key values for this trace header" );

  pdef->addParam( "mode", "Mode of NMO application", NUM_VALUES_FIXED );
  pdef->addValue( "apply", VALTYPE_OPTION );
  pdef->addOption( "apply", "Apply NMO." );
  pdef->addOption( "remove", "Remove NMO." );

  pdef->addParam( "empirical", "Empirical 'NMO' function", NUM_VALUES_FIXED,
                  "Empirical NMO is a residual NMO, to be applied after initial NMO or LMO. Best scan for the two parameters to find out their effect on the data" );
  pdef->addValue( "4000", VALTYPE_NUMBER, "Offset apex [m]" );
  pdef->addValue( "5", VALTYPE_NUMBER, "Zero-offset damping (High value: high damping, 0: No damping)" );

  pdef->addParam( "horizon_nmo", "Apply horizon based NMO?", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "no", "Do not apply horizon based NMO" );
  pdef->addOption( "yes", "Apply horizon based NMO" );
  pdef->addValue( "linear", VALTYPE_OPTION );
  pdef->addOption( "linear", "Use linear interpolation method" );
  pdef->addOption( "quadratic", "Use quadratic interpolation method" );

  pdef->addParam( "dump", "Dump velocity functions to standard output?", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "no", "Do not dump velocity functions" );
  pdef->addOption( "yes", "Dump velocity functons retrieved at every location to standard output" );

  pdef->addParam( "output_vel", "Output velocity field instead of NMO'd data?", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_STRING );
  pdef->addOption( "no", "Output input data with NMO applied/removed" );
  pdef->addOption( "yes", "Output velocity field read in from input velocity table" );

  pdef->addParam( "table_old", "Velocity table (OLD format)", NUM_VALUES_FIXED, "This option is provided to provide some backward compatibility" );
  pdef->addValue( "", VALTYPE_STRING, "Velocity table file name.",
                  "Velocity table format: The first line must contain a header title giving the names of all columns (space separated). The remaining lines give the data values. Key headers are specified in the front with a preceding '@' sign. The column specifying the time in milliseconds must be named 'time'. The last column must contain a velocity in m/s. It doesn't matter what name is chosen for the velocity column, unit is meters/second [m/s]. Example:  '@source  time  velocity'. The table does not need to contain a key trace header name; Example: 'time  velocity'." );
}

extern "C" void _params_mod_nmo_( csParamDef* pdef ) {
  params_mod_nmo_( pdef );
}
extern "C" void _init_mod_nmo_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_nmo_( param, env, log );
}
extern "C" bool _exec_mod_nmo_( csTrace* trace, int* port, csExecPhaseEnv* env, csLogWriter* log ) {
  return exec_mod_nmo_( trace, port, env, log );
}


