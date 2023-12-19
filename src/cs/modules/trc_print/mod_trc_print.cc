/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include <cmath>
#include <string>

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: TRC_PRINT
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_trc_print {
  struct VariableStruct {
    int option;
    cseis_geolib::type_t hdrType_user;
    int hdrId_user;
    bool print_hdr;
    std::string format_time;
    std::string format_value;
    std::string printFormat;
    FILE* fout;
    int traceCounter;
  };
}
using mod_trc_print::VariableStruct;

const int OPTION_LIST  = 10;
const int OPTION_TABLE = 11;

//*************************************************************************************************
// Init phase
//
//
//
//*************************************************************************************************
void init_mod_trc_print_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csExecPhaseDef*   edef = env->execPhaseDef;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  edef->setExecType( EXEC_TYPE_SINGLETRACE );

  vars->option       = 0;
  vars->hdrId_user   = -1;
  vars->hdrType_user = TYPE_UNKNOWN;
  vars->print_hdr    = false;
  vars->fout         = NULL;
  vars->format_time  = "%10.2f";
  vars->format_value = "%16.10e";
  vars->printFormat = "";
  vars->traceCounter = 0;

  if( param->exists("filename") ) {
    string filename;
    param->getString("filename",&filename);
    vars->fout = fopen(filename.c_str(),"w");
  }
  else {
    vars->fout = log->getFile();
  }

  if( param->exists("format") ) {
    param->getString("format", &vars->format_time,0);
    param->getString("format", &vars->format_value,1);
  }
  if( param->exists("header") ) {
    vars->print_hdr  = true;
    string name;
    param->getString("header", &name);
    vars->hdrId_user   = env->headerDef->headerIndex(name);
    vars->hdrType_user = env->headerDef->headerType(name);
  }
  vars->printFormat = "%d  " + vars->format_time + "  " + vars->format_value + "\n";
}

//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************
bool exec_mod_trc_print_(
  csTrace* trace,
  int* port,
  csExecPhaseEnv* env, csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>( env->execPhaseDef->variables() );
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader const* shdr = env->superHeader;

  if( edef->isCleanup()){
    delete vars; vars = NULL;
    return true;
  }

  //  log->line("TRC_PRINT: numSamples: %d, superheader: %d",trace->numSamples(), shdr->numSamples);
  float* samples = trace->getTraceSamples();
  vars->traceCounter += 1;

  if( vars->traceCounter != 1 ) fprintf(vars->fout,"\n");

  if( vars->print_hdr ) {
    if( vars->hdrType_user == TYPE_INT ) {
      string printFormat = "%d  %8d " + vars->format_time + "  " + vars->format_value + "\n";
      int value = trace->getTraceHeader()->intValue( vars->hdrId_user );
      for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
        float time = isamp*shdr->sampleInt;
        fprintf(vars->fout,printFormat.c_str(), isamp, value, time, samples[isamp] );
        //        fprintf(vars->fout,"%d  %8d %10.2f  %16.10e\n", isamp, value, time, samples[isamp] );
      }
    }
    else if( vars->hdrType_user == TYPE_FLOAT || vars->hdrType_user == TYPE_DOUBLE ) {
      string printFormat = "%d  %14.6f " + vars->format_time + "  " + vars->format_value + "\n";
      double value = trace->getTraceHeader()->doubleValue( vars->hdrId_user );
      for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
        float time = isamp*shdr->sampleInt;
        fprintf(vars->fout,printFormat.c_str(), isamp, value, time, samples[isamp] );
        //        fprintf(vars->fout,"%d  %14.6f %10.2f  %16.10e\n", isamp, value, time, samples[isamp] );
      }
    }
    else if( vars->hdrType_user == TYPE_STRING ) {
      string printFormat = "%d  %s " + vars->format_time + "  " + vars->format_value + "\n";
      string value = trace->getTraceHeader()->stringValue( vars->hdrId_user );
      for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
        float time = isamp*shdr->sampleInt;
        fprintf(vars->fout,printFormat.c_str(), isamp, value.c_str(), time, samples[isamp] );
        //        fprintf(vars->fout,"%d  %s %10.2f  %16.10e\n", isamp, value.c_str(), time, samples[isamp] );
      }
    }
  }
  else {
    for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
      float time = isamp*shdr->sampleInt;
      fprintf(vars->fout,vars->printFormat.c_str(), isamp, time, samples[isamp] );
      //      fprintf(vars->fout,"%d  %10.2f  %16.10e\n", isamp, time, samples[isamp] );
    }
  }
  return true;
}

//*************************************************************************************************
// Parameter definition
//
//
//*************************************************************************************************
void params_mod_trc_print_( csParamDef* pdef ) {
  pdef->setModule( "TRC_PRINT", "Print trace samples", "Print trace samples to log file" );

  pdef->addParam( "filename", "Output file name", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_STRING, "Output file name" );

  pdef->addParam( "header", "Trace header to print", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_STRING, "Trace header name" );

  pdef->addParam( "format", "Specify floating point format (C style)", NUM_VALUES_FIXED );
  pdef->addValue( "%10.2f", VALTYPE_STRING, "Format for sample time/frequency" );
  pdef->addValue( "%16.10e", VALTYPE_STRING, "Format for sample value" );
}

extern "C" void _params_mod_trc_print_( csParamDef* pdef ) {
  params_mod_trc_print_( pdef );
}
extern "C" void _init_mod_trc_print_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_trc_print_( param, env, log );
}
extern "C" bool _exec_mod_trc_print_( csTrace* trace, int* port, csExecPhaseEnv* env, csLogWriter* log ) {
  return exec_mod_trc_print_( trace, port, env, log );
}

