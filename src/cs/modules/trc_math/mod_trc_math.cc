/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include <cmath>
#include <cstring>
#include "csToken.h"
#include "csVector.h"
#include "csEquationSolver.h"

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: trc_math
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_trc_math {
  struct VariableStruct {
    float value;
    bool isAdd;
    cseis_geolib::csEquationSolver* solver;
    double* userConstantValues;
    int  numVariables;
    int option;
    double dbScalar;
    float* buffer;
  };
  static int const OPTION_NONE     = 0;
  static int const OPTION_EQUATION = 1;
  static int const OPTION_ADD      = 2;
  static int const OPTION_DB       = 4;
  static int const OPTION_FLIP     = 8;
}
using namespace mod_trc_math;

// BUGFIX 080805  TRC_MATH was only working correctly if sample variable 'x' was used once. Fixed now. However, inefficient. Should set data values only once...

//*************************************************************************************************
// Init phase
//
//
//
//*************************************************************************************************
void init_mod_trc_math_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csExecPhaseDef*   edef = env->execPhaseDef;
  csSuperHeader*    shdr = env->superHeader;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  edef->setExecType( EXEC_TYPE_SINGLETRACE );

  vars->value           = 0;
  vars->option          = OPTION_NONE;
  vars->solver          = NULL;
  vars->numVariables    = 0;
  vars->userConstantValues = NULL;
  vars->dbScalar = 10.0;
  vars->buffer = NULL;

  if( param->exists("add") ) {
    vars->option = OPTION_ADD;
    param->getFloat( "add", &vars->value );
  }
  if( param->exists("flip") ) {
    string text;
    param->getString( "flip", &text );
    if( !text.compare("yes") ) {
      vars->option |= OPTION_FLIP;
      vars->buffer = new float[shdr->numSamples];
    }
    else if( text.compare("no") ) {
      log->error("Unknown option: %s", text.c_str());
    }
  }
  if( param->exists("db") ) {
    //    if( vars->option != OPTION_NONE ) log->error("Only one trc_math option can be specified at once.");
    vars->option |= OPTION_DB;
    param->getFloat( "db", &vars->value );
    if( param->getNumValues("db") > 1 ) {
      string text;
      param->getString( "db", &text, 1 );
      if( !text.compare("power") ) {
	vars->dbScalar = 10.0;
      }
      else if( !text.compare("amp") ) {
	vars->dbScalar = 20.0;	
      }
      else {
	log->error("Unknown option: %s", text.c_str());
      }
    }
  }
  if( param->exists("equation") ) {
    //    if( vars->option != OPTION_NONE ) log->error("Only one trc_math option can be specified at once.");
    vars->option |= OPTION_EQUATION;
    std::string equationText;
    param->getString( "equation", &equationText );
    vars->solver = new csEquationSolver();
    string variableName = "x";
    if( !vars->solver->prepare( equationText, &variableName, 1 ) ) {
      log->error("Error occurred: %s", vars->solver->getErrorMessage().c_str() );
    }
    csVector<string> constList;
    vars->solver->prepareUserConstants( &constList );

    vars->numVariables = constList.size();
    vars->userConstantValues = new double[vars->numVariables];
    for( int ivar = 0; ivar < vars->numVariables; ivar++ ) {
      if( constList.at(ivar).compare("x") ) {
        log->error("Unknown variable name in equation: '%s'. Use variable 'x' to reference sample value.", constList.at(ivar).c_str() );
        env->addError();
      }
      if( edef->isDebug() ) log->line("Variable #%d: %s", ivar, constList.at(ivar).c_str() );
    }
  }
}

//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************
bool exec_mod_trc_math_(
  csTrace* trace,
  int* port,
  csExecPhaseEnv* env, csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>( env->execPhaseDef->variables() );
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader const* shdr = env->superHeader;

  if( edef->isCleanup()){
    if( vars->solver != NULL ) {
      delete vars->solver; vars->solver = NULL;
    }
    if( vars->userConstantValues != NULL ) {
      delete [] vars->userConstantValues;
      vars->userConstantValues = NULL;
    }
    if( vars->buffer != NULL ) {
      delete [] vars->buffer; vars->buffer = NULL;
    }
    delete vars; vars = NULL;
    return true;
  }

  float* samples = trace->getTraceSamples();
  int nSamples = shdr->numSamples;

  if( vars->solver != NULL ) {
    if( vars->numVariables == 1 ) {
      for( int isamp = 0; isamp < nSamples; isamp++ ) {
        vars->userConstantValues[0] = (double)samples[isamp];
        vars->solver->setUserConstants( vars->userConstantValues, 1 );
        samples[isamp] = (float)vars->solver->solve();
      }
    }
    else if( vars->numVariables > 1 ) {
      for( int isamp = 0; isamp < nSamples; isamp++ ) {
        for( int ivar = 0; ivar < vars->numVariables; ivar++ ) {
          vars->userConstantValues[ivar] = (double)samples[isamp];
        }
        vars->solver->setUserConstants( vars->userConstantValues, vars->numVariables );
        samples[isamp] = (float)vars->solver->solve();
      }
    }
    else {
      for( int isamp = 0; isamp < nSamples; isamp++ ) {
        samples[isamp] = (float)vars->solver->solve();
      }
    }
  }

  if( (vars->option & OPTION_ADD) != 0 ) {
    for( int isamp = 0; isamp < nSamples; isamp++ ) {
      samples[isamp] += vars->value;
    }
  }
  
  if( (vars->option & OPTION_DB) != 0 ) {
    for( int isamp = 0; isamp < nSamples; isamp++ ) {
      samples[isamp] = vars->dbScalar * log10( samples[isamp] + vars->value );
    }
  }

  if( (vars->option & OPTION_FLIP) != 0 ) {
    for( int isamp = 0; isamp < nSamples; isamp++ ) {
      vars->buffer[nSamples-isamp-1] = samples[isamp];
    }
    memcpy(samples,vars->buffer,nSamples*sizeof(float));
  }
  
  return true;
}

//*************************************************************************************************
// Parameter definition
//
//
//*************************************************************************************************
void params_mod_trc_math_( csParamDef* pdef ) {
  pdef->setModule( "trc_math", "Trace sample computation", "Perform mathematical computations on trace sample values." );

  pdef->addDoc("If more than one user parameter is specified, operations are applied in the order they are defined in this help document, NOT in the order they are supplied in the flow file");

  pdef->addParam( "equation", "Mathematical equation", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_STRING, "Arbitrary mathematical equation to apply to each individual trace sample. Use 'x' to reference original sample value.",
		  "Constants: pi,e. Functions: abs,acos,asin,atan,atan2,ceil,cos,cosh,exp,floor,log,log10,max,min,mod,pow,int,round,sin,sinh,sqrt,tan,tanh,todegrees,toradians,sign");

  pdef->addParam( "add", "Add constant value to sample value", NUM_VALUES_FIXED );
  pdef->addValue( "0", VALTYPE_NUMBER, "Constant value to add to trace sample value" );

  pdef->addParam( "db", "Convert (power) to dB )", NUM_VALUES_VARIABLE );
  pdef->addValue( "0", VALTYPE_NUMBER, "Added noise. Set to other than zero to prevent taking logarithm of zero" );
  pdef->addValue( "power", VALTYPE_OPTION );
  pdef->addOption( "power", "Apply power dB equation: (10*log10(x+noise)" );
  pdef->addOption( "amp", "Apply amplitude dB equation: (20*log10(x+noise)" );

  pdef->addParam( "flip", "Flip trace upside down?", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "no", "Do not flip trace" );
  pdef->addOption( "yes", "Flip trace upside down: Reverse the order of the sample values" );
}

extern "C" void _params_mod_trc_math_( csParamDef* pdef ) {
  params_mod_trc_math_( pdef );
}
extern "C" void _init_mod_trc_math_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_trc_math_( param, env, log );
}
extern "C" bool _exec_mod_trc_math_( csTrace* trace, int* port, csExecPhaseEnv* env, csLogWriter* log ) {
  return exec_mod_trc_math_( trace, port, env, log );
}

