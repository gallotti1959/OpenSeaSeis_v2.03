/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include "csVector.h"
#include "csFlexNumber.h"
#include "geolib_math.h"
#include "geolib_methods.h"
#include "csGeolibUtils.h"
#include <cmath>

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: PZSUM
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_pz_sum {
  struct VariableStruct {
    int input;
    int hdrId_sensor;
    int hdrId_rec_z;
    int method;
    int windowLengthSamples;
    float percent;
    float water_vel;
    int first_sample;
    int last_sample;
    bool doEqualize;
    float zScalar;
  };

  
  static int const METHOD_SUMMATION = 1;
  static int const METHOD_ZERO = 2;
  static int const METHOD_ANALYSIS = 3;

  static int const SENSOR_INDEX_P = 1;
  static int const SENSOR_INDEX_Z = 5;
  static int const SENSOR_INDEX_Z2 = 2;
  static int const NO_VALUE = -999;
}
using namespace mod_pz_sum;

void pz_summation( float* samples_p, float const* samples_z, float percent, int nSamples, int windowLengthSamples, float zScalar, bool doEqualize );
void pz_zero( float* samples_p, float const* samples_z, int nSamples );

//*************************************************************************************************
// Init phase
//
//
//
//*************************************************************************************************
void init_mod_pz_sum_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csTraceHeaderDef* hdef = env->headerDef;
  csExecPhaseDef*   edef = env->execPhaseDef;
  csSuperHeader*    shdr = env->superHeader;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  edef->setExecType( EXEC_TYPE_MULTITRACE );
  edef->setTraceSelectionMode( TRCMODE_FIXED, 2 );

  std::string text;

  vars->water_vel = 1500.0;

  vars->input  = 0;
  vars->hdrId_sensor  = -1;
  vars->hdrId_rec_z   = -1;
  vars->method = 0;
  vars->windowLengthSamples = 0;
  vars->percent      = 0;
  vars->first_sample = 0;
  vars->last_sample  = 0;
  vars->doEqualize   = true;
  vars->zScalar      = 1.0;

//-----------------------------------------------------------------

  param->getString( "method", &text );
  if( !text.compare("sum") ) {
    vars->method = METHOD_SUMMATION;
  }
  else if( !text.compare("subtract") ) {
    vars->zScalar = -1;
    vars->method = METHOD_SUMMATION;
  }
  else if( !text.compare("zero") ) {
    vars->method = METHOD_ZERO;
  }
  else if( !text.compare("analysis") ) {
    vars->method = METHOD_ANALYSIS;
  }
  else {
    log->error("Unknown option '%s'", text.c_str());
  }

  //-------------------------------------------------------------------------
  if( vars->method == METHOD_ANALYSIS ) {
    float startTime = -1.0;
    float endTime   = -2.0;
    param->getFloat( "start_time", &startTime );
    param->getFloat( "end_time", &endTime );
    if( startTime == 0.0 && endTime == 0.0 ) {
      vars->first_sample = 0;
      vars->last_sample  = shdr->numSamples-1;
    }
    else if( startTime > endTime ) {
      log->error("Specified start/end times are invalid. Please check input parameters.");
    }
    else {
      vars->first_sample = (int)(startTime / shdr->sampleInt + 0.5);
      vars->last_sample  = (int)(endTime / shdr->sampleInt + 0.5);
    }

    if( !hdef->headerExists( "rec_z" ) ) {
      log->error("Required trace header 'rec_z' does not exist.");
    }
    vars->hdrId_rec_z  = hdef->headerIndex( "rec_z" );
  }
  else if( vars->method == METHOD_SUMMATION ) {
    if( param->exists( "equalize" ) ) {
      param->getString( "equalize", &text );
      if( !text.compare("yes") ) {
        vars->doEqualize = true;
      }
      else if( !text.compare("no") ) {
        vars->doEqualize = false;
      }
      else {
        log->error("Unknown option '%s'", text.c_str());
      }
    }

    float windowLength = 0.0;
    param->getFloat( "window_length", &windowLength );
    if( windowLength < 0.0 ) {
      log->error("Window length cannot be smaller than 0: %f", windowLength);
    }
    else if( windowLength == 0.0 ) {
      vars->windowLengthSamples = shdr->numSamples;
    }
    else {
      vars->windowLengthSamples = (int)(windowLength/shdr->sampleInt + 0.5);
      if( vars->windowLengthSamples > shdr->numSamples ) {
        log->warning("Specified window length is larger than total number of samples: %f", windowLength);
        vars->windowLengthSamples = shdr->numSamples;
      }
    }

    //-----------------------------------------------------------------
    param->getFloat( "percent", &vars->percent );
    if( vars->percent < 0.0 || vars->percent > 100.0 ) {
      log->error("Percentage cannot be in the range [0,100]: %f", vars->percent);
    }
  }
  
//-------------------------------------------------------------------------
// Check headers
  if( !hdef->headerExists( "sensor" ) ) {
    log->error("Required trace header 'sensor' does not exist.");
  }
  if( hdef->headerType( "sensor" ) != TYPE_INT ) {
    log->error("Trace header sensor has the wrong type: %s. Should be integer type", csGeolibUtils::typeText( hdef->headerType( "sensor" ) ) );
  }  

  vars->hdrId_sensor = hdef->headerIndex( "sensor" );
}

//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************
void exec_mod_pz_sum_(
  csTraceGather* traceGather,
  int* port,
  int* numTrcToKeep,
  csExecPhaseEnv* env,
  csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>( env->execPhaseDef->variables() );
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader const* shdr = env->superHeader;

  if( edef->isCleanup()){
    delete vars; vars = NULL;
    return;
  }

  int nTraces = traceGather->numTraces();
  if( nTraces < 2 ) {
    log->error("Module %s: Incorrect number of input traces. Expected 2 (or more), found %d", nTraces, edef->moduleName().c_str() );
  }

  int sensor;
  int trace_index_p = NO_VALUE;
  int trace_index_z = NO_VALUE;

  for( int itrc = 0; itrc < nTraces; itrc++ ) {
    sensor = traceGather->trace(itrc)->getTraceHeader()->intValue(vars->hdrId_sensor);
    if( sensor == SENSOR_INDEX_P ) {
      if( trace_index_p != NO_VALUE ) {
        log->error("Found more than one sensor P (%d) trace in input trace pair.", SENSOR_INDEX_P );
      }
      trace_index_p = itrc;
    }
    else if( sensor == SENSOR_INDEX_Z || sensor == SENSOR_INDEX_Z2 ) {
      if( trace_index_z != NO_VALUE ) {
        log->error("Found more than one sensor Z (%d/%d) trace in input trace pair.", SENSOR_INDEX_Z, SENSOR_INDEX_Z2 );
      }
      trace_index_z = itrc;
    }
  }
  if( trace_index_p == NO_VALUE ) {
    log->error("Ensemble is missing a sensor P (%d) trace.", SENSOR_INDEX_P );
  }
  if( trace_index_z == NO_VALUE ) {
    log->error("Ensemble is missing a sensor Z (%d/%d) trace.", SENSOR_INDEX_Z, SENSOR_INDEX_Z2 );
  }

//---------------------------------------------------
//  csTraceHeader* trcHdr_p = traceGather->trace(trace_index_p)->getTraceHeader();
//  csTraceHeader* trcHdr_z = traceGather->trace(trace_index_z)->getTraceHeader();

  if( vars->method == METHOD_SUMMATION ) {
    pz_summation(
                 traceGather->trace(trace_index_p)->getTraceSamples(),
                 traceGather->trace(trace_index_z)->getTraceSamples(),
                 vars->percent,
                 shdr->numSamples,
                 vars->windowLengthSamples,
                 vars->zScalar,
                 vars->doEqualize );
  }
  else if( vars->method == METHOD_ZERO ) {
    pz_zero(
            traceGather->trace(trace_index_p)->getTraceSamples(),
            traceGather->trace(trace_index_z)->getTraceSamples(),
            shdr->numSamples );
  }
  else if( vars->method == METHOD_ANALYSIS ) {
    int nTraces = 1;
    float** ptr_p_data = new float*[nTraces];
    float** ptr_z_data = new float*[nTraces];
    for( int itrc = 0; itrc < nTraces; itrc++ ) {
      ptr_p_data[itrc] = &traceGather->trace(trace_index_p)->getTraceSamples()[vars->first_sample];
      ptr_z_data[itrc] = &traceGather->trace(trace_index_z)->getTraceSamples()[vars->first_sample];
    }
    float refl_coef;
    float pz_scalar;
    float wdep = traceGather->trace(0)->getTraceHeader()->floatValue(vars->hdrId_rec_z);

    float z_phase;
    /*    pz_analysis(
                ptr_p_data,
                ptr_z_data,
                nTraces,
                (vars->last_sample-vars->first_sample+1),
                &refl_coef,
                &pz_scalar,
                &wdep,
                &z_phase );  */
    pz_analysis2(
                 ptr_p_data,
                 ptr_z_data,
                 nTraces,
                 (vars->last_sample-vars->first_sample+1),
                 vars->water_vel,
                 &refl_coef,
                 &pz_scalar,
                 &wdep,
                 &z_phase );

    delete [] ptr_p_data;
    delete [] ptr_z_data;
  }
  traceGather->freeTrace(trace_index_z);
}

//*************************************************************************************************
// Parameter definition
//
//
//*************************************************************************************************
void params_mod_pz_sum_( csParamDef* pdef ) {
  pdef->setModule( "PZ_SUM", "PZ combination (STILL EXPERIMENTAL)", "Combine P and Z trace pair. The combined output trace will keep all trace headers from the P trace" );
  pdef->addDoc("Required input: Pairs of P and Z traces.");
  pdef->addDoc("Required trace headers in input data: sensor, rec_z");

  pdef->addParam( "method", "PZ combination method", NUM_VALUES_FIXED );
  pdef->addValue( "sum", VALTYPE_OPTION );
  pdef->addOption( "sum", "Sum P and Z traces in x-t domain, sample by sample." );
  pdef->addOption( "subtract", "Subtract P and Z traces in x-t domain, sample by sample." );
  pdef->addOption( "zero", "Zero samples of opposite polarity." );
  //  pdef->addOption( "analysis", "Analysis. Determine reflection coefficient, PZ scalar, and water depth." );

  pdef->addParam( "window_length", "Length of sliding time window [ms]", NUM_VALUES_FIXED );
  pdef->addValue( "500.0", VALTYPE_NUMBER, "Length of sliding time window [ms] in which normalization factor is computed" );

  pdef->addParam( "start_time", "Start time of analysis window [ms]", NUM_VALUES_FIXED );
  pdef->addValue( "0.0", VALTYPE_NUMBER, "Start time of analysis window [ms]" );

  pdef->addParam( "end_time", "End time of analysis window [ms]", NUM_VALUES_FIXED );
  pdef->addValue( "0.0", VALTYPE_NUMBER, "End time of analysis window [ms]" );

  pdef->addParam( "percent", "Percentage of normalization", NUM_VALUES_FIXED );
  pdef->addValue( "0", VALTYPE_NUMBER, "How much normalization shall be done [%]? 0%: No normalization, 100%: Full normalization, scale Z trace by normalization factor" );

  pdef->addParam( "equalize", "Equalize RMS amplitudes between P & Z trace before summation", NUM_VALUES_FIXED );
  pdef->addValue( "yes", VALTYPE_OPTION );
  pdef->addOption( "yes", "Perform full trace equalization on Z trace before summation." );
  pdef->addOption( "no", "Do not equalize Z trace before summation." );
}

//--------------------------------------------------------------------------------
//
//
void pz_summation( float* samples_p, float const* samples_z, float percent, int nSamples, int windowLengthSamples,
                   float zScalar, bool doEqualize ) { // float const* rms_z_in, float const* rms_p_in ) {
  float pz_weight = percent / 100.0;

  float* rms_z;
  float* rms_p;
  rms_z = new float[nSamples];
  rms_p = new float[nSamples];

  if( rms_z == NULL || rms_p == NULL ) throw( csException("pz_summation: Out of memory") );

  for( int isamp = 0; isamp < nSamples; isamp++ ) {
    rms_z[isamp] = samples_z[isamp] * samples_z[isamp];
    rms_p[isamp] = samples_p[isamp] * samples_p[isamp];
  }

  float ratio_all = 1.0;

  if( doEqualize ) {
    float rms_all_p = 0.0;
    float rms_all_z = 0.0;
    for( int isamp = 0; isamp < nSamples; isamp++ ) {
      rms_all_z += rms_z[isamp];
      rms_all_p += rms_p[isamp];
    }
    rms_all_p = sqrt( rms_all_p / (float)nSamples );
    rms_all_z = sqrt( rms_all_z / (float)nSamples );
    if( rms_all_z != 0.0 ) {
      ratio_all = rms_all_p / rms_all_z;
    }
  }

  zScalar *= ratio_all;

  int windHalf = (int)( windowLengthSamples / 2 );
  int sampWindStart = 0;
  int sampWindEnd   = windowLengthSamples-1;

  // Compute sum of all RMS values over window, for first sample
  double sum_p = 0.0;
  double sum_z = 0.0;
  double ratio = 1.0;
  for( int isampWind = sampWindStart; isampWind <= sampWindEnd; isampWind++ ) {
    sum_p += rms_p[isampWind];
    sum_z += rms_z[isampWind];
  }
  if( sum_z > 0.0 ) {
    ratio = 1.0 + pz_weight * ( sqrt(sum_p/sum_z)/ratio_all - 1.0 );
  }

  //  fprintf( stdout,"Window 0, PZ weight: %10.5f, ratio: %7.5f   %7.5f\n", pz_weight, ratio, ratio_all );
  for( int isamp = 0; isamp < windHalf; isamp++ ) {
    samples_p[isamp] += zScalar * ratio * samples_z[isamp];
  }
  int sampEnd = nSamples-windHalf;
  for( int isamp = windHalf; isamp < sampEnd; isamp++ ) {
    sampWindStart = isamp-windHalf;
    sum_p -= rms_p[sampWindStart];
    sum_z -= rms_z[sampWindStart];
    if( sum_z < 0 ) sum_z = 0.0;
    if( sum_p < 0 ) sum_p = 0.0;
    sampWindStart += 1;
    sampWindEnd   += 1;
    sum_p = sum_p + rms_p[sampWindEnd];
    sum_z = sum_z + rms_z[sampWindEnd];
    if( sum_z > 0.0 ) {
      ratio = 1.0 + pz_weight * ( sqrt(sum_p/sum_z)/ratio_all - 1.0 );
      samples_p[isamp] += zScalar * ratio * samples_z[isamp];
    }
    else {
      ratio = 1.0;
      samples_p[isamp] = 0.0;
    }
    // fprintf( stdout, "%d %d  %d  %f   %f %f  %f %f\n", isamp, sampWindStart, sampWindEnd, ratio, sum_p, sum_z, rms_p[isamp], rms_z[isamp] );
  }
  sampEnd = nSamples-1;
  for( int isamp = nSamples-windHalf; isamp < sampEnd; isamp++ ) {
    samples_p[isamp] += zScalar * ratio * samples_z[isamp];
  }

  delete [] rms_z;
  delete [] rms_p;
}

void pz_zero( float* samples_p, float const* samples_z, int nSamples ) {
  float rms_p = 0.0;
  for( int isamp = 0; isamp < nSamples; isamp++ ) {
    rms_p += samples_p[isamp] * samples_p[isamp];
  }
  rms_p = sqrt( rms_p );

  for( int isamp = 0; isamp < nSamples; isamp++ ) {
    float pz = samples_p[isamp]*samples_z[isamp];
    if( pz < 0.0 ) {  // Only try to recompute pz_weight if last sample has not been reached yet
      samples_p[isamp] = 0.0;
    }
    else {
      samples_p[isamp] = pz;
      if( samples_z[isamp] < 0.0 ) samples_p[isamp] *= -1.0;
    }
  }
  float rms_pz = 0.0;
  for( int isamp = 0; isamp < nSamples; isamp++ ) {
    rms_pz += samples_p[isamp] * samples_p[isamp];
  }
  rms_pz = sqrt( rms_pz );

  if( rms_pz > 0.0 ) {
    float ratio = rms_p / rms_pz;
    for( int isamp = 0; isamp < nSamples; isamp++ ) {
      samples_p[isamp] *= ratio;
    }
  }
}

extern "C" void _params_mod_pz_sum_( csParamDef* pdef ) {
  params_mod_pz_sum_( pdef );
}
extern "C" void _init_mod_pz_sum_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_pz_sum_( param, env, log );
}
extern "C" void _exec_mod_pz_sum_( csTraceGather* traceGather, int* port, int* numTrcToKeep, csExecPhaseEnv* env, csLogWriter* log ) {
  exec_mod_pz_sum_( traceGather, port, numTrcToKeep, env, log );
}

