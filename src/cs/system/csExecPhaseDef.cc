/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "csExecPhaseDef.h"
#include "csTraceGather.h"
#include "csTraceHeaderDef.h"
#include "csSuperHeader.h"
#include "cseis_defines.h"
#include "csFlexNumber.h"
#include "csException.h"

using namespace cseis_system;

csExecPhaseDef::csExecPhaseDef( std::string& moduleName ) {
  myModuleName = moduleName;
  numTraces  = 1;
  traceMode  = TRCMODE_UNKNOWN;
  myExecType   = EXEC_TYPE_SINGLETRACE;
  varPtr       = NULL;
  myIsCleanup  = false;
  myIsDebug    = false;
  myTracesAreWaiting = false;
  myIsLastCall  = false;
}
csExecPhaseDef::~csExecPhaseDef() {
}
void csExecPhaseDef::setTraceSelectionMode( int mode ) {
  traceMode = mode;
}
void csExecPhaseDef::setTraceSelectionMode( int mode, int nTraces ) {
  traceMode = mode;
  numTraces = nTraces;
  if( nTraces <= 0 ) {
    throw( cseis_geolib::csException("csExecPhaseDef::setTraceSelectionMode: Incorrect number of traces specified: %d. This is most likely a program bug in the calling function", nTraces) );
  }
}
void csExecPhaseDef::setExecType( int theExecType ) {
  myExecType = theExecType;
}
void csExecPhaseDef::setTracesAreWaiting() {
  myTracesAreWaiting = true;
}
bool csExecPhaseDef::tracesAreWaiting() const {
  return myTracesAreWaiting;
}
bool csExecPhaseDef::isLastCall() const {
  return myIsLastCall;
}

