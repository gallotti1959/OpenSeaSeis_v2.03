/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include "csFlexNumber.h"
#include "csTableAll.h"
#include "csTableManagerNew.h"
#include <cmath>

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: MUTE
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_mute {
  struct VariableStruct {
    int mode;
    float mute_time;
    int mute_start_samp;
    int mute_end_samp;
    csTableManagerNew* tableManager;
    bool killZeroTraces;
    int taperLengthSamples;
    bool indicate;
    float indicateValue;
    int   indicateWidthSamples;
    int taperType;
  };
  static int const TAPER_LINEAR = 1;
  static int const TAPER_COSINE = 2;
}
using mod_mute::VariableStruct;

//*************************************************************************************************
// Init phase
//
//
//
//*************************************************************************************************
void init_mod_mute_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csExecPhaseDef*   edef = env->execPhaseDef;
  csTraceHeaderDef* hdef = env->headerDef;
  csSuperHeader*    shdr = env->superHeader;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  edef->setExecType( EXEC_TYPE_SINGLETRACE );

  vars->mode         = MUTE_FRONT;
  vars->tableManager        = NULL;
  vars->mute_start_samp = 0;
  vars->mute_end_samp   = 0;
  vars->killZeroTraces  = false;
  vars->indicate        = false;
  vars->indicateValue   = 0.0;
  vars->indicateWidthSamples = 1;
  vars->taperType       = mod_mute::TAPER_LINEAR;
  float taperLength     = 0; // See below

//---------------------------------------------
// Retrieve mute table
//
  std::string text;

  if( param->exists("table") ) {

    param->getString("table", &text );

    try {
      vars->tableManager = new csTableManagerNew( text, csTableAll::TABLE_TYPE_UNIQUE_KEYS, hdef );
      if( vars->tableManager->valueName().compare("time") ) {
        log->error("Mute table must contain 'value' column labelled 'time', for example: '@%s time'. Value label found: '%s'",
                   vars->tableManager->numKeys() > 0 ? vars->tableManager->keyName(0).c_str() : "offset",
                   vars->tableManager->valueName().c_str() );
      }
    }
    catch( csException& exc ) {
      log->error("Error when initializing input table '%s': %s\n", text.c_str(), exc.getMessage() );
    }
    if( vars->tableManager->numKeys() < 1 ) {
      log->error("Number of table keys = %d. Specify table key by placing the character '%c' in front of the key name. Example: %csource time vel  (source is a table key)",
                 csTableAll::KEY_CHAR, csTableAll::KEY_CHAR);
    }

    if( edef->isDebug() ) {
      vars->tableManager->dump();
    }

  }

//---------------------------------------------
// Retrieve mute time
//
  if( param->exists("mode") ) {
    param->getString("mode",&text);
    if( !text.compare("front") ) {
      vars->mode = MUTE_FRONT;
    }
    else if( !text.compare("end") ) {
      vars->mode = MUTE_END;
    }
    else {
      log->error("Unknown mode option: '%s'", text.c_str());
    }
  }

  if( param->exists("kill") ) {
    param->getString("kill",&text);
    if( !text.compare("yes") ) {
      vars->killZeroTraces = true;
    }
    else if( !text.compare("no") ) {
      vars->killZeroTraces = false;
    }
    else {
      log->error("Unknown option: '%s'", text.c_str());
    }
  }
  if( param->exists("indicate") ) {
    vars->indicate = true;
    param->getFloat( "indicate", &vars->indicateValue );
    if( param->getNumValues("indicate") > 1 ) {
      param->getInt( "indicate", &vars->indicateWidthSamples, 1 );
    }
  }

  if( param->exists("taper_len") ) {
    param->getFloat("taper_len", &taperLength );
  }
  vars->taperLengthSamples = (int)( taperLength/shdr->sampleInt + 0.5 );

  if( param->exists("taper_type") ) {
    param->getString("taper_type", &text);
    if( !text.compare("cos") ) {
      vars->taperType = mod_mute::TAPER_COSINE;
    }
    else if( !text.compare("linear") ) {
      vars->taperType = mod_mute::TAPER_LINEAR;
    }
    else {
      log->error("Unknown option: %s", text.c_str());
    }
  }

  if( !vars->tableManager ) {
    param->getFloat( "time", &vars->mute_time );
    float recordLength = (float)(shdr->numSamples-1) * shdr->sampleInt;
    int sampleIndex = (int)( vars->mute_time / shdr->sampleInt );

    if( vars->mute_time < 0 || vars->mute_time > recordLength ) {
      log->error("Specified mute time (%.0fms) is outside of valid range (0-%.0fms).",
        vars->mute_time, shdr->numSamples*shdr->sampleInt );
    }

    if( vars->mode == MUTE_FRONT ) {
      vars->mute_start_samp = 0;
      vars->mute_end_samp   = sampleIndex-1;
    }
    else {
      vars->mute_start_samp = sampleIndex+1;
      vars->mute_end_samp   = shdr->numSamples-1;
    }
  }

//----------------------------------------------

}

//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************
bool exec_mod_mute_(
  csTrace* trace,
  int* port,
  csExecPhaseEnv* env, csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>( env->execPhaseDef->variables() );
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader const* shdr = env->superHeader;
  //  csTraceHeaderDef const* hdef = env->headerDef;

  if( edef->isCleanup()){
    if( vars->tableManager != NULL ) {
      delete vars->tableManager;
      vars->tableManager = NULL;
    }
    delete vars; vars = NULL;
    return true;
  }

  float* samples = trace->getTraceSamples();
  if( vars->tableManager != NULL ) {
    vars->mute_time = vars->tableManager->getValue( trace->getTraceHeader() );
    int sampleIndex = (int)( vars->mute_time / shdr->sampleInt );
    if( vars->mode == MUTE_FRONT ) {
      vars->mute_start_samp = 0;
      vars->mute_end_samp   = std::min( shdr->numSamples-1,sampleIndex-1);
    }
    else { // MUTE_END
      vars->mute_start_samp = std::max( 0, sampleIndex+1 );
      vars->mute_end_samp   = shdr->numSamples-1;
    }
    if( edef->isDebug() ) log->line("Mute time: %f  Start/end sample: %d %d", vars->mute_time, vars->mute_start_samp, vars->mute_end_samp );
  }

  if( edef->isDebug() ) {
    log->line("Mute time sample index: %d %d (nsamp: %d)", vars->mute_start_samp, vars->mute_end_samp, shdr->numSamples);
  }

  if( vars->indicate ) { 
    int sampleIndex = (int)( vars->mute_time / shdr->sampleInt );
    int endSamp = std::min( sampleIndex+vars->indicateWidthSamples, shdr->numSamples );
    for( int isamp = std::max( sampleIndex-vars->indicateWidthSamples, 0); isamp < endSamp; isamp++ ) {
      samples[isamp] = vars->indicateValue;
    }
  }
  else {
    if( vars->killZeroTraces && vars->mute_end_samp >= shdr->numSamples-1 && vars->mute_start_samp <= 1 ) {
      return false;
    }
    for( int isamp = vars->mute_start_samp; isamp <= vars->mute_end_samp; isamp++ ) {
      samples[isamp] = 0.0;
    }
    if( vars->mode == MUTE_FRONT ) {
      int startSamp = vars->mute_end_samp+1;
      int endSamp   = std::min( startSamp+vars->taperLengthSamples, shdr->numSamples );
      if( edef->isDebug() ) log->line("Start/end sample of taper: %d %d", startSamp, endSamp);
      if( vars->taperType == mod_mute::TAPER_LINEAR ) {
        for( int isamp = startSamp; isamp < endSamp; isamp++ ) {
          float weight = (float)(isamp-startSamp+1)/(float)vars->taperLengthSamples;
          samples[isamp] *= weight;
        }
      }
      else { // COSINE taper
        for( int isamp = startSamp; isamp < endSamp; isamp++ ) {
          float phase  = ( ( (float)(isamp-startSamp) / (float)vars->taperLengthSamples ) - 1.0 ) * M_PI;
          float weight = 0.5 * (cos(phase) + 1.0);
          samples[isamp] *= weight;
        }
      }
    }
    else {
      int startSamp = std::max( 0, vars->mute_start_samp-vars->taperLengthSamples );
      int endSamp   = std::min( vars->mute_start_samp, shdr->numSamples );
      if( edef->isDebug() ) log->line("Start/end sample of taper: %d %d", startSamp, endSamp);
      if( vars->taperType == mod_mute::TAPER_LINEAR ) {
        for( int isamp = startSamp; isamp < endSamp; isamp++ ) {
          float weight = (float)(endSamp-isamp)/(float)vars->taperLengthSamples;
          samples[isamp] *= weight;
        }
      }
      else { // COSINE taper
        for( int isamp = startSamp; isamp < endSamp; isamp++ ) {
          float phase  = ( ( (float)(endSamp-isamp) / (float)vars->taperLengthSamples ) - 1.0 ) * M_PI;
          float weight = 0.5 * (cos(phase) + 1.0);
          samples[isamp] *= weight;
        }
      }
    }
  }

  return true;
}

//*************************************************************************************************
// Parameter definition
//
//
//*************************************************************************************************
void params_mod_mute_( csParamDef* pdef ) {
  pdef->setModule( "MUTE", "Mute trace data" );

  pdef->addParam( "mode", "Front mute or end mute", NUM_VALUES_FIXED );
  pdef->addValue( "front", VALTYPE_OPTION );
  pdef->addOption( "front", "Front mute. Specify time of first unmuted sample.", "Samples are muted from 0ms to specified time" );
  pdef->addOption( "end", "End mute. Specify time of last unmuted sample.", "Samples are muted from specified time to the end of the trace" );

  pdef->addParam( "table", "Mute table", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_STRING, "Mute table file, full path.",
                  "The mute table must have at least two columns, one giving a key and the second giving the mute time in [ms]" );
  //  pdef->addValue( "", VALTYPE_STRING, "Mute table. This must be specified within the flow file, using directive &table <tablename> <filename> {methodInterpolation}",
  //    "The mute table must have at least two columns, one giving a key and the second giving the mute time in [ms]" );

  pdef->addParam( "time", "Mute time [ms]", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_NUMBER, "Mute time [ms]" );

  pdef->addParam( "taper_len", "Taper length [ms]", NUM_VALUES_FIXED );
  pdef->addValue( "0", VALTYPE_NUMBER, "Mute taper length [ms]" );

  pdef->addParam( "taper_type", "Type of mute taper", NUM_VALUES_FIXED );
  pdef->addValue( "linear", VALTYPE_OPTION );
  pdef->addOption( "linear", "Apply linear taper" );
  pdef->addOption( "cos", "Apply cosine taper" );

  pdef->addParam( "kill", "Kill zero traces?", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "no", "Kill zero traces" );
  pdef->addOption( "yes", "Do not kill zero traces" );

  pdef->addParam( "indicate", "Do not mute, indicate mute time by setting samples to the given value", NUM_VALUES_VARIABLE,
                  "Input data will not be muted. Instead, spikes are placed at the mute times, with the given amplitude." );
  pdef->addValue( "0", VALTYPE_NUMBER, "Value that mute samples are set to." );
  pdef->addValue( "1", VALTYPE_NUMBER, "Width in samples to indicate with given value." );
}

extern "C" void _params_mod_mute_( csParamDef* pdef ) {
  params_mod_mute_( pdef );
}
extern "C" void _init_mod_mute_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_mute_( param, env, log );
}
extern "C" bool _exec_mod_mute_( csTrace* trace, int* port, csExecPhaseEnv* env, csLogWriter* log ) {
  return exec_mod_mute_( trace, port, env, log );
}

