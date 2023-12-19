/* Copyright (c) Colorado School of Mines, 2013.*/
/* All rights reserved.                       */

#include "cseis_includes.h"
#include "csGeolibUtils.h"
#include <cmath>
#include <cstring>
#include <limits>

extern "C" {
 #include <fftw3.h>
}

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

    // Debug, etc. 
    bool debug;
    int  callCounter;
    bool firstCall;       // First call to exec phase?

    // User information
    int    direction;     // Direction of transform: 'forward' or 'reverse'
    string direction_str;

    bool normalize;      // Normalize data: Yes or no?
    double normScalar;   // Normalisation scalar to be applied before inverse FFT.
                         // This scalar accounts for zeros that may have been padded to input trace
    bool outputEven;     // Output an even number of samples (See parameter description below.)

    int nSamplesToOutput;   // Number of samples output as equivalent # of floats (ex: 1 complex = 2 floats)
    int fftDataType;        // Datatype of the transformed data after FORWARD. 

    int taperType;            // Type of taper tp apply to input 
    int taperLengthInSamples; // Taper length in number of samples (from 0 to 1)

    // Data details
    int nSamplesInput;      // Number of samples input (4-byte float)
    float sampleRateInput;  // Sample rate of input data (ms or hz).

    float timeSampleRate;   // Sample rate in time domain

    int nSamplesTime;       // Number of samples in time domain *before* FOWARD
    int nSamplesToFFT;      // Number of samples after padding *before* FOWARD
    int nFreqToFFT;         // Number of samples in frequency domain (complex == 2*float)

    float freqSampleRate;   // Sample rate in frequency domain
    float nyquist;          // Nyquist frequency

    // FFTW stuff
    unsigned fftw_flag;           // Flag for FFTW plan routine
    float* realBuffer;            // Float buffer 
    fftwf_complex* complexBuffer; // Complex buffer
    fftwf_plan  myPlan;           // FFTW plan (presumably a structure)

  };
  static const string MY_NAME = "fft";

  static const int TAPER_NONE    = -1;
  static const int TAPER_COSINE  = 1;
  static const int TAPER_HANNING = 2;
  static const int TAPER_BLACKMAN = 3;

  static const int FORWARD = FFTW_FORWARD;
  static const int INVERSE = FFTW_BACKWARD; 

}
using mod_fft::VariableStruct;

int factor_2357( int *in );

//*************************************************************************************************
// Init phase
//*************************************************************************************************
void init_mod_fft_( csParamManager* param, csInitPhaseEnv* env, csLogWriter* log )
{
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader*  shdr = env->superHeader;
  VariableStruct* vars = new VariableStruct();
  edef->setVariables( vars );

  edef->setExecType( EXEC_TYPE_SINGLETRACE );

  std::string text;

  vars->firstCall      = true;
  vars->callCounter    = 0;
  vars->realBuffer     = NULL;
  vars->complexBuffer  = NULL;
  vars->myPlan         = NULL;
  vars->direction      = mod_fft::FORWARD;
  vars->normScalar     = 1.0;
  vars->normalize      = false;
  vars->outputEven     = false;
  vars->fftDataType    = FX_NONE;
  vars->taperType      = mod_fft::TAPER_NONE;
  vars->taperLengthInSamples = 10;

  vars->nSamplesInput   = shdr->numSamples;
  vars->sampleRateInput = shdr->sampleInt;

  // Dump parameter not used? 
  if( param->exists("dump") ) {log->warning("Parameter 'dump' currently not supported.");}

  // Direction
  if( param->exists("direction") ) {
    param->getString( "direction", &text );
    text = toLowerCase( text );

    if( text.compare("forward") == 0 ) {
      vars->direction = mod_fft::FORWARD;
      vars->direction_str = "FORWARD";

    } else if( text.compare("inverse") == 0 ) {
      vars->direction = mod_fft::INVERSE;
      vars->direction_str = "INVERSE";

    } else {
      log->line("ERROR:Unknown 'direction' option: %s", text.c_str());
      env->addError();
    }

  }

  // Ignore domain stored in the superheader if it conflicts with the user parameters.
  bool override_domain = false;
  if( param->exists("override") ) {
    param->getString("override", &text);
    text = toLowerCase( text );
    if( text.compare("yes") == 0 ) {
      override_domain = true;
    } else if( text.compare("no")  == 0 ) {
      override_domain = false;
    } else {
      log->line("ERROR:Unknown 'override_domain' option: %s", text.c_str());
      env->addError();
    }
  }

  // Normalize the output.
  if( param->exists("norm") ) {
    param->getString("norm", &text);
    text = toLowerCase( text );
    if( text.compare("yes") == 0 ) {
      vars->normalize = true;
    } else if( text.compare("no") == 0 ) {
      vars->normalize = false;
    }
    else {
      log->line("ERROR:Unknown 'norm' option: %s", text.c_str());
      env->addError();
    }
  }

  // Taper the input
  if( param->exists("taper_type") ) {

    if( vars->direction == mod_fft::INVERSE ) {
      log->warning("User parameter 'taper_type' will be ignored for INVERSE FFT transform");
    } else {
      param->getString("taper_type", &text);
      text = toLowerCase( text );

      if( text.compare("none") == 0 ) {
        vars->taperType = mod_fft::TAPER_NONE;

      }else if( text.compare("cos") == 0 ) {
        vars->taperType = mod_fft::TAPER_COSINE;

      } else if( text.compare("hanning") == 0 ) {
        vars->taperType = mod_fft::TAPER_HANNING;
        vars->taperLengthInSamples = shdr->numSamples/2;

      } else if( text.compare("blackman") == 0 ) {
        vars->taperType = mod_fft::TAPER_BLACKMAN;
        vars->taperLengthInSamples = shdr->numSamples/2;

      } else {
        log->line("ERROR:Unknown 'taper_type' option: %s", text.c_str());
        env->addError();
      }
    }
  }
  if( param->exists("taper_len") ) {

    if( vars->direction == mod_fft::INVERSE ) {
      log->warning("User parameter 'taper_len' will be ignored for INVERSE FFT transform");
    } else {
      if( vars->taperType == mod_fft::TAPER_HANNING ) {
        log->line("ERROR:Cannot specify 'taper_len' for Hanning taper, taper length is fixed to half the trace length");
        env->addError();
      } else {
        param->getInt("taper_len", &vars->taperLengthInSamples);
      }
    }
  }

  // Output type of the FORWARD transform data
  if( param->exists("output") ) {
    if( vars->direction == mod_fft::INVERSE ) {
      log->warning("User parameter 'output' will be ignored for INVERSE FFT transform");

    } else {
      param->getString("output", &text);
      if( text.compare("amp_phase") == 0 ) {
        vars->fftDataType = FX_AMP_PHASE;

      } else if( text.compare("amp") == 0 ) {
        vars->fftDataType = FX_AMP;

      } else if( text.compare("psd") == 0 ) {
        vars->fftDataType = FX_PSD;

      } else if( text.compare("psd_even") == 0 ) {
        vars->fftDataType = FX_PSD;
        vars->outputEven = true;

      } else if( text.compare("real_imag") == 0 ) {
        vars->fftDataType = FX_REAL_IMAG;

      } else {
        log->line("ERROR:Unknown 'output' option: %s", text.c_str());
        env->addError();
      }
    }

  } else if( vars->direction == mod_fft::FORWARD ) {
    log->line("ERROR:User parameter 'output' is required for forward FFT transform");
    env->addError();
  }

  // FORWARD FFT
  if( vars->direction == mod_fft::FORWARD ) {
    log->line("FFT FORWARD transform." );

    if( shdr->domain != DOMAIN_XT && shdr->domain != DOMAIN_XD && !override_domain ) {
      log->line("ERROR:Current trace is not in XT (or XD) domain. FFT forward transform not possible. Actual domain: %s (%d)", csGeolibUtils::domain2Text(shdr->domain) );
      env->addError();              
    }

    // Reset input trace length?
    vars->nSamplesTime = vars->nSamplesInput;
    int new_nsamples = vars->nSamplesTime;
    if( param->exists("nsamples") ) {
      param->getInt("nsamples", &new_nsamples);
      if ( new_nsamples <= 0 ){
        log->line("ERROR: invalid value for `nsamples` %d.", new_nsamples );
        env->addError();              
      } else if ( new_nsamples < vars->nSamplesTime ){
        log->line( "ERROR: Value for 'nsamples', %d, is less than input trace length, %d.", new_nsamples, vars->nSamplesTime );
        env->addError();              
      } else {
        log->line( "Resetting input trace length from %d to %d samples.", vars->nSamplesTime, new_nsamples );
      }
    }    

    bool use_unopt = false;
    if( param->exists("use_unopt") ) {
      param->getString("use_unopt", &text);
      text = toLowerCase( text );
      if( text.compare("yes") == 0 ) {
        use_unopt = true;
      } else if( text.compare("no")  == 0 ) {
        use_unopt = false;
      } else {
        log->line("ERROR:Unknown 'use_unopt' option: %s", text.c_str());
        env->addError();
      }
    }

    // Optimize trace length for FFTW
    int out;
    out = factor_2357( &new_nsamples ); 
    if ( out > 0  ){
      log->line("Optimizing number of real samples to transform to %d (input is %d).", out, new_nsamples );
    } else {
      if ( use_unopt && new_nsamples > 0 ){
        log->warning("Failed optimizing FFT length %d but 'use_unopt' specified.", new_nsamples );
        log->warning("Proceeding with %d for length. Performance will be slow!", new_nsamples );
        out = new_nsamples;
      } else {
        log->line("ERROR:Failed optimizing FFT length %d.", new_nsamples );
        env->addError();              
      }
    }    
    vars->nSamplesToFFT = out;

    if ( vars->nSamplesToFFT > 999999 ){log->warning("FFT length %d is very large.", new_nsamples );}

    vars->nFreqToFFT = vars->nSamplesToFFT/2+1;
    log->line("Number of frequencies is %d.",vars->nFreqToFFT );
 
    if( vars->fftDataType == FX_AMP_PHASE ) {
      vars->nSamplesToOutput = 2*vars->nFreqToFFT; 

    } else if( vars->fftDataType == FX_AMP || vars->fftDataType == FX_PSD ) {
      vars->nSamplesToOutput = vars->nFreqToFFT; 
      if( vars->outputEven ) vars->nSamplesToOutput = vars->nSamplesToFFT/2;

    } else if( vars->fftDataType == FX_REAL_IMAG ) {
      vars->nSamplesToOutput = 2*vars->nFreqToFFT; 
    }

    log->line("Number of output samples is %d.",vars->nSamplesToOutput );

    // Always MEASURE. 
    vars->fftw_flag = FFTW_MEASURE;

    // Evaluate frequency info
    vars->timeSampleRate = vars->sampleRateInput;
    vars->nyquist = 1./(2.*(vars->timeSampleRate/1000.0));
    vars->freqSampleRate = vars->nyquist/(vars->nFreqToFFT-1);
    log->line("FFT:Input sample rate = %f -> Nyquist is %f @ delta f = %f.",
              vars->timeSampleRate, vars->nyquist, vars->freqSampleRate );

    // Save the time-domain information now.
    shdr->numSamplesXT = vars->nSamplesInput;
    shdr->sampleIntXT  = vars->sampleRateInput;

    // Reset samples & sample rate for output
    shdr->numSamples = vars->nSamplesToOutput;
    shdr->sampleInt  = vars->freqSampleRate;

    shdr->domain      = DOMAIN_FX;
    shdr->fftDataType = vars->fftDataType;

    // INVERSE FFT
  } else if( vars->direction == mod_fft::INVERSE ) {
    log->line("FFT INVERSE transform." );

    if( param->exists("nsamples") ) { log->warning("User parameter 'nsamples' will be ignored for INVERSE FFT transform"); }

    vars->nFreqToFFT = vars->nSamplesToFFT = 0;
    if( shdr->domain != DOMAIN_FX ) {
      log->line("ERROR:Current trace is not in FX domain. FFT inverse transform not possible. Actual domain: %s", csGeolibUtils::domain2Text(shdr->domain) );
      env->addError();

    } else if( shdr->fftDataType == FX_AMP_PHASE || shdr->fftDataType == FX_REAL_IMAG ) {
      vars->nFreqToFFT = vars->nSamplesInput/2;
      vars->nSamplesToFFT = (vars->nFreqToFFT-1)*2;

    } else { // if( shdr->fftDataType != FX_REAL_IMAG && shdr->fftDataType != FX_AMP_PHASE ) {
      string text = "UNKNOWN";
      if( shdr->fftDataType == FX_AMP ) text = "AMP";
      else if( shdr->fftDataType == FX_PSD ) text = "PSD";
      log->line("ERROR:Cannot perform inverse transform due to lack of information: Data is neither of type AMP_PHASE or REAL_IMAG. Actual type defined in super header: %s", text.c_str());
      env->addError();              
    }
    vars->fftDataType = shdr->fftDataType;

    vars->nSamplesTime  = shdr->numSamplesXT; 
    if( vars->normalize ) vars->normScalar = (double)shdr->numSamplesXT/(double)shdr->numSamples;

    // Always MEASURE. 
    vars->fftw_flag = FFTW_MEASURE; 

    // Number of samples on output & reset sample rate for time.
    vars->nSamplesToOutput = shdr->numSamplesXT;
    shdr->numSamples   = vars->nSamplesToOutput;
    shdr->sampleInt    = shdr->sampleIntXT;
    shdr->numSamplesXT = 0;
    shdr->sampleIntXT  = 0;

    shdr->domain      = DOMAIN_XT;  // CHANGE: May need to set to XD domain...
    shdr->fftDataType = FX_NONE; 

  }
  shdr->dump( log->getFile() );

}

//*************************************************************************************************
// Exec phase
//*************************************************************************************************
bool exec_mod_fft_(
  csTrace* trace,
  int* port,
  csExecPhaseEnv* env, csLogWriter* log )
{
  VariableStruct* vars = reinterpret_cast<VariableStruct*>( env->execPhaseDef->variables() );
  csExecPhaseDef* edef = env->execPhaseDef;
  csSuperHeader const* shdr = env->superHeader;

  vars->callCounter++;

  if( edef->isCleanup()){
    if( vars->realBuffer != NULL ) 
      { fftwf_free(vars->realBuffer); vars->realBuffer = NULL; }
    if( vars->complexBuffer != NULL ) 
      { fftwf_free(vars->complexBuffer); vars->complexBuffer = NULL; }   
    if( vars->myPlan != NULL ) 
      { fftwf_destroy_plan (vars->myPlan); vars->myPlan = NULL; }
    delete vars; vars = NULL;
    return true; 
  }

  float *fftR;
  fftwf_complex *fftC; 
  fftwf_plan fftPlan;

  float* samples = trace->getTraceSamples();

  // Forward transform
  if( vars->direction == mod_fft::FORWARD ) {

    // Apply taper to input trace
    if( vars->taperType == mod_fft::TAPER_COSINE || vars->taperType == mod_fft::TAPER_HANNING ) {
      int nSteps = vars->taperLengthInSamples;
      for( int i = 0; i < nSteps; i++ ) {
        float scalar = cos( M_PI_2 * (float)(nSteps-i)/(float)nSteps );
        samples[i] *= scalar;
      }
      for( int i = vars->nSamplesInput-nSteps; i < vars->nSamplesInput; i++ ) {
        float scalar = cos( M_PI_2 * (float)(nSteps-vars->nSamplesInput+i+1)/(float)nSteps );
        samples[i] *= scalar;
      }
    } else if( vars->taperType == mod_fft::TAPER_BLACKMAN ) {
      float alpha = 0.16;
      float a0 = 0.5 * (1.0 - alpha);
      float a1 = 0.5;
      float a2 = 0.5 * alpha;
      for( int i = 0; i < vars->nSamplesInput; i++ ) {
        float piFactor = (2.0 * M_PI) * (float)i / (float)(vars->nSamplesInput - 1);
        float weight = a0 - a1*cos( piFactor ) + a2*cos( 2 * piFactor );
        samples[i] *= weight;
      }
    }

    int nt    = vars->nSamplesTime;
    int ntp   = vars->nSamplesToFFT;
    int nfreq = vars->nFreqToFFT;

    // Evaluate the FFTW plan & allocate buffers on the first call.
    if ( vars->myPlan == NULL ){

      // Buffer allocations 
     fftR = (float*)fftwf_malloc(sizeof(float) * ntp);
      fftC = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * nfreq);
      fftPlan = fftwf_plan_dft_r2c_1d( ntp, fftR, fftC, vars->fftw_flag );

      vars->realBuffer    = fftR;
      vars->complexBuffer = fftC;
      vars->myPlan        = fftPlan;

    }
    fftR = vars->realBuffer;
    fftC = vars->complexBuffer;
    fftPlan = vars->myPlan;
    
    // copy input to real buffer
    memset( fftR, 0, ntp*sizeof(float) );
    memcpy( fftR, samples, nt*sizeof(float) );

    // FFT
    memset( fftC, 0, sizeof(fftwf_complex)*nfreq );
    fftwf_execute(fftPlan);

    double normSpectralDensity = 1.0;
    if( vars->fftDataType == FX_PSD ) {
      int numValues = vars->nSamplesToFFT/2;
      if( !vars->outputEven ) { 
        numValues += 1;
      }
      normSpectralDensity = 1.0;
      if( vars->normalize ) {
        int numSamplesNonZero = 0;
        for( int i = 0; i < nt; i++ ) {
          if( samples[i] != 0.0 ) numSamplesNonZero += 1;
        }
        if( numSamplesNonZero == 0 ) numSamplesNonZero = 1;
        normSpectralDensity = 1.0 / (numSamplesNonZero * (1000.0/vars->sampleRateInput));
      }

      // Multiply complex specturm with conjugate complex = (R + I) * (R - I)
      for( int is = 0; is < numValues; is++ ) {
        //        samples[is] = 2.0 * ( real[is]*real[is] + imag[is]*imag[is] ) * normSpectralDensity;
        samples[is] = 2.0 * ( fftC[is][0]*fftC[is][0] + 
                              fftC[is][1]*fftC[is][1] ) * normSpectralDensity;
      }
 
    } else if( vars->fftDataType == FX_REAL_IMAG ) {
      if( vars->normalize ) {
        normSpectralDensity = 1.0 / sqrt( (double)vars->nSamplesInput );
      }
      for( int is = 0; is < nfreq; is++ ) {
        samples[is]       = (float)(fftC[is][0]*normSpectralDensity);
        samples[is+nfreq] = (float)(fftC[is][1]*normSpectralDensity);
      }

    } else if( vars->fftDataType == FX_AMP_PHASE ) {

      float amplitude, phase;
      for ( int isamp=0; isamp<nfreq; isamp++) {
        // amplitude = sqrt (fftC[isamp][0]*fftC[isamp][0] + 
        //                   fftC[isamp][1]*fftC[isamp][1]);
        // phase     = atan2(fftC[isamp][1], fftC[isamp][0]);
        // NOTE: The following amp/phase calculations are modified (compared to the  previous)
        // for consistency with the csFFTTools class.
        amplitude = 2.0*sqrt (fftC[isamp][0]*fftC[isamp][0] + 
                              fftC[isamp][1]*fftC[isamp][1]);
        phase     = atan2( -fftC[isamp][1], fftC[isamp][0] );
        samples[isamp] = amplitude;
        samples[isamp + nfreq] = phase;
      }
      int numSamplesNormalise = vars->nSamplesToFFT/2;
      //      int numSamplesNormalise = nfreq;
      if( vars->normalize ) {
        normSpectralDensity = 1.0 / sqrt( (double)vars->nSamplesInput );
        for( int is = 0; is < numSamplesNormalise; is++ ) {
          samples[is] *= normSpectralDensity;
        }
      }

    } else if( vars->fftDataType == FX_AMP ) {
      
      int numSamplesNormalise = shdr->numSamples;
      float amplitude;
      //      for ( int isamp=0; isamp<nfreq; isamp++) {
      //      printf( "BAP: %d\n", numSamplesNormalise );
      for ( int isamp=0; isamp<numSamplesNormalise; isamp++) {
        // amplitude = sqrt (fftC[isamp][0]*fftC[isamp][0] + 
        //                   fftC[isamp][1]*fftC[isamp][1]);
        // NOTE: The following amp/phase calculations are modified (compared to the  previous)
        // for consistency with the csFFTTools class.
        amplitude = 2.0*sqrt (fftC[isamp][0]*fftC[isamp][0] + 
                              fftC[isamp][1]*fftC[isamp][1]);
        samples[isamp] = amplitude;
      }
      if( vars->normalize ) {
        normSpectralDensity = 1.0 / sqrt( (double)vars->nSamplesInput );
        for( int is = 0; is < numSamplesNormalise; is++ ) {
          samples[is] *= normSpectralDensity;
        }
      }

    }

  }

  // Inverse transform.
  if( vars->direction == mod_fft::INVERSE ) {
    int nt    = vars->nSamplesTime;
    int ntp   = vars->nSamplesToFFT;
    int nfreq = vars->nFreqToFFT;

    float scale = 1./(ntp);

    // Evaluate the FFTW plan & allocate buffers on the first call.
    if ( vars->myPlan == NULL ){

      // Buffer allocations
      fftR = (float*)fftwf_malloc(sizeof(float) * ntp);
      fftC = (fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex) * nfreq);
      fftPlan = fftwf_plan_dft_c2r_1d( ntp, fftC, fftR, vars->fftw_flag );

      vars->realBuffer    = fftR;
      vars->complexBuffer = fftC;
      vars->myPlan        = fftPlan;

    }
    fftR = vars->realBuffer;
    fftC = vars->complexBuffer;
    fftPlan = vars->myPlan;

    // // copy input to complex buffer
    // float* out = (float*)(fftC);
    // memcpy( out, samples, nfreq*2*sizeof(float) );

    if( vars->fftDataType == FX_AMP_PHASE ) {
      
      float amplitude, phase;
      for (int isamp=0; isamp<nfreq; isamp++) {
        amplitude = samples[isamp];
        phase     = samples[isamp + nfreq];
        // fftC[isamp][0] = amplitude * cos(phase);
        // fftC[isamp][1] = amplitude * sin(phase);
        // NOTE: The following amp/phase calculations are modified (compared to the  previous)
        // for consistency with the csFFTTools class.
        fftC[isamp][0] = 0.5*amplitude * cos(phase);
        fftC[isamp][1] = -0.5*amplitude * sin(phase);
      }        
    } else if ( vars->fftDataType == FX_REAL_IMAG ) {

      for (int isamp=0; isamp<nfreq; isamp++) {
        fftC[isamp][0] = samples[isamp];
        fftC[isamp][1] = samples[isamp+nfreq];
      }        
      
    }

    // FFT
    memset( fftR, 0, ntp*sizeof(float) );
    fftwf_execute(fftPlan);
    
    for( int isamp=0; isamp<nt; isamp++ ) {    
      samples[isamp] = fftR[isamp]*scale;
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
  pdef->setModule( "FFT", "FFT transform", "Transform input data from x-t to x-w domain (forward) or vice versa (inverse)");
  pdef->setVersion(1,0);
  pdef->addDoc("NOTE: This version optimizes the input trace length to be the next greater value which is a multiple of small primes, 2, 3, 5, and 7. Previous versions used a power of 2. Thus the number of samples processed is typically less than previous versions of the module.");
 
  pdef->addParam( "direction", "Direction of transform.", NUM_VALUES_FIXED );
  pdef->addValue( "forward", VALTYPE_OPTION );
  pdef->addOption( "forward", "Forward transform from x-t to x-w" );
  pdef->addOption( "inverse", "Inverse transform from x-w to x-t",
    "Inverse transform will only work if a forward transform was applied before" );

  pdef->addParam( "norm", "Normalize output.", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "yes", "Normalize output values", "Example: Using the same input data but with different amount of added zeros, the amplitude spectrum will look exactly the same for the same output frequency" );
  pdef->addOption( "no", "Do not normalize output values" );

  pdef->addParam( "taper_type", "Taper type of taper to apply to input trace.", NUM_VALUES_FIXED );
  pdef->addValue( "none", VALTYPE_OPTION );
  pdef->addOption( "none", "Do not apply any taper to input trace" );
  pdef->addOption( "cos", "Apply cosine taper to input trace" );
  pdef->addOption( "hanning", "Apply 'Hanning' cosine taper to input trace. Taper length is 1/2 trace" );
  pdef->addOption( "blackman", "Apply 'Blackman' taper (alpha=0.16) to input trace" );

  pdef->addParam( "taper_len", "Taper length in number of samples.", NUM_VALUES_FIXED );
  pdef->addValue( "10", VALTYPE_NUMBER, "Length in number of samples" );

  pdef->addParam( "output", "Output options for forward transform.", NUM_VALUES_FIXED );
  pdef->addValue( "amp_phase", VALTYPE_OPTION );
  pdef->addOption( "amp_phase", "Output amplitude and phase spectrum concatenated into one trace, amplitudes at the beginning of the trace, phase at the end" );
  pdef->addOption( "amp", "Output amplitude spectrum only" );
  pdef->addOption( "real_imag", "Output real and imaginary values concatenated into one trace, real values at the beginning of the trace, imaginary values at the end" );
  pdef->addOption( "psd", "Output PSD spectrum" );
  pdef->addOption( "psd_even", "Output PSD spectrum. Omit value at Nyquist frequency, i.e. output 2^N samples" );

  pdef->addParam( "override", "Override domain.", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "yes", "Override domain found in super header" );
  pdef->addOption( "no", "Acknowledge domain found in super header" );

  pdef->addParam( "nsamples", "For FORWARD transform, reset the number of samples for the input trace.", NUM_VALUES_FIXED,
                  "This in turn will be reset to an optimum value for the FFT. The trace will be padded with additional zero samples." );
  pdef->addValue( "", VALTYPE_NUMBER, "Number of samples to use in the FFT. If less than the original trace length, the original length will be used" );

  pdef->addParam( "use_unopt", "If the trace length (either input or specified by 'nsamples' parameter) can't be optimized, use it anyway. Note this usually only happens if it is a very large number so it will be very inefficent for the FFT.", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "yes", "Use the trace length regardless of whether it is optimized or not." );
  pdef->addOption( "no", "Only use the optimized length for the FFT. Fail if it can't be optimized" );

  pdef->addParam( "dump", "Dump FFT values to log file. NOTE: this has been disabled since previous versions.", NUM_VALUES_FIXED );
  pdef->addValue( "no", VALTYPE_OPTION );
  pdef->addOption( "yes", "Dump FFT values to log file" );
  pdef->addOption( "no", "Do not dump FFT values" );

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

////////////////////////////////////////////////////////////////////////////////
// Return the first multiple of small primes (2,3,5 and 7) greater than or 
// equal to the input value. 
//
// The return value is -1 if the value can't be reduced to a factor of small
// primes. That is, if it is less than 0 or too large (*).
//
// (*)NOTE: The largest 32-bit integer that is still a factor of small primes 
// appears to be 
//
//          INT_MAX - 3330622  = 2147483647 - 3330622
//                             = 2144153025
//
// Numbers larger than this are still valid integers up to INT_MAX but can't 
// be factored.
// 
// It is up to the calling program to decide what to do in that case. The return
// value will still be -1.
//
////////////////////////////////////////////////////////////////////////////////
int factor_2357( int *my_in ){
  int  test,  n2, n3, n5, n7, count; 

  int in = *my_in;
  int out; 

  out = -1;

  if ( in < 0 || in > (std::numeric_limits<int>::max()-1) ) return out; // Fail if input bad  
  if ( in <=1 ){ out = 1; return out; } // Skip the loop for the easy one. 

  // Factor-out small primes from the input number. It should reduce to 1 once 
  // it is a perfect factor of 2,3,5 and 7. If not, increment the input and repeat.
  // Fail if the number of iterations or the test value gets to large. 
  test  = 0;
  count = 0;
  while ( test != 1 && count < std::numeric_limits<int>::max() ){
     test = in+count;
     if ( test == std::numeric_limits<int>::max() ) break;
     count++;     

     n2=n3=n5=n7=0;
     while ( test%7 == 0 ){ test = test/7; n7++; }
     while ( test%5 == 0 ){ test = test/5; n5++; }
     while ( test%3 == 0 ){ test = test/3; n3++; }
     while ( test%2 == 0 ){ test = test/2; n2++; }

  }
  if ( test != 1 ) return out;
  out = (int)round( pow(2.0,n2) * pow(3.0,n3) * pow(5.0,n5) * pow(7.0,n7) );

  return out;
}


