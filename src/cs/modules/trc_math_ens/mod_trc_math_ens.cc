/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include <cmath>
#include "hdr_math_ens_defines.h"
#include "csEquationSolver.h"

using std::string;
using namespace cseis_geolib;
using namespace cseis_system;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: TRC_MATH_ENS
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_trc_math_ens {
  struct VariableStruct {
    int  method;
    int numEns;
    int hdrId_ens;
    int hdrId_dim2;
    int num_dim2;
    mod_trc_math_ens::Ens* ens;
    cseis_system::csTraceGather* gather;
    bool isFirstCall;
    cseis_geolib::csEquationSolver* solver;
    double* userVarValues;
    int  numVariables;
    int* varIndexList;
  };
  static const int METHOD_DEBIAS = 1;
  static const int METHOD_MEAN   = 2;
  static const int METHOD_EQUATION = 3;

}
using namespace mod_trc_math_ens;

void computeEnsemble( int indexEns, cseis_system::csTraceGather* gatherIn, int nTraces, mod_trc_math_ens::Ens* ens, int numSamples, int numDim2, csTraceGather* gatherOut );

//*************************************************************************************************
// Init phase
//
//
//
//*************************************************************************************************
void init_mod_trc_math_ens_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csTraceHeaderDef* hdef = env->headerDef;
  csExecPhaseDef*   edef = env->execPhaseDef;
//  csSuperHeader*    shdr = env->superHeader;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );
  edef->setExecType( EXEC_TYPE_MULTITRACE );

  vars->method   = METHOD_DEBIAS;
  vars->hdrId_ens  = -1;
  vars->hdrId_dim2 = -1;
  vars->numEns = 0;
  vars->num_dim2 = 0;
  vars->ens = NULL;
  vars->gather = NULL;
  vars->isFirstCall = true;
  vars->solver = NULL;
  vars->numVariables = 0;
  vars->userVarValues = NULL;
  vars->varIndexList = NULL;

  //----------------------------------------------------------------
  std::string text;
  param->getString( "method", &text );
  text = toLowerCase( text );
  if( !text.compare("debias") ) {
    vars->method = METHOD_DEBIAS;
    edef->setTraceSelectionMode( TRCMODE_ENSEMBLE, 2 );
  }
  else if( !text.compare("mean") ) {
    vars->method = METHOD_MEAN;
    edef->setTraceSelectionMode( TRCMODE_ENSEMBLE );
  }
  else if( !text.compare("equation") ) {
    vars->method = METHOD_EQUATION;
  }
  else {
    log->line("Method not recognized: %s.", text.c_str());
    env->addError();
  }

  if( vars->method == mod_trc_math_ens::METHOD_MEAN ) {
    param->getString("roll_ens",&text);
    param->getInt("roll_ens",&vars->numEns,1);
    vars->hdrId_ens = hdef->headerIndex(text);
    vars->numEns = 2*vars->numEns + 1;

    param->getString("roll_dim2",&text);
    param->getInt("roll_dim2",&vars->num_dim2,1);
    vars->hdrId_dim2 = hdef->headerIndex(text);
    vars->num_dim2 = 2*vars->num_dim2 + 1;

    vars->ens = new mod_trc_math_ens::Ens(vars->numEns);
    vars->gather = new cseis_system::csTraceGather( hdef );
  }
  else if( vars->method == mod_trc_math_ens::METHOD_EQUATION ) {
    std::string equationText;
    param->getString( "equation", &equationText );

    char* name = new char[3];
    name[2] = '\0';
    int numTraces = 0;
    for( int i = 1; i < 10; i++ ) {
      sprintf(name,"x%1d",i);
      if( equationText.find(name) == string::npos ) {
	break;
      }
      numTraces += 1;
    }
    log->line("Number of trace variables (xN) found in equation: %d", numTraces);
    if( numTraces < 2 ) {
      log->error("Did not find at least two trace variables (x1 & x2) in equation");
    }
    std::string* allTraceNames = new std::string[numTraces];
    for( int i = 0; i < numTraces; i++ ) {
      sprintf(name,"x%1d",i+1);
      allTraceNames[numTraces-i-1] = name;
    }
    vars->solver = new csEquationSolver();
    if( !vars->solver->prepare( equationText, allTraceNames, numTraces ) ) {
      log->error("Error occurred: %s", vars->solver->getErrorMessage().c_str() );
    }
    delete [] allTraceNames;

    csVector<string> constList;
    vars->solver->prepareUserConstants( &constList );

    vars->numVariables = constList.size();
    vars->userVarValues = new double[vars->numVariables];
    vars->varIndexList = new int[vars->numVariables];

    if( vars->numVariables != numTraces ) {
      log->error("Inconsistent number of input traces. Expected %d, found %d\n", vars->numVariables, numTraces);
    }
    edef->setTraceSelectionMode( TRCMODE_FIXED, numTraces );

    for( int ivar = 0; ivar < vars->numVariables; ivar++ ) {
      log->line("Variable #%d: %s", ivar+1, constList.at(ivar).c_str());
      if( constList.at(ivar).at(0) != 'x' ) {
        log->error("Unknown variable name in equation: '%s'. Use variable 'xN' to reference trace N.", constList.at(ivar).c_str() );
        env->addError();
      }
      if( edef->isDebug() ) log->line("Variable #%d: %s", ivar, constList.at(ivar).c_str() );
      char numChar = constList.at(ivar).at(1);
      int traceNum = numChar - 48;
      if( traceNum < 1 || traceNum > numTraces ) {
	log->error("Unknown variable name in equation: '%s'. Use variable 'xN' to reference trace N.", constList.at(ivar).c_str() );
	env->addError();
      }
      vars->varIndexList[ivar] = traceNum-1;
    }
  }


}

//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************
void exec_mod_trc_math_ens_(
                            csTraceGather* traceGather,
                            int* port,
                            int* numTrcToKeep,
                            csExecPhaseEnv* env,
                            csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>( env->execPhaseDef->variables() );
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader const* shdr = env->superHeader;
  //  csTraceHeaderDef const* hdef = env->headerDef;

  if( edef->isCleanup() ) {
    if( vars->solver != NULL ) {
      delete vars->solver; vars->solver = NULL;
    }
    if( vars->userVarValues != NULL ) {
      delete [] vars->userVarValues;
      vars->userVarValues = NULL;
    }
    if( vars->varIndexList != NULL ) {
      delete [] vars->varIndexList;
      vars->varIndexList = NULL;
    }
    if( vars->ens != NULL ) {
      delete vars->ens;
      vars->ens = NULL;
    }
    if( vars->gather != NULL ) {
      delete vars->gather;
      vars->gather = NULL;
    }
    delete vars; vars = NULL;
    return;
  }
  //----------------------------

  int nTraces = traceGather->numTraces();

  if( vars->method == mod_trc_math_ens::METHOD_DEBIAS ) {
    float** tracePtr = new float*[nTraces];
    for( int itrc = 0; itrc < nTraces; itrc++) {
      tracePtr[itrc] = traceGather->trace(itrc)->getTraceSamples();
    }
    for( int isamp = 0; isamp < shdr->numSamples; isamp++) {
      float mean = 0.0;
      for( int itrc = 0; itrc < nTraces; itrc++) {
	mean += tracePtr[itrc][isamp];
      }
      mean /= (float)nTraces;
      for( int itrc = 0; itrc < nTraces; itrc++) {
	tracePtr[itrc][isamp] -= mean;
      }
    }  
    delete [] tracePtr;
  }
  //--------------------------------------------------------------------------------
  else if( vars->method == mod_trc_math_ens::METHOD_MEAN ) {
    double ensValue = traceGather->trace( nTraces-1 )->getTraceHeader()->doubleValue( vars->hdrId_ens );
    if( nTraces > 0 ) {
      vars->ens->addEns( nTraces, ensValue );
      edef->setTracesAreWaiting();
      traceGather->moveTracesTo( 0, nTraces, vars->gather );
    }

    if( vars->ens->numEns() == vars->numEns || edef->isLastCall() ) {
      int indexCenterEns = (int)( vars->ens->numEns()/2 );
      if( vars->isFirstCall ) {
	for( int iens = 0; iens < indexCenterEns; iens++ ) {
	  computeEnsemble( iens, vars->gather, nTraces, vars->ens, shdr->numSamples, vars->num_dim2, traceGather );
	}
      }

      computeEnsemble( indexCenterEns, vars->gather, nTraces, vars->ens, shdr->numSamples, vars->num_dim2, traceGather );
      vars->ens->releaseEns(); // Release first ensemble, not needed anymore
      vars->gather->freeTraces(0,nTraces);

      if( edef->isLastCall() ) {
	for( int iens = indexCenterEns+1; iens < vars->ens->numEns(); iens++ ) {
	  computeEnsemble( iens, vars->gather, nTraces, vars->ens, shdr->numSamples, vars->num_dim2, traceGather );
	}
      }

      // ............................
      vars->isFirstCall = false;
      //      exit(-1);
    }
  }
  else {
    if( vars->numVariables != traceGather->numTraces() ) {
      log->error("Inconsistent number of input traces. Expected %d, found %d\n", vars->numVariables, traceGather->numTraces());
    }
    float* samplesOut = traceGather->trace(0)->getTraceSamples();
    int nSamples = shdr->numSamples;
    for( int isamp = 0; isamp < nSamples; isamp++ ) {
      for( int ivar = 0; ivar < vars->numVariables; ivar++ ) {
	float* samples = traceGather->trace(ivar)->getTraceSamples();
	vars->userVarValues[ vars->varIndexList[ivar] ] = (double)samples[isamp];
      }
      vars->solver->setUserConstants( vars->userVarValues, vars->numVariables );
      samplesOut[isamp] = (float)vars->solver->solve();
    }
    traceGather->freeTraces( 1, traceGather->numTraces()-1 );
  }
}

//********************************************************************************
//
//
void params_mod_trc_math_ens_( csParamDef* pdef ) {
  pdef->setModule( "TRC_MATH_ENS", "Multi-trace sample computation", "Apply multi-trace mathematical equation to sample values" );

  pdef->addParam( "method", "", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_OPTION );
  pdef->addOption( "debias", "Debias trace samples: For each sample time, compute mean value across all traces and remove it from trace sample values" );
  pdef->addOption( "mean", "Rolling mean filter, applied on a sample-by-sample basis" );
  pdef->addOption( "equation", "General math equation. Specify under parameter 'equation'" );

  pdef->addParam( "roll_ens", "", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_STRING, "Header" );
  pdef->addValue( "", VALTYPE_NUMBER, "Number of traces to roll" );

  pdef->addParam( "roll_dim2", "", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_STRING, "Header" );
  pdef->addValue( "", VALTYPE_NUMBER, "Number of traces to roll" );

  pdef->addParam( "equation", "Mathematical equation on N consecutive traces", NUM_VALUES_FIXED, "One trace will be output" );
  pdef->addValue( "", VALTYPE_STRING, "Arbitrary mathematical equation to apply to each individual trace sample. Use 'xN' to reference sample value of trace N.",
		  "Constants: pi,e. Functions: abs,acos,asin,atan,atan2,ceil,cos,cosh,exp,floor,log,log10,max,min,mod,pow,int,round,sin,sinh,sqrt,tan,tanh,todegrees,toradians,sign");
}

extern "C" void _params_mod_trc_math_ens_( csParamDef* pdef ) {
  params_mod_trc_math_ens_( pdef );
}
extern "C" void _init_mod_trc_math_ens_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_trc_math_ens_( param, env, log );
}
extern "C" void _exec_mod_trc_math_ens_( csTraceGather* traceGather, int* port, int* numTrcToKeep, csExecPhaseEnv* env, csLogWriter* log ) {
  exec_mod_trc_math_ens_( traceGather, port, numTrcToKeep, env, log );
}

//********************************************************************************
//
//
void computeEnsemble( int indexEns, csTraceGather* gatherIn, int nTraces, mod_trc_math_ens::Ens* ens, int numSamples, int numDim2, csTraceGather* gatherOut ) {
  int traceIndexFirstCentreEns  = indexEns * nTraces;
  int nTracesOutCurrent = gatherOut->numTraces();

  int numEnsHalf = (int)( ens->numEns()/2 );
  int ensFirst = std::max(0, indexEns - numEnsHalf );
  int ensLast  = std::max( indexEns - numEnsHalf, ens->numEns()-1 );

  // ...for every trace in the ensemble that shall be computed:
  for( int itrc = 0; itrc < nTraces; itrc++ ) {
    // Copy currently processed trace from centre ensemble to output gather	  
    int traceIndexIn = traceIndexFirstCentreEns + itrc;
    gatherIn->copyTraceTo( traceIndexIn, gatherOut );
    // Retrieve pointer to data samples
    float* samples = gatherOut->trace(nTracesOutCurrent+itrc)->getTraceSamples();
    // Zero out current trace samples
    for( int isamp = 0; isamp < numSamples; isamp++ ) {
      samples[isamp] = 0.0;
    }
    int trcFirst = std::max( 0, itrc - numDim2 );
    int trcLast  = std::min( itrc + numDim2, nTraces-1 );
    // ..for every ensemble that shall be averaged for this trace
    for( int iensIn = ensFirst; iensIn <= ensLast; iensIn++ ) {
      int traceIndexCurrentEns = iensIn * nTraces;
      // ..for every trace that shall be averaged for this trace
      for( int itrcIn = trcFirst; itrcIn <= trcLast; itrcIn++ ) {
	float* samplesIn = gatherIn->trace( traceIndexCurrentEns + itrcIn )->getTraceSamples(); 
	for( int isamp = 0; isamp < numSamples; isamp++ ) {
	  samples[isamp] += samplesIn[isamp];
	}
      } // END: for itrcIn
    } // END: for iensIn
    // Normalization:
    float nSummedTraces = (float)( (ensLast-ensFirst+1) * (trcLast-trcFirst+1) );
    for( int isamp = 0; isamp < numSamples; isamp++ ) {
      samples[isamp] /= nSummedTraces;
    }
  } // END: for itrc
}
