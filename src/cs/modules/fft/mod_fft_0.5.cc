/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include "csFFTTools.h"
#include "csGeolibUtils.h"
#include <cmath>
#include <string>

using namespace cseis_system;
using namespace cseis_geolib;
using namespace std;

/**
 * CSEIS - Seabed Seismic Processing System
 * Module: FFT
 *
 * @author Bjorn Olofsson
 * @date   2007
 */
namespace mod_fft {
  struct VariableStruct {
    bool dump;           // True if FFT result shall be dumped to log
    float T_interval;    // Interval delta T. 1/T is the sampling interval in the frequency domain
    int direction;       // Direction of transform: 'forward' or 'reverse'
    int sampleIntIn;     // Sample interval in input trace
    int numSamplesIn;
//    double* bufferReal;  // Buffer for real part (of length numFFTSamples)
//    double* bufferImag;  // Buffer for imaginary part (of length numFFTSamples)
    int taperType;
    int taperLengthInSamples;     // Taper length in number of samples (from 0 to 1)
    int output;
    bool normalize;      // Normalize data: Yes or no?
    double normScalar;   // Normalisation scalar to be applied before inverse FFT.
                         // This scalar accounts for zeros that may have been padded to input trace
    cseis_geolib::csFFTTools* fftTool;
    int fftDataType;
    bool outputEven;
  };
  static const int TAPER_NONE    = -1;
  static const int TAPER_COSINE  = 1;
  static const int TAPER_HANNING = 2;
  static const int TAPER_BLACKMAN = 3;
}
using namespace mod_fft;

//*************************************************************************************************
// Init phase
//
//
//
//*************************************************************************************************
void init_mod_fft_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
//  csTraceHeaderDef* hdef = env->headerDef;
  csExecPhaseDef*   edef = env->execPhaseDef;
  csSuperHeader*    shdr = env->superHeader;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  edef->setExecType( EXEC_TYPE_SINGLETRACE );

  vars->numSamplesIn   = shdr->numSamples;
  vars->dump           = false;
  vars->T_interval     = 0;
  vars->direction      = csFFTTools::FORWARD;
  vars->sampleIntIn    = shdr->sampleInt;
  vars->taperType      = TAPER_NONE;
  vars->taperLengthInSamples = 10;
  vars->fftDataType       = FX_NONE;
  vars->normScalar     = 1.0;
  vars->normalize      = false;
  vars->fftTool        = NULL;
  vars->outputEven     = false;

//-------------------------------------------

  std::string text;
  if( param->exists("dump") ) {
    param->getString("dump", &text);
    if( !text.compare("yes") ) {
      vars->dump = true;
    }
    else if( !text.compare("no") ) {
      vars->dump = false;
    }
    else {
      log->error("Unknown option: %s", text.c_str());
    }
  }
  bool override_domain = false;
  if( param->exists("override") ) {
    param->getString("override", &text);
    if( !text.compare("yes") ) {
      override_domain = true;
    }
    else if( !text.compare("no") ) {
      override_domain = false;
    }
    else {
      log->error("Unknown option: %s", text.c_str());
    }
  }

  if( param->exists("norm") ) {
    param->getString("norm", &text);
    if( !text.compare("yes") ) {
      vars->normalize = true;
    }
    else if( !text.compare("no") ) {
      vars->normalize = false;
    }
    else {
      log->error("Unknown option: %s", text.c_str());
    }
  }

//-------------------------------------------
  if( param->exists("taper_type") ) {
    param->getString("taper_type", &text);
    if( !text.compare("none") ) {
      vars->taperType = TAPER_NONE;
    }
    else if( !text.compare("cos") ) {
      vars->taperType = TAPER_COSINE;
    }
    else if( !text.compare("hanning") ) {
      vars->taperType = TAPER_HANNING;
      vars->taperLengthInSamples = shdr->numSamples/2;
    }
    else if( !text.compare("blackman") ) {
      vars->taperType = TAPER_BLACKMAN;
      vars->taperLengthInSamples = shdr->numSamples/2;
    }
    else {
      log->error("Unknown option: %s", text.c_str());
    }
  }
  if( param->exists("taper_len") ) {
    if( vars->taperType == TAPER_HANNING ) {
      log->error("Cannot specify user defined taper length. For Hanning taper, taper length is fixed to half the trace length");
    }
    else {
      param->getInt("taper_len", &vars->taperLengthInSamples);
    }
  }

//-------------------------------------------
  if( param->exists("direction") ) {
    param->getString( "direction", &text );
    if( !text.compare("forward") ) {
      vars->direction = csFFTTools::FORWARD;
    }
    else if( !text.compare("inverse") ) {
      vars->direction = csFFTTools::INVERSE;
    }
    else {
      log->error("Unknown option: %s", text.c_str());
    }
  }

//-------------------------------------------

  if( param->exists("output") ) {
    if( vars->direction == csFFTTools::INVERSE ) {
      log->warning("User parameter 'output' will be ignored for inverse FFT transform");
    }
    else {
      param->getString("output", &text);
      if( !text.compare("amp_phase") ) {
        vars->fftDataType = FX_AMP_PHASE;
      }
      else if( !text.compare("amp") ) {
        vars->fftDataType = FX_AMP;
      }
      else if( !text.compare("psd") ) {
        vars->fftDataType = FX_PSD;
      }
      else if( !text.compare("psd_even") ) {
        vars->fftDataType = FX_PSD;
        vars->outputEven = true;
      }
      else if( !text.compare("real_imag") ) {
        vars->fftDataType = FX_REAL_IMAG;
      }
      else {
        log->error("Unknown option: %s", text.c_str());
      }
    }
  }
  else if( vars->direction == csFFTTools::FORWARD ) {
    log->error("User parameter 'output' is required for forward FFT transform");
  }
  //-------------------------------------------
  // Set trace parameters depending on FORWARD or INVERSE transforms
  //

  vars->numSamplesIn = shdr->numSamples;
  if( vars->direction == csFFTTools::FORWARD ) {
    if( shdr->domain != DOMAIN_XT && shdr->domain != DOMAIN_XD && !override_domain ) {
      log->error("Current trace is not in XT/XD domain. FFT forward transform not possible. Actual domain: %s (%d)", csGeolibUtils::domain2Text(shdr->domain), shdr->domain );
    }

    vars->fftTool = new csFFTTools( shdr->numSamples );
    int numFFTSamples = vars->fftTool->numFFTSamples();
    //Frequency interval in output spectrum: 1/T = 1/131.0720   =   0.007629; Max freq =     500.00
    //Perform FFT transform. Total number of samples to transform: 131072

    shdr->domain       = DOMAIN_FX;
    shdr->sampleIntXT  = shdr->sampleInt;
    shdr->numSamplesXT = shdr->numSamples;
    vars->T_interval  = (float)numFFTSamples*shdr->sampleIntXT/1000.0;
    shdr->sampleInt    = 1.0/vars->T_interval;

    if( vars->fftDataType == FX_AMP_PHASE ) {
      shdr->numSamples = numFFTSamples + 2;  // numFFTSamples/2+1 samples for both amplitude and phase
    }
    else if( vars->fftDataType == FX_AMP || vars->fftDataType == FX_PSD ) {
      shdr->numSamples = numFFTSamples/2+1;
      if( vars->outputEven ) shdr->numSamples = numFFTSamples/2;
    }
    else if( vars->fftDataType == FX_REAL_IMAG ) {
      shdr->numSamples = 2*numFFTSamples;
    }
    else {
      log->error("Unknown FFT order (%d). This is most likley a program bug in the FFT module", vars->fftDataType );
    }

    shdr->fftDataType = vars->fftDataType;
  }
  //--------------------------------------------------------------------------------
  // Inverse FFT
  //
  else if( vars->direction == csFFTTools::INVERSE ) {
    int numFFTSamples = 0;
    if( shdr->domain != DOMAIN_FX ) {
      log->error("Current trace is not in FX domain. FFT inverse transform not possible. Actual domain: %s (%d)", csGeolibUtils::domain2Text(shdr->domain), shdr->domain );
    }
    else if( shdr->fftDataType == FX_AMP_PHASE ) {
      numFFTSamples = shdr->numSamples - 2;  // numFFTSamples/2+1 samples in both amplitude and phase
    }
    else if( shdr->fftDataType == FX_REAL_IMAG ) {
      numFFTSamples = shdr->numSamples/2;
    }
    else { // if( shdr->fftDataType != FX_REAL_IMAG && shdr->fftDataType != FX_AMP_PHASE ) {
      string text = "UNKNOWN";
      if( shdr->fftDataType == FX_AMP ) text = "AMP";
      else if( shdr->fftDataType == FX_PSD ) text = "PSD";
      log->error("Cannot perform inverse transform due to lack of information: Data is neither of type AMP_PHASE or REAL_IMAG. Actual type defined in super header: %s", text.c_str());
    }

    if( edef->isDebug() ) {
      log->line("Number of samples: Input/XT samples/Computed FFT samples: %d %d %d", 
                shdr->numSamples, shdr->numSamplesXT, numFFTSamples );
    }

    vars->fftDataType = shdr->fftDataType;
    if( vars->normalize ) vars->normScalar = (double)shdr->numSamplesXT/(double)shdr->numSamples;
    if( edef->isDebug() ) log->line("FFT Inverse norm scalar: %f, num XT samples: %d, num FFT samples: %d\n", vars->normScalar, shdr->numSamplesXT, shdr->numSamples );
    shdr->numSamples   = shdr->numSamplesXT;
    shdr->sampleInt    = shdr->sampleIntXT;
    shdr->numSamplesXT = 0;
    shdr->sampleIntXT  = 0;

    vars->fftTool = new csFFTTools( numFFTSamples );

    if( numFFTSamples != vars->fftTool->numFFTSamples() ) {
      log->error("Incorrect number of input samples. Expected: %d, found: %d", vars->fftTool->numFFTSamples(), numFFTSamples );
    }
    shdr->domain     = DOMAIN_XT;
    shdr->fftDataType   = FX_NONE;
    vars->T_interval = (float)numFFTSamples*shdr->sampleInt/1000.0;
  }

  shdr->dump( log->getFile() );

  log->line("Perform FFT transform. Total number of samples to transform:  %d", vars->fftTool->numFFTSamples() );
  log->line("Number of input/output samples:  %d / %d, Number of XT samples saved in super header: %d",
            vars->numSamplesIn, shdr->numSamples, shdr->numSamplesXT );
  log->line("Frequency interval in output spectrum: 1/T = 1/%-10.4f = %10.6f; Max freq = %10.2f",
            vars->T_interval, 1/vars->T_interval, (float)vars->fftTool->numFFTSamples()/(2.0*vars->T_interval) );
}

//*************************************************************************************************
// Exec phase
//
//
//
//*************************************************************************************************
bool exec_mod_fft_(
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

  //----------------------------------------------------------------------------
  //----------------------------------------------------------------------------
  //
  // Forward transform
  //
  //
  if( vars->direction == csFFTTools::FORWARD ) {
    // (1) Apply taper to input trace
    if( vars->taperType == TAPER_COSINE || vars->taperType == TAPER_HANNING ) {
      int nSteps = vars->taperLengthInSamples;
      for( int i = 0; i < nSteps; i++ ) {
        float scalar = cos( M_PI_2 * (float)(nSteps-i)/(float)nSteps );
        samples[i] *= scalar;
      }
      for( int i = vars->numSamplesIn-nSteps; i < vars->numSamplesIn; i++ ) {
        float scalar = cos( M_PI_2 * (float)(nSteps-vars->numSamplesIn+i+1)/(float)nSteps );
        samples[i] *= scalar;
      }
    }
    else if( vars->taperType == TAPER_BLACKMAN ) {
      float alpha = 0.16;
      float a0 = 0.5 * (1.0 - alpha);
      float a1 = 0.5;
      float a2 = 0.5 * alpha;
      for( int i = 0; i < vars->numSamplesIn; i++ ) {
        float piFactor = (2.0 * M_PI) * (float)i / (float)(vars->numSamplesIn - 1);
        float weight = a0 - a1*cos( piFactor ) + a2*cos( 2 * piFactor );
        samples[i] *= weight;
      }
    }

    //-----------------------------------------------------------
    // Store amplitude & phase spectra
    //
    // samples[0]: 0Hz
    // samples[1]: 1/T Hz...
    // samples[numFFTSamples/2]: N/2T Hz (= Nyquist)
    // samples[numFFTSamples/2+1]: Phase 1/T Hz, ...(N-1)/2T Hz,  T = sampleInt*numFFTSamples
    // samples[numFFTSamples-1]: Phase (N-1)/T Hz
    //  where...
    //    T = sampleInt*numFFTSamples
    //
    // NOTE: Amplitudes for positive freq amplitude spectrum have to be multiplied by 2 for all frequencies except 0Hz and Nyquist.
    //

    double normSpectralDensity = 1.0;

    bool success = false;
    int numFFTSamples  = vars->fftTool->numFFTSamples();
    if( vars->fftDataType == FX_REAL_IMAG || vars->fftDataType == FX_PSD ) {
      success = vars->fftTool->fft_forward( samples );
      if( vars->fftDataType == FX_PSD ) {
        double const* real = vars->fftTool->realData();
        double const* imag = vars->fftTool->imagData();
        int numValues = numFFTSamples/2;
        if( !vars->outputEven ) {
          numValues += 1;
        }
        normSpectralDensity = 1.0;
        if( vars->normalize ) {
          int numSamplesNonZero = 0;
          for( int i = 0; i < vars->numSamplesIn; i++ ) {
            if( samples[i] != 0.0 ) numSamplesNonZero += 1;
          }
          if( numSamplesNonZero == 0 ) numSamplesNonZero = 1;
          normSpectralDensity = 1.0 / (numSamplesNonZero * (1000.0/vars->sampleIntIn));
        }
        // Multiply complex specturm with conjugate complex = (R + I) * (R - I)
        for( int is = 0; is < numValues; is++ ) {
          samples[is] = 2.0 * ( real[is]*real[is] + imag[is]*imag[is] ) * normSpectralDensity;
        }
        //if( !vars->outputEven ) {
        //  samples[numFFTSamples/2] = sqrt( real[numFFTSamples/2]*real[numFFTSamples/2] * normSpectralDensity; // Nyquist
        //}
      }
      else { //  if( vars->fftDataType == FX_REAL_IMAG ) {
        if( vars->normalize ) {
          normSpectralDensity = 1.0 / sqrt( (double)vars->numSamplesIn );
        }
        double const* real = vars->fftTool->realData();
        double const* imag = vars->fftTool->imagData();
        for( int is = 0; is < numFFTSamples; is++ ) {
          samples[is]               = (float)(real[is]*normSpectralDensity);
          samples[is+numFFTSamples] = (float)(imag[is]*normSpectralDensity);
        }
      }
    }
    else {
      int numSamplesNormalise = shdr->numSamples;
      if( vars->fftDataType == FX_AMP_PHASE ) {
        success = vars->fftTool->fft_forward( samples, &samples[0], &samples[numFFTSamples/2+1], false );
        numSamplesNormalise = numFFTSamples/2;
      }
      else if( vars->fftDataType == FX_AMP ) {
        success = vars->fftTool->fft_forward( samples, &samples[0], false );
      }
      if( vars->normalize ) {
        normSpectralDensity = 1.0 / sqrt( (double)vars->numSamplesIn );
        for( int is = 0; is < numSamplesNormalise; is++ ) {
          samples[is] *= normSpectralDensity;
        }
      }
    }
    if( !success ) {
      log->warning("FFT transform failed for unknown reasons...");
      return false;
    }
  }
  //------------------------------------------------------------------------------------
  // Inverse transform.
  // Assumes that forward transform has been applied previously, using option REAL_IMAGE or AMP_PHASE
  //
  else {  // INVERSE
    bool success = vars->fftTool->fft_inverse( samples, vars->fftDataType );
    if( !success ) {
      log->warning("FFT transform failed for unknown reasons...");
      return false;
    }
    
    double const* real = vars->fftTool->realData();
    for( int i = 0; i < shdr->numSamples; i++ ) {
      samples[i] = real[i];
    }
  }
  return true;
}

//*************************************************************************************************
// Parameter definition
//
//
//*************************************************************************************************
void params_mod_fft_( csParamDef* pdef ) {
  pdef->setModule( "FFT", "FFT transform", "Transform input data from x-t to x-w domain (forward) or vice versa (inverse)" );
  pdef->setVersion(0,5);

  pdef->addParam( "direction", "Direction of transform", NUM_VALUES_FIXED );
  pdef->addValue( "forward", VALTYPE_OPTION );
  pdef->addOption( "forward", "Forward transform from x-t to x-w" );
  pdef->addOption( "inverse", "Inverse transform from x-w to x-t.",
    "Inverse transform will only work if a forward transform was applied before" );

  pdef->addParam( "dump", "Dump FFT values to log file", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "yes", "Dump FFT values to log file" );
  pdef->addOption( "no", "Do not dump FFT values" );

  pdef->addParam( "norm", "Normalize output", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "yes", "Normalize output values", "Example: Using the same input data but with different amount of added zeros, the amplitude spectrum will look exactly the same for the same output frequency" );
  pdef->addOption( "no", "Do not normalize output values" );

  pdef->addParam( "taper_type", "Taper type of taper to apply to input trace", NUM_VALUES_FIXED );
  pdef->addValue( "none", VALTYPE_OPTION );
  pdef->addOption( "none", "Do not apply any taper to input trace" );
  pdef->addOption( "cos", "Apply cosine taper to input trace" );
  pdef->addOption( "hanning", "Apply 'Hanning' cosine taper to input trace. Taper length is 1/2 trace" );
  pdef->addOption( "blackman", "Apply 'Blackman' taper (alpha=0.16) to input trace" );

  pdef->addParam( "taper_len", "Taper length in number of samples", NUM_VALUES_FIXED );
  pdef->addValue( "10", VALTYPE_NUMBER, "Length in number of samples" );

  pdef->addParam( "output", "Output options for forward transform", NUM_VALUES_FIXED );
  pdef->addValue( "amp_phase", VALTYPE_OPTION );
  pdef->addOption( "amp_phase", "Output amplitude and phase spectrum, concatenated into one trace" );
  pdef->addOption( "amp", "Output amplitude spectrum only" );
  pdef->addOption( "real_imag", "Output real and imaginary values concatenated into one trace" );
  pdef->addOption( "psd", "Output PSD spectrum" );
  pdef->addOption( "psd_even", "Output PSD spectrum. Omit value at Nyquist frequency, i.e. output 2^N samples." );

  pdef->addParam( "override", "Override domain", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "yes", "Override domain found in super header" );
  pdef->addOption( "no", "Acknowledge domain found in super header" );

}

extern "C" void _params_mod_fft_( csParamDef* pdef ) {
  params_mod_fft_( pdef );
}
extern "C" void _init_mod_fft_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log ) {
  init_mod_fft_( param, env, log );
}
extern "C" bool _exec_mod_fft_( csTrace* trace, int* port, csExecPhaseEnv* env, csLogWriter* log ) {
  return exec_mod_fft_( trace, port, env, log );
}


