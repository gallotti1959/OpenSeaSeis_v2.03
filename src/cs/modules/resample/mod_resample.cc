/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include "geolib_methods.h"
#include "csFFTTools.h"
#include <cmath>
#include <cstring>

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: RESAMPLE
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_resample {
  struct VariableStruct {
    int   numSamplesOld;
    float sampleIntOld;  // [ms]
    float* buffer;
    float cutOffHz;
    bool debias;
    int filter;

    cseis_geolib::csFFTTools* fftTool;
  };
  static int const FILTER_FIR    = 11;
  static int const FILTER_BUTTER = 22;
}
using namespace mod_resample;

namespace cseis_geolib {
  void resample( float* samples, int numSamples, float cutOffFreqHz, int order, float sampleInt_ms_in, float sampleInt_ms_out );
}

void create_filter_coef( float cutOffFreqHz, int order, float* coef, int numCoef );

//*************************************************************************************************
// Init phase
//
//
//
//*************************************************************************************************
void init_mod_resample_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csExecPhaseDef*   edef = env->execPhaseDef;
  csSuperHeader*    shdr = env->superHeader;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  edef->setExecType( EXEC_TYPE_SINGLETRACE );

  vars->buffer = NULL;
  vars->debias = false;
  vars->filter = FILTER_BUTTER;

//---------------------------------------------
  vars->numSamplesOld = shdr->numSamples;
  vars->sampleIntOld  = shdr->sampleInt;

  float sampleIntNew;
  param->getFloat( "sample_int", &sampleIntNew );  // [ms]

  if( sampleIntNew <= 0 ) {
    log->error("Non-physical sample interval specified: %fms", sampleIntNew);
  }

  float cutOffRatio = 0.8;
  if( param->exists( "cutoff" ) ) {
    param->getFloat( "cutoff", &cutOffRatio );
    if( cutOffRatio <= 0.0 || cutOffRatio >= 1.0 ) {
      log->error("Given cut-off ratio (=%f) outside of valid range (=0.0-1.0)", cutOffRatio);
    }
  }

  if( param->exists( "debias" ) ) {
    std::string text;
    param->getString( "debias", &text );
    text = toLowerCase( text );
    if( !text.compare( "yes" ) ) {
      vars->debias = true;
    }
    else if( !text.compare( "no" ) ) {
      vars->debias = false;
    }
    else {
      log->line("Unknown argument for user parameter 'debias': '%s'.", text.c_str());
      env->addError();
    }
  }

  if( param->exists( "filter" ) ) {
    std::string text;
    param->getString( "filter", &text );
    text = toLowerCase( text );
    if( !text.compare( "butterworth" ) ) {
      vars->filter = FILTER_BUTTER;
    }
    else if( !text.compare( "fir" ) ) {
      vars->filter = FILTER_FIR;
    }
    else {
      log->line("Unknown option: '%s'.", text.c_str());
      env->addError();
    }
  }

  float order = 10;
  if( param->exists("order") ) {
    param->getFloat("order", &order );
    if( order < 1 || order > 100 ) {
      log->error("Specified order = %.2f out of valid range = 1-100", order);
    }
  }

  int numSamplesNew = shdr->numSamples;
  if( sampleIntNew < shdr->sampleInt ) {
    numSamplesNew = (int)( ((float)vars->numSamplesOld)*(shdr->sampleInt/sampleIntNew) );
    vars->buffer = new float[numSamplesNew];
  }
  else if( sampleIntNew > shdr->sampleInt ) {
    float freqNy      = 500.0/shdr->sampleInt;
    double ratio      = (int)((double)sampleIntNew / (double)shdr->sampleInt );
    int ratio_int = (int)round( ratio );
    int m;
    int twopm;
    csFFTTools::Powerof2( ratio_int, &m, &twopm );
    if( twopm != ratio_int ) {
      ratio = twopm * 2;
    }
    else {
      ratio = ratio_int;
    }
    sampleIntNew = shdr->sampleInt*ratio;
    log->warning("Output sample interval was set to %f. Current implementation requires new_sample_int to be N*old_sample_int, where N=2^M", sampleIntNew);
    float cutOffFreq  = ( freqNy / ratio ) * cutOffRatio;
    numSamplesNew     = (int)ceil((double)vars->numSamplesOld / ratio);  // Workaround BUGFIX 100504: Avoids clash for num samples = 2^N+1
    vars->fftTool     = new csFFTTools( vars->numSamplesOld, numSamplesNew, vars->sampleIntOld, sampleIntNew );
    vars->fftTool->setFilter( order, cutOffFreq, false );

    log->line("Sample interval: %f ms\nNyquist: %f Hz\nCut-off frequency: %f Hz\nRatio: %f\nOrder: %d\n", shdr->sampleInt, freqNy, cutOffFreq, ratio, order );
    if( edef->isDebug() ) {
      fprintf(stderr,"SampleInt: %f, Nyquist: %f, cutOff: %f, ratio: %f\n", shdr->sampleInt, freqNy, cutOffFreq, ratio );
    }
  }
  else {
    log->warning("Specified sample interval of %fms is the same as in the input data.\nData will stay unchanged.", sampleIntNew );
  }

  shdr->numSamples = numSamplesNew;
  shdr->sampleInt  = sampleIntNew;

  if( edef->isDebug() ) {
    float ratio = shdr->sampleInt/vars->sampleIntOld;
    int ratio_int = (int)round( ratio );
    log->line("nSamples (old,new): %d %d, sampleInt (old,new): %8.4f %8.4f, ratio: %f (%d)",
              vars->numSamplesOld, shdr->numSamples, vars->sampleIntOld, shdr->sampleInt, ratio, ratio_int);
    fprintf(stderr,"nSamples (old,new): %d %d, sampleInt (old,new): %8.4f %8.4f, ratio: %f (%d)\n",
            vars->numSamplesOld, shdr->numSamples, vars->sampleIntOld, shdr->sampleInt, ratio, ratio_int);
  }
}

//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************
bool exec_mod_resample_(
  csTrace* trace,
  int* port,
  csExecPhaseEnv* env, csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>( env->execPhaseDef->variables() );
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader const* shdr = env->superHeader;

  if( edef->isCleanup()){
    if( vars->buffer != NULL ) {
      delete [] vars->buffer;
      vars->buffer = NULL;
    }
    if( vars->fftTool != NULL ) {
      delete vars->fftTool;
      vars->fftTool = NULL;
    }
    delete vars; vars = NULL;
    return true;
  }

  float* samples = trace->getTraceSamples();
  int numSamplesNew = shdr->numSamples;

  if( shdr->sampleInt < vars->sampleIntOld ) {
    for( int isamp = 0; isamp < numSamplesNew; isamp++ ) {
      double sampleIndexOld = (double)(isamp*shdr->sampleInt) / vars->sampleIntOld;
      vars->buffer[isamp] = getQuadAmplitudeAtSample( samples, sampleIndexOld, vars->numSamplesOld );
    }
    memcpy( samples, vars->buffer, numSamplesNew*sizeof(float) );
  }
  else if( shdr->sampleInt > vars->sampleIntOld ) {
    if( !vars->debias ) {
      vars->fftTool->resample( samples );
    } // Remove DC bias first
    else {
      double sum = 0.0;
      int sampleCounter = 0;
      for( int isamp = 0; isamp < vars->numSamplesOld; isamp++ ) {
        if( samples[isamp] != 0 ) {
          sampleCounter += 1;
          sum += samples[isamp];
        }
      }
      float mean = sum / (double)sampleCounter;
      for( int isamp = 0; isamp < vars->numSamplesOld; isamp++ ) {
        if( samples[isamp] != 0 ) {
          samples[isamp] -= mean;
        }
      }

      vars->fftTool->resample( samples );

      // Reapply DC bias afterwards
      for( int isamp = 0; isamp < shdr->numSamples; isamp++ ) {
        if( samples[isamp] != 0 ) {
          samples[isamp] += mean;
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
void params_mod_resample_( csParamDef* pdef ) {
  pdef->setModule( "RESAMPLE", "Resample trace to different sample interval (LIMITED FUNCTIONALITY!)" );

  pdef->addParam( "sample_int", "New sample interval [ms]", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_NUMBER, "Sample interval [ms]" );

  pdef->addParam( "debias", "Remove DC bias before resampling?", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "yes", "Remove DC bias before resampling, reapply afterwards.", "This should be done to avoid FFT related artefacts due to DC bias" );
  pdef->addOption( "no", "Do not remove DC bias before resampling." );

  pdef->addParam( "order", "Filter 'order'", NUM_VALUES_FIXED, "The filter 'order' defines the steepness of the filter taper. The higher the order, the shorter the taper." );
  pdef->addValue( "10", VALTYPE_NUMBER, "Filter order (1-100)" );

  pdef->addParam( "cutoff", "Cut-off (-3db) frequency", NUM_VALUES_FIXED );
  pdef->addValue( "0.8", VALTYPE_NUMBER, "Cut-off frequency, given as ratio of Nyquist (0.0-1.0)", "For example: 0.9 --> cut-off frequency is 90% of Nyquist" );

  //  pdef->addParam( "filter", "Anti-alias filter method to use", NUM_VALUES_FIXED );
  //  pdef->addValue( "butterworth", VALTYPE_OPTION );
  //  pdef->addOption( "butterworth", "Standard butterworth filter" );
  //  pdef->addOption( "fir", "Standard fir x-t filter" );
}


extern "C" void _params_mod_resample_( csParamDef* pdef ) {
  params_mod_resample_( pdef );
}
extern "C" void _init_mod_resample_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_resample_( param, env, log );
}
extern "C" bool _exec_mod_resample_( csTrace* trace, int* port, csExecPhaseEnv* env, csLogWriter* log ) {
  return exec_mod_resample_( trace, port, env, log );
}

/*
 * Create filter coefficients for standard FIR low-pass filter
 *
 * h(n) = w/pi  * sin(w*m)/(w*m)
 * m = (n - N/2)
 * N: Order
 * w = 2*pi*f
 * f = cut-off frequency
 */
void create_filter_coef( float cutOffFreqHz, int order, float* coef, int numCoef ) {
  double angularFreq = cutOffFreqHz * M_PI * 2.0;
  double orderHalf = (double)order / 2.0;

  for( int i = 0; i < order; i++ ) {
    double id = (double)i;
    coef[i] = (float)( sin( angularFreq * ( id - orderHalf ) ) / (M_PI * ( id - orderHalf )) );
  }
  for( int i = order+1; i < numCoef; i++ ) {
    double id = (double)i;
    coef[i] = (float)( sin( angularFreq * ( id - orderHalf ) ) / (M_PI * ( id - orderHalf )) );
  }
  coef[order] = (float)( 2.0*cutOffFreqHz );

  // Apply Hamming window:
  for( int i = 0; i < numCoef; i++ ) {
    coef[i] *= (float)( ( 0.54 - 0.46 * cos(2.0*M_PI*(double)i/(double)numCoef) ) );
  }
}


