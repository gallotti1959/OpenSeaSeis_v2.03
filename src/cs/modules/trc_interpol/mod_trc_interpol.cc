/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: TRC_INTERPOL
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_trc_interpol {
  struct VariableStruct {
    int method;
  };
  static int const METHOD_SIMPLE_AVERAGE = 1;
}
using namespace mod_trc_interpol;

//*******************************************************************
//
//  Init phase
//
//*******************************************************************

void init_mod_trc_interpol_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
//  csTraceHeaderDef* hdef = env->headerDef;
//  csSuperHeader*    shdr = env->superHeader;
  csExecPhaseDef*   edef = env->execPhaseDef;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  vars->method = METHOD_SIMPLE_AVERAGE;
  
  edef->setExecType( EXEC_TYPE_MULTITRACE );
  edef->setTraceSelectionMode( TRCMODE_FIXED, 2 );

}

//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************

void exec_mod_trc_interpol_(
  csTraceGather* traceGather,
  int* port,
  int* numTrcToKeep,
  csExecPhaseEnv* env,
  csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>( env->execPhaseDef->variables() );
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader const* shdr = env->superHeader;
  csTraceHeaderDef const* hdef = env->headerDef;

  if( edef->isCleanup()){
    delete vars; vars = NULL;
    return;
  }

  if( edef->isDebug() ) log->line("Number of input traces: %d, num trc to keep: %d", traceGather->numTraces(), *numTrcToKeep);

  if( traceGather->numTraces() != 2 ) {  // This must be the last trace of the data set. Simply return, do nothing
    return;
  }

  int nSamples = shdr->numSamples;
  // 1) Create new trace in between the two traces
  traceGather->createTraces( 1, 1, hdef, nSamples );

  // 2) Interpolate trace samples
  float* samples1 = traceGather->trace(0)->getTraceSamples();
  float* samples2 = traceGather->trace(2)->getTraceSamples();
  float* samplesNew = traceGather->trace(1)->getTraceSamples();
  
  if( vars->method == METHOD_SIMPLE_AVERAGE ) {
    for( int isamp = 0; isamp < nSamples; isamp++ ) {
      samplesNew[isamp] = (samples1[isamp] + samples2[isamp])*0.5;
    }
  }

  // 2) Interpolate trace header values
  csTraceHeader* trcHdr1 = traceGather->trace(0)->getTraceHeader();
  csTraceHeader* trcHdr2 = traceGather->trace(2)->getTraceHeader();
  csTraceHeader* trcHdrNew = traceGather->trace(1)->getTraceHeader();

//  double position = 0.5;
  for( int ihdr = 0; ihdr < hdef->numHeaders(); ihdr++ ) {
    char type = hdef->headerType(ihdr);
    switch( type ) {
      case TYPE_INT:
        trcHdrNew->setIntValue( ihdr, (trcHdr1->intValue(ihdr)+trcHdr2->intValue(ihdr))/2 );
        break;
      case TYPE_FLOAT:
        trcHdrNew->setFloatValue( ihdr, (trcHdr1->floatValue(ihdr)+trcHdr2->floatValue(ihdr))/2 );
        break;
      case TYPE_DOUBLE:
        trcHdrNew->setDoubleValue( ihdr, (trcHdr1->doubleValue(ihdr)+trcHdr2->doubleValue(ihdr))/2 );
        break;
      case TYPE_INT64:
        trcHdrNew->setInt64Value( ihdr, (trcHdr1->int64Value(ihdr)+trcHdr2->int64Value(ihdr))/2 );
        break;
      case TYPE_STRING:
        trcHdrNew->setStringValue( ihdr, trcHdr1->stringValue(ihdr) );
        break;
      default:
        log->error("mod_trc_interpol: Unknown trace header type, code: %d", type);
    }
  }

  *numTrcToKeep = 1;  // Keep last trace for next pair of traces to interpolate in between
}

//*************************************************************************************************
// Parameter definition
//
//
//*************************************************************************************************
void params_mod_trc_interpol_( csParamDef* pdef ) {
  pdef->setModule( "TRC_INTERPOL", "Interpolate traces (EXPERIMENTAL MODULE)", "Interpolate 1 trace between every two adjacent traces, e.g. if 10 traces are input, 19 traces will be output" );

  pdef->addParam( "method", "", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_OPTION );
  pdef->addOption( "simple_average", "Interpolate new trace by simply averaging adjacent traces. Header values are averaged." );
}

extern "C" void _params_mod_trc_interpol_( csParamDef* pdef ) {
  params_mod_trc_interpol_( pdef );
}
extern "C" void _init_mod_trc_interpol_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_trc_interpol_( param, env, log );
}
extern "C" void _exec_mod_trc_interpol_( csTraceGather* traceGather, int* port, int* numTrcToKeep, csExecPhaseEnv* env, csLogWriter* log ) {
  exec_mod_trc_interpol_( traceGather, port, numTrcToKeep, env, log );
}

