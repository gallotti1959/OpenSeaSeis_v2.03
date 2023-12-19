/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include "csFlexNumber.h"
#include <cmath>
#include <cstring>

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: GAIN
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_gain {
  struct VariableStruct {
    bool is_tgain;
    bool is_agc;
    bool is_trace;
    float tgain;
    int agcWindowLengthSamples;
    float* buffer;
    float traceAmp;
  };
}
using mod_gain::VariableStruct;

//*************************************************************************************************
// Init phase
//
//
//
//*************************************************************************************************
void init_mod_gain_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csExecPhaseDef*   edef = env->execPhaseDef;
  csSuperHeader*    shdr = env->superHeader;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  edef->setExecType( EXEC_TYPE_SINGLETRACE );

  vars->is_tgain = false;
  vars->is_agc   = false;
  vars->is_trace = false;
  vars->tgain    = 0;
  vars->agcWindowLengthSamples = 0;
  vars->buffer   = NULL;
  vars->traceAmp = 0;
//---------------------------------------------
//
  
  std::string tableName;
  if( param->exists("tgain") ) {
    param->getFloat("tgain", &vars->tgain );
    vars->is_tgain = true;
  }
  else {
    vars->is_tgain = false;
  }

  if( param->exists("trace_equal") ) {
    param->getFloat("trace_equal", &vars->traceAmp );
    vars->is_trace = true;
  }
  else {
    vars->is_trace = false;
  }

  float window = 0;
  if( param->exists("agc") ) {
    param->getFloat("agc", &window );
    vars->agcWindowLengthSamples = (int)( window/shdr->sampleInt + 0.5 );
    vars->is_agc = true;
    vars->buffer = new float[shdr->numSamples];
  }
  else {
    vars->is_agc = false;
  }
  
  if( (vars->is_tgain && vars->is_agc) || (vars->is_tgain && vars->is_trace) || (vars->is_agc && vars->is_trace) ) {
    log->error("More than one gain option specified. Can only specify one gain option at one time.");
  }
  else if( !vars->is_tgain && !vars->is_agc && !vars->is_trace ) {
    log->error("No gain option specified.");
  }
}

//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************
bool exec_mod_gain_(
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
    delete vars; vars = NULL;
    return true;
  }

  float* samples = trace->getTraceSamples();
  float sampleInt_s = shdr->sampleInt/1000.0;

  if( vars->is_tgain ) {
    if( vars->tgain == 2 ) {
      for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
        float time = (float)isamp*sampleInt_s;
        samples[isamp] *= time*time;
      }
    }
    else if( vars->tgain == 1 ) {
      for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
        float time = (float)isamp*sampleInt_s;
        samples[isamp] *= time;
      }
    }
    else {
      for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
        float time = (float)isamp*sampleInt_s;
        samples[isamp] *= pow( time, vars->tgain );
      }
    }
  }
  else if( vars->is_agc ) {
    for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
      int firstSamp = MAX(0,isamp-vars->agcWindowLengthSamples);
      int lastSamp  = MIN(shdr->numSamples-1,isamp+vars->agcWindowLengthSamples);
      float rms = 0.0;
      for( int isampSum = firstSamp; isampSum <= lastSamp; isampSum++ ) {
        rms += samples[isampSum]*samples[isampSum];
      }
      rms = sqrt( rms/(lastSamp-firstSamp+1) );
      if( rms != 0 ) vars->buffer[isamp] = samples[isamp] / rms;
      else vars->buffer[isamp] = 0.0;
    }
    memcpy( samples, vars->buffer, shdr->numSamples*sizeof(float) );
  }
  else if( vars->is_trace ) {
    float sum = 0.0;
    for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
      sum += fabs( samples[isamp] );
    }
    float mean = sum / ( (float)shdr->numSamples * vars->traceAmp );
    for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
      samples[isamp] /= mean;
    }
  }
  return true;
}

//*************************************************************************************************
// Parameter definition
//
//
//*************************************************************************************************
void params_mod_gain_( csParamDef* pdef ) {
  pdef->setModule( "GAIN", "Apply gain function to trace samples" );

  pdef->addParam( "tgain", "Apply time gain function", NUM_VALUES_FIXED );
  pdef->addValue( "2", VALTYPE_NUMBER, "Time gain value: gain = t^value" );

  pdef->addParam( "agc", "Apply automatic gain control (AGC)", NUM_VALUES_FIXED );
  pdef->addValue( "500", VALTYPE_NUMBER, "AGC sliding window length [ms]" );

  pdef->addParam( "trace_equal", "Apply full trace equalisation", NUM_VALUES_FIXED );
  pdef->addValue( "1", VALTYPE_NUMBER, "Average output amplitude" );
}

extern "C" void _params_mod_gain_( csParamDef* pdef ) {
  params_mod_gain_( pdef );
}
extern "C" void _init_mod_gain_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_gain_( param, env, log );
}
extern "C" bool _exec_mod_gain_( csTrace* trace, int* port, csExecPhaseEnv* env, csLogWriter* log ) {
  return exec_mod_gain_( trace, port, env, log );
}

