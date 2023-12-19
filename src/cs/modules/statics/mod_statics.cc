/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include "csInterpolation.h"
#include <string>
#include <cstring>
#include <cmath>

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: STATICS
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_statics {
  struct VariableStruct {
    bool   doHeaderStatic;
    int    hdrId;
    int    hdrType;
    float  bulkShift_ms;
    float* buffer;
    bool   isApplyMode;
    int hdrID_time_samp1_s;
    int hdrID_time_samp1_us;
    cseis_geolib::csInterpolation* interpol;
  };
}
using mod_statics::VariableStruct;

//*************************************************************************************************
// Init phase
//
//
//
//*************************************************************************************************
void init_mod_statics_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csTraceHeaderDef* hdef = env->headerDef;
  csExecPhaseDef*   edef = env->execPhaseDef;
  csSuperHeader*    shdr = env->superHeader;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  edef->setExecType( EXEC_TYPE_SINGLETRACE );

  vars->doHeaderStatic = false;
  vars->hdrId     = -1;
  vars->hdrType   = 0;
  vars->bulkShift_ms = 0;
  vars->buffer    = NULL;
  vars->isApplyMode = true;
  vars->interpol  = NULL;
  vars->hdrID_time_samp1_s = -1;
  vars->hdrID_time_samp1_us = -1;

//---------------------------------------------
  // For now, stick to the default interpolation re number of coefficients etc
  int numCoefficients = 8;
  if( param->exists("ncoef") ) {
    param->getInt("ncoef", &numCoefficients );
    if( numCoefficients < 2 ) {
      log->error("Number of interpolation coefficients too small: %d. Minimum is 2", numCoefficients);
    }
    if( numCoefficients > 32 ) {
      log->warning("Number of interpolation coefficients is very large: %d.", numCoefficients);
    }
    if( numCoefficients > 128 ) {
      log->warning("Number of interpolation coefficients too large: %d.", numCoefficients);
    }
  }

  vars->interpol = new csInterpolation( shdr->numSamples, shdr->sampleInt, numCoefficients );

  if( param->exists("header") ) {
    std::string headerName;
    vars->doHeaderStatic = true;
    param->getString( "header", &headerName );
    if( ( headerName.c_str() ) ) {
      vars->hdrType = hdef->headerType( headerName.c_str() );
      vars->hdrId   = hdef->headerIndex( headerName.c_str() );
    }
    else {
      log->warning("Unknown trace header name: '%s'", headerName.c_str());
      env->addError();
    }
    log->line("Header: '%s', index: %d", headerName.c_str(), vars->hdrId );
  }

  if( param->exists("mode") ) {
    string text;
    param->getString("mode",&text);
    if( !text.compare("apply") ) {
      vars->isApplyMode = true;
    }
    else if( !text.compare("remove") ) {
      vars->isApplyMode = false;
    }
    else {
      log->error("Unknown mode option: '%s'", text.c_str());
    }
  }

  if( param->exists("bulk_shift") ) {
    param->getFloat( "bulk_shift", &vars->bulkShift_ms );  // [ms]
  }
  else {
    if( !vars->doHeaderStatic ) {
      log->line("Error: No statics option selected. Empty module.");
      env->addError();
    }
    vars->bulkShift_ms = 0.0;
  }

  if( !vars->isApplyMode ) {
    vars->bulkShift_ms *= -1.0;
  }

  vars->buffer = new float[shdr->numSamples];
  vars->hdrID_time_samp1_s  = hdef->headerIndex( HDR_TIME_SAMP1.name );
  vars->hdrID_time_samp1_us = hdef->headerIndex( HDR_TIME_SAMP1_US.name );
}

//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************
bool exec_mod_statics_(
  csTrace* trace,
  int* port,
  csExecPhaseEnv* env, csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>( env->execPhaseDef->variables() );
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader const* shdr = env->superHeader;

  if( edef->isCleanup()){
    if( vars->buffer ) {
      delete [] vars->buffer;
      vars->buffer = NULL;
    }
    if( vars->interpol != NULL ) {
      delete vars->interpol;
      vars->interpol = NULL;
    }

    delete vars; vars = NULL;
    return true;
  }

  float shift_ms = vars->bulkShift_ms;  // Shift in [ms]
  if( vars->doHeaderStatic ) {
    float stat_hdr = trace->getTraceHeader()->floatValue(vars->hdrId);
    if( vars->isApplyMode ) {
      shift_ms += stat_hdr;
    }
    else {
      shift_ms -= stat_hdr;
    }
    if( edef->isDebug() ) log->line("Static total shift = %f ms (...incl header shift: %f ms)", shift_ms, stat_hdr);
  }
  float* samples = trace->getTraceSamples();

  if( edef->isDebug() ) { log->line("Apply static %f ms", shift_ms); }

  memcpy( vars->buffer, trace->getTraceSamples(), shdr->numSamples*sizeof(float) );
  vars->interpol->static_shift( shift_ms, vars->buffer, samples );

  csTraceHeader* trcHdr = trace->getTraceHeader();
  int time_samp1_s  = trcHdr->intValue( vars->hdrID_time_samp1_s );
  int time_samp1_us = trcHdr->intValue( vars->hdrID_time_samp1_us );
  time_samp1_us -= (int)round(shift_ms*1000.0);
  if( time_samp1_us < 0 ) {
    time_samp1_us += 1000000;
    time_samp1_s  -= 1;
  }
  else if( time_samp1_us >= 1000000 ) {
    time_samp1_us -= 1000000;
    time_samp1_s  += 1;
  }
  trcHdr->setIntValue( vars->hdrID_time_samp1_s, time_samp1_s );
  trcHdr->setIntValue( vars->hdrID_time_samp1_us, time_samp1_us );

  return true;
}

//*************************************************************************************************
// Parameter definition
//
//
//*************************************************************************************************
void params_mod_statics_( csParamDef* pdef ) {
  pdef->setModule( "STATICS", "Apply trace statics" );

  pdef->addParam( "bulk_shift", "Apply static bulk shift to all traces", NUM_VALUES_FIXED );
  pdef->addValue( "0.0", VALTYPE_NUMBER, "Static shift [ms]. Positive value shifts samples downwards" );

  pdef->addParam( "header", "Apply static shift from trace header", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_STRING, "Name of trace header containing static shift [ms]. Positive value shifts samples downwards" );

  pdef->addParam( "mode", "Mode of application", NUM_VALUES_FIXED );
  pdef->addValue( "apply", VALTYPE_OPTION );
  pdef->addOption( "apply", "Apply all specified statics" );
  pdef->addOption( "remove", "Remove all specified statics", "Apply inverse/negative statics" );

  pdef->addParam( "ncoef", "Number of interpolation coefficients", NUM_VALUES_FIXED );
  pdef->addValue( "8", VALTYPE_STRING, "Number of interpolation coefficients" );
}


extern "C" void _params_mod_statics_( csParamDef* pdef ) {
  params_mod_statics_( pdef );
}
extern "C" void _init_mod_statics_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_statics_( param, env, log );
}
extern "C" bool _exec_mod_statics_( csTrace* trace, int* port, csExecPhaseEnv* env, csLogWriter* log ) {
  return exec_mod_statics_( trace, port, env, log );
}

