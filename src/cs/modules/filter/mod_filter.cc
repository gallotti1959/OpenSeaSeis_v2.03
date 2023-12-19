/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include "csFlexNumber.h"
#include "csFFTTools.h"
#include <cmath>
#include <cstring>

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: FILTER
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_filter {
  struct VariableStruct {
    float cutoffLow;
    float cutoffHigh;
    bool isLowPass;
    bool isHighPass;
    bool outputImpulse;
    float order;
    csFFTTools* fftTool;
    int filterType;
    bool isPad;
    int paddedSamples;
    int numSamplesInclPad;
    float* bufferPaddedTrace;
  };
  static int const UNIT_HZ   = 1;
  static int const UNIT_PERCENT = 2;
  static int const TYPE_BUTTER = 10;
}
using namespace mod_filter;

//*************************************************************************************************
// Init phase
//
//
//
//*************************************************************************************************
void init_mod_filter_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csExecPhaseDef*   edef = env->execPhaseDef;
  csSuperHeader*    shdr = env->superHeader;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  edef->setExecType( EXEC_TYPE_SINGLETRACE );

  vars->cutoffLow     = 0;
  vars->cutoffHigh    = 0;
  vars->outputImpulse = false;
  vars->isLowPass     = false;
  vars->isHighPass    = false;
  vars->fftTool       = NULL;
  vars->order         = 4;
  vars->filterType = mod_filter::TYPE_BUTTER;

  vars->isPad             = false;
  vars->paddedSamples     = 0;
  vars->numSamplesInclPad = 0;
  vars->bufferPaddedTrace = NULL;

//---------------------------------------------
//
  bool doRestore = false;
  std::string text;
  if( param->exists("mode") ) {
    param->getString("mode", &text );
    if( !text.compare("restore") ) {
      doRestore = true;
    }
    else if( !text.compare("apply") ) {
      doRestore = false;
    }
    else {
      log->error("Unknown option: %s", text.c_str() );
    }
  }

  if( param->exists("type") ) {
    param->getString("type", &text );
    if( !text.compare("butterworth") ) {
      vars->filterType = mod_filter::TYPE_BUTTER;
    }
    else {
      log->error("Unknown option: %s", text.c_str() );
    }
  }

//---------------------------------------------
//
  if( param->exists("pad") ) {
    float padLength;
    param->getFloat("pad", &padLength );
    vars->paddedSamples = (int)round( padLength / shdr->sampleInt );
    vars->numSamplesInclPad = 2 * vars->paddedSamples + shdr->numSamples;
    vars->bufferPaddedTrace = new float[vars->numSamplesInclPad];
    vars->isPad = true;
  }

  //--------------------------------------------------------------------------------
  //
  if( vars->filterType == mod_filter::TYPE_BUTTER ) {
    if( param->exists("highpass") ) {
      param->getFloat("highpass", &vars->cutoffHigh );
      vars->isHighPass = true;
    }

    if( param->exists("lowpass") ) {
      param->getFloat("lowpass", &vars->cutoffLow );
      vars->isLowPass = true;
    }

    if( param->exists("slope") ) {
      float slope;
      param->getFloat("slope", &slope );
      vars->order = fabs(slope) / 6.0;
    }
    if( param->exists("order") ) {
      if( param->exists("slope") ) {
	log->error("Specify only one of the following two user parameters: 'slope' and 'order'");
      }
      param->getFloat("order", &vars->order );
    }
    if( vars->order < 0.1 || vars->order > 100 ) {
      log->error("Specified order = %f out of valid range = 0.1-100", vars->order);
    }
    if( doRestore ) vars->order = -vars->order;

    if( param->exists("impulse") ) {
      std::string text;
      param->getString("impulse", &text);
      if( !text.compare("yes") ) {
        vars->outputImpulse = true;
      }
      else if( !text.compare("no") ) {
        vars->outputImpulse = false;
      }
      else {
        log->error("Unknown option: '%s'", text.c_str() );
      }
    }

    int freqUnit = mod_filter::UNIT_HZ;
    if( param->exists("unit") ) {
      std::string text;
      param->getString("unit", &text);
      if( !text.compare("hz") ) {
        freqUnit = mod_filter::UNIT_HZ;
      }
      else if( !text.compare("percent") ) {
        freqUnit = mod_filter::UNIT_PERCENT;
      }
      else {
        log->error("Unknown option: '%s'", text.c_str() );
      }
    }

    float freqNy = 500.0f/shdr->sampleInt;
    if( freqUnit == mod_filter::UNIT_PERCENT ) {
      if( vars->cutoffHigh < 0 || vars->cutoffHigh > 100 ) {
        log->error("Filter frequency specified as percent (%) exceeds valid range (0-100): %f", vars->cutoffHigh );
      }
      if( vars->cutoffLow < 0 || vars->cutoffLow > 100 ) {
        log->error("Filter frequency specified as percent (%) exceeds valid range (0-100): %f", vars->cutoffLow );
      }
      
      vars->cutoffHigh *= freqNy / 100.0f;
      vars->cutoffLow  *= freqNy / 100.0f;
    }
    if( vars->cutoffHigh < 0 || vars->cutoffHigh > freqNy ) {
      log->error("Filter frequency exceeds valid range (0-%f): %f", freqNy, vars->cutoffHigh );
    }
    if( vars->cutoffLow < 0 || vars->cutoffLow > freqNy ) {
      log->error("Filter frequency exceeds valid range (0-%f): %f", freqNy, vars->cutoffLow );
    }

    if( !vars->isLowPass && !vars->isHighPass ) {
      log->error("No filter option specified. Specify for user parameter 'lowpass' and/or 'highpass'");
    }

    if( !vars->isPad ) {
      vars->fftTool = new csFFTTools( shdr->numSamples, shdr->sampleInt );
    }
    else {
      vars->fftTool = new csFFTTools( vars->numSamplesInclPad, shdr->sampleInt );
    }
  } // END: Setup bandpass filter

}
//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************
bool exec_mod_filter_(
  csTrace* trace,
  int* port,
  csExecPhaseEnv* env, csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>( env->execPhaseDef->variables() );
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader const* shdr = env->superHeader;

  if( edef->isCleanup()){
    if( vars->fftTool != NULL ) {
      delete vars->fftTool;
      vars->fftTool = NULL;
    }
    delete vars; vars = NULL;
    return true;
  }

  float* samples = trace->getTraceSamples();

  // Padding: Pad samples and extrapolate data samples
  if( vars->isPad ) {
    for( int isamp = 0; isamp < vars->numSamplesInclPad; isamp++ ) {
      vars->bufferPaddedTrace[isamp] = 0;
    }
    memcpy( &vars->bufferPaddedTrace[vars->paddedSamples], samples, shdr->numSamples * sizeof(float) );
    // Extrapolate start of trace
    float value2x = 2 * samples[0];
    for( int isamp = vars->paddedSamples-1; isamp >= 0; isamp-- ) {
      vars->bufferPaddedTrace[isamp] = value2x - samples[vars->paddedSamples-isamp];
    }
    // Extrapolate end of trace
    value2x = 2 * samples[shdr->numSamples-1];
    int num = 2*shdr->numSamples+vars->paddedSamples-2;
    for( int isamp = shdr->numSamples+vars->paddedSamples; isamp < vars->numSamplesInclPad; isamp++ ) {
      vars->bufferPaddedTrace[isamp] = value2x - samples[num-isamp];
    }
    samples = vars->bufferPaddedTrace;
  }

  if( vars->filterType == mod_filter::TYPE_BUTTER ) {
    if( vars->isLowPass ) {
      vars->fftTool->lowPass( samples, vars->order, vars->cutoffLow, vars->outputImpulse );
    }
    if( vars->isHighPass ) {
      vars->fftTool->highPass( samples, vars->order, vars->cutoffHigh, vars->outputImpulse );
    }
  }

  // Padding: Remove padded samples
  if( vars->isPad ) {
    memcpy( trace->getTraceSamples(), &samples[vars->paddedSamples], shdr->numSamples * sizeof(float) );    
  }

  return true;
}

//*************************************************************************************************
// Parameter definition
//
//
//*************************************************************************************************
void params_mod_filter_( csParamDef* pdef ) {
  pdef->setModule( "FILTER", "Frequency filter" );

  pdef->addParam( "type", "Filter type", NUM_VALUES_FIXED );
  pdef->addValue( "butterworth", VALTYPE_OPTION );
  pdef->addOption( "butterworth", "Butterworth filter" );

  pdef->addParam( "lowpass", "Lowpass filter", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_NUMBER, "Cutoff frequency for low-pass filter [Hz]",
     "The cutoff frequency will be damped by -3db" );

  pdef->addParam( "highpass", "Highpass filter", NUM_VALUES_FIXED );
  pdef->addValue( "", VALTYPE_NUMBER, "Cutoff frequency for highpass filter [Hz]",
     "The cutoff frequency will be damped by -3db" );

  pdef->addParam( "impulse", "Output filter impulse response", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "yes", "Output filter impulse response, zero phase, placed at trace centre." );
  pdef->addOption( "no", "Do not output filter impulse response. (i.e. output filtered input data)" );

  pdef->addParam( "unit", "Unit of frequency values supplied in parameters", NUM_VALUES_FIXED );
  pdef->addValue( "hz", VALTYPE_OPTION );
  pdef->addOption( "hz", "Frequencies are specified in [Hz]" );
  pdef->addOption( "percent", "Frequencies are specified as percent of Nyquist" );

  pdef->addParam( "order", "Filter 'order'", NUM_VALUES_FIXED, "...can also be specified as slope/oct, see user parameter 'slope'" );
  pdef->addValue( "4", VALTYPE_NUMBER, "Filter order (1-100)" );

  pdef->addParam( "slope", "Filter slope in dB/octave", NUM_VALUES_FIXED, "...can also be specified as filter 'order', see user parameter 'order'" );
  pdef->addValue( "12", VALTYPE_NUMBER, "Filter slope [dB/oct]", "Will be converted into filter order (= 1/6 x slope)" );

  pdef->addParam( "pad", "Pad trace to avoid filter edge effects", NUM_VALUES_FIXED, "Pad and extrapolate trace at top and bottom before filter application. Remove padded samples after filter." );
  pdef->addValue( "0", VALTYPE_NUMBER, "Pad length at top and bottom, in units of trace (for example [ms])" );

  pdef->addParam( "mode", "Mode of operation: Apply specified filter or inverse filter", NUM_VALUES_FIXED );
  pdef->addValue( "apply", VALTYPE_OPTION );
  pdef->addOption( "apply", "Apply filter" );
  pdef->addOption( "restore", "'Restore' filter: Apply inverse filter" );
}

extern "C" void _params_mod_filter_( csParamDef* pdef ) {
  params_mod_filter_( pdef );
}
extern "C" void _init_mod_filter_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_filter_( param, env, log );
}
extern "C" bool _exec_mod_filter_( csTrace* trace, int* port, csExecPhaseEnv* env, csLogWriter* log ) {
  return exec_mod_filter_( trace, port, env, log );
}

