/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

namespace mod_test {
  struct VariableStruct {
    int traceCounter;
    int numTracesToSkip;
  };
}
using mod_test::VariableStruct;

//*************************************************************************************************
// Init phase
//
void init_mod_test_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csExecPhaseDef*   edef = env->execPhaseDef;
  //  csSuperHeader*    shdr = env->superHeader;
  VariableStruct* vars   = new VariableStruct();
  edef->setVariables( vars );

  vars->traceCounter    = 0;
  vars->numTracesToSkip = 0;
  edef->setExecType( EXEC_TYPE_SINGLETRACE );

  if( param->exists("skip") ) {
    param->getInt("skip", &vars->numTracesToSkip );
    if( vars->numTracesToSkip < 0 ) {
      log->error("Incorrect entry for number of traces to skip: %d", vars->numTracesToSkip);
    }
  }

  //  int numSamples  = shdr->numSamples;
  //  float sampleInt = shdr->sampleInt;
}

//*************************************************************************************************
// Exec phase
//
bool exec_mod_test_(
  csTrace* trace,
  int* port,
  csExecPhaseEnv* env, csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>(env->execPhaseDef->variables());
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader const* shdr = env->superHeader;

  if( edef->isCleanup()){
    delete vars; vars = NULL;
    return true;
  }

  vars->traceCounter += 1;
  if( vars->traceCounter > vars->numTracesToSkip ) {
    float* samples = trace->getTraceSamples();
    double mean = 0.0;
    for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
      mean += samples[isamp];
    }
    mean /= shdr->numSamples;
    for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
      samples[isamp] -= (float)mean;
    }
    vars->traceCounter = 0;
    return true;
  }
  else {
    return false;
  }
}

//*************************************************************************************************
// Parameter definition
//
void params_mod_test_( csParamDef* pdef ) {
  pdef->setModule( "TEST", "Demonstration single trace module", "Extra description..." );

  pdef->addParam( "skip", "Number of traces to skip", NUM_VALUES_FIXED );
  pdef->addValue( "0", VALTYPE_NUMBER, "Number of traces to skip" );

 // pdef->addParam( "zero", "Reset traces to zero?", NUM_VALUES_FIXED );
 // pdef->addValue( "no", VALTYPE_OPTION );   // Default option = no
 // pdef->addOption( "yes", "Reset traces" );  // An option can be any arbitrary text
 // pdef->addOption( "no", "Do not reset traces" );
}
/*
  if( param->exists("zero") ) {
    std::string text;
    param->getString("zero", &text);
    if( !text.compare("yes") ) {
      vars->doReset = true;
    }
    else if( !text.compare("no") ) {
      vars->doReset = false;
    }
  }
*/
extern "C" void _params_mod_test_( csParamDef* pdef ) {
  params_mod_test_( pdef );
}
extern "C" void _init_mod_test_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_test_( param, env, log );
}
extern "C" bool _exec_mod_test_( csTrace* trace, int* port, csExecPhaseEnv* env, csLogWriter* log ) {
  return exec_mod_test_( trace, port, env, log );
}

