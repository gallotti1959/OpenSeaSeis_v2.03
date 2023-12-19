/* Copyright (c) Colorado School of Mines, 2011.*/
/* All rights reserved.                       */

/* SUKDMIG2D: $Revision: 1.26 $ ; $Date: 2011/11/16 22:14:43 $	*/

#include "csException.h"
#include "csSUTraceManager.h"
#include "csSUArguments.h"
#include "csSUGetPars.h"
#include "su_complex_declarations.h"
#include "cseis_sulib.h"
#include <string>

extern "C" {
  #include <pthread.h>
}
#include "su.h"
#include "segy.h"

/*********************** self documentation **********************/
std::string sdoc_sukdmig2d =
" 									"
"SUKDMIG2D - Kirchhoff Depth Migration of 2D poststack/prestack data	"
" 									"
"    sukdmig2d  infile=  outfile=  ttfile=   [parameters] 		"
"									"
" Required parameters:							"
" infile=stdin		file for input seismic traces			"
" outfile=stdout	file for common offset migration output  	"
" ttfile=		file for input traveltime tables		"
"									"
" ...  The following 9 parameters describe traveltime tables:		"
" fzt= 			first depth sample in traveltime table		"
" nzt= 			number of depth samples in traveltime table	"
" dzt=			depth interval in traveltime table		"
" fxt=			first lateral sample in traveltime table	"
" nxt=			number of lateral samples in traveltime table	"
" dxt=			lateral interval in traveltime table		"
" fs= 			x-coordinate of first source			"
" ns= 			number of sources				"
" ds= 			x-coordinate increment of sources		"
"									"
" Optional Parameters:							"
" dt= or from header (dt) 	time sampling interval of input data	"
" ft= or from header (ft) 	first time sample of input data		"
" dxm= or from header (d2) 	sampling interval of midpoints 		"
" fzo=fzt		    z-coordinate of first point in output trace	"
" dzo=0.2*dzt		vertical spacing of output trace 		"
" nzo=5*(nzt-1)+1 	number of points in output trace		"	
" fxo=fxt		    x-coordinate of first output trace 		"
" dxo=0.5*dxt		horizontal spacing of output trace 		"
" nxo=2*(nxt-1)+1  	number of output traces 			"	
" off0=0		   	first offest in output 			"
" doff=99999		offset increment in output 			"
" noff=1	 	number of offsets in output 			"	
" absoff=0      flag for using absolute offsets of input traces		"
"               =0 means use offset=gx-sx                		"
"               =1 means use abs(gx-sx)                  		"
" limoff=0      flag for only using input traces that fall within the range"
"               of defined output offset bins (off0,doff,noff) 		"
"               =0 means use all input traces                 		"
"               =1 means limit traces used by offset           		"
" fmax=0.25/dt		frequency-highcut for input traces		"
" offmax=99999		maximum absolute offset allowed in migration 	"
" aperx=nxt*dxt/2  	migration lateral aperature 			"
" angmax=60		migration angle aperature from vertical 	"
" v0=1500(m/s)		reference velocity value at surface		"	
" dvz=0.0  		reference velocity vertical gradient		"
"									"
" ls=1			flag for line source				"
" jpfile=stderr		job print file name 				"
"									"
" mtr=100  		print verbal information at every mtr traces	"
" ntr=100000		maximum number of input traces to be migrated	"
" npv=0			flag of computing quantities for velocity analysis"
" rscale=1000.0 	scaling for roundoff error suppression		"
"									"
"   ...if npv>0 specify the following three files:			"
" tvfile=tvfile		input file of traveltime variation tables	"
"			tv[ns][nxt][nzt]				"
" csfile=csfile		input file of cosine tables cs[ns][nxt][nzt]	"
" outfile1=outfile1	file containning additional migration output   	"
"			with extra amplitude				"
"									"
" Notes:								"
" 1. Traveltime tables were generated by program rayt2d (or other ones)	"
"    on relatively coarse grids, with dimension ns*nxt*nzt. In the	"
"    migration process, traveltimes are interpolated into shot/gephone 	"
"    positions and output grids.					"
" 2. Input seismic traces must be SU format and can be any type of 	"
"    gathers (common shot, common offset, common CDP, and so on).	" 
" 3. Migrated traces are output in CDP gathers if velocity analysis	"
"    is required, with dimension nxo*noff*nzo.  			" 
" 4. If the offset value of an input trace is not in the offset array 	"
"    of output, the nearest one in the array is chosen. 		"
" 5. Memory requirement for this program is about			"
"    	[ns*nxt*nzt+noff*nxo*nzo+4*nr*nzt+5*nxt*nzt+npa*(2*ns*nxt*nzt   "
"	+noff*nxo*nzo+4*nxt*nzt)]*4 bytes				"
"    where nr = 1+min(nxt*dxt,0.5*offmax+aperx)/dxo. 			"
" 6. Amplitudes are computed using the reference velocity profile, v(z),"
"    specified by the parameters v0= and dvz=.				"
" 7. Input traces must specify source and receiver positions via the header"
"    fields tr.sx and tr.gx. Offset is computed automatically.		"
" 8. if limoff=0, input traces from outside the range defined by off0, doff, "
"    noff, will get migrated into the extremal offset bins/planes.  E.g. if "
"    absoff=0 and limoff=0, all traces with gx<sx will get migrated into the "
"    off0 bin."
"									"
;

/*  Sun Feb 24 13:30:07 2013
  Automatically modified for usage in SeaSeis  */
namespace sukdmig2d {

/*
 * Author:  Zhenyue Liu, 03/01/95,  Colorado School of Mines 
 * Modifcations:
 *    Gary Billings, Talisman Energy, Sept 2005:  added absoff, limoff.
 *
 * Trace header fields accessed: ns, dt, delrt, d2
 * Trace header fields modified: sx, gx
 */
 
/**************** end self doc ***********************************/
  void resit(int nx,float fx,float dx,int nz,int nr,float dr,
	float **tb,float **t,float x0);
  void interpx(int nxt,float fxt,float dxt,int nx,float fx,float dx,
	int nzt,float **tt,float **t);
  void sum2(int nx,int nz,float a1,float a2,float **t1,float **t2,float **t);
  void timeb(int nr,int nz,float dr,float dz,float fz,float a,
	float v0,float **t,float **p,float **sig,float **ang);
  void mig2d(float *trace,int nt,float ft,float dt,
	float sx,float gx,float **mig,float aperx,
  	int nx,float fx,float dx,float nz,float fz,float dz,
	int ls,int mtmax,float dxm,float fmax,float angmax,
	float **tb,float **pb,float **cs0b,float **angb,int nr,
	float **tsum,int nzt,float fzt,float dzt,int nxt,float fxt,float dxt,
	int npv,float **cssum,float **tvsum,float **mig1);

/* define */
#define RSCALE_KDMIG 1000.0

/* segy trace */
segy tr, tro;

void* main_sukdmig2d( void* args )
{
	int nt;		/* number of time samples in input data		*/
	int nzt;	/* number of z-values in traveltime table	*/
	int nxt;	/* number of x-values in traveltime table	*/
	int nzo;	/* number of z-values in output data		*/
	int nxo;	/* number of x-values in output data		*/
	int ns;		/* number of sources				*/
	int noff;	/* number of offsets in output data		*/
	int nr;
	int is,io,ixo,izo; /* counters */
	int ls,ntr,jtr,ktr,mtr,npv,mtmax;
	int   offset,absoff,limoff;
	off_t nseek;
	float   ft,fzt,fxt,fzo,fxo,fs,off0,dt,dzt,dxt,dzo,dxo,ds,doff,dxm,
		ext,ezt,ezo,exo,es,s,scal;	
	float v0,dvz,fmax,angmax,offmax,rmax,aperx,sx=0,gx=0;
	float ***mig=NULL,***ttab=NULL,**tb=NULL,**pb=NULL;
	float **cs0b=NULL,**angb=NULL,**tsum=NULL,**tt=NULL;
	float **tvsum=NULL,***mig1=NULL,***cs=NULL,***tv=NULL,**cssum=NULL;

	float rscale;			/* scaling factor for roundoff */
	
	char *infile="stdin",*outfile="stdout",*ttfile,*jpfile,*tvfile,
	     *csfile,*outfile1;
	FILE *infp,*outfp,*ttfp,*jpfp,*tvfp=NULL,*out1fp=NULL,*csfp=NULL;


	/* hook up getpar to handle the parameters */
	cseis_su::csSUArguments* suArgs = (cseis_su::csSUArguments*)args;
	cseis_su::csSUTraceManager* cs2su = suArgs->cs2su;
	cseis_su::csSUTraceManager* su2cs = suArgs->su2cs;
	int argc = suArgs->argc;
	char **argv = suArgs->argv;
	cseis_su::csSUGetPars parObj;

	void* retPtr = NULL;  /*  Dummy pointer for return statement  */
	su2cs->setSUDoc( sdoc_sukdmig2d );
	if( su2cs->isDocRequestOnly() ) return retPtr;
	parObj.initargs(argc, argv);

	try {  /* Try-catch block encompassing the main function body */


	/* open input and output files	*/
	if( !parObj.getparstring("infile",&infile)) {
		infp = stdin;
	} else  
		if ((infp=fopen(infile,"r"))==NULL)
			throw cseis_geolib::csException("cannot open infile=%s\n",infile);
	if( !parObj.getparstring("outfile",&outfile)) {
		outfp = stdout;
	} else  
		outfp = fopen(outfile,"w");
	efseeko(infp,(off_t) 0,SEEK_CUR);
	efseeko(outfp,(off_t) 0,SEEK_CUR);
	if( !parObj.getparstring("ttfile",&ttfile))
		throw cseis_geolib::csException("must specify ttfile!\n");
	if ((ttfp=fopen(ttfile,"r"))==NULL)
		throw cseis_geolib::csException("cannot open ttfile=%s\n",ttfile);
	if( !parObj.getparstring("jpfile",&jpfile)) {
		jpfp = stderr;
	} else  
		jpfp = fopen(jpfile,"w");

	/* get information from the first header */
	if (!fgettr(infp,&tr)) throw cseis_geolib::csException("can't get first trace");
	nt = tr.ns;
	if (!parObj.getparfloat("dt",&dt)) dt = ((double) tr.dt)/1000000.0; 
	if (dt<0.0000001) throw cseis_geolib::csException("dt must be positive!\n");
	if (!parObj.getparfloat("ft",&ft)) ft = tr.delrt/1000.0; 
 	if (!parObj.getparfloat("dxm",&dxm)) dxm = tr.d2;
	if  (dxm<0.0000001) throw cseis_geolib::csException("dxm must be positive!\n");
	
	/* get traveltime tabel parameters	*/
	if (!parObj.getparint("nxt",&nxt)) throw cseis_geolib::csException("must specify nxt!\n");
	if (!parObj.getparfloat("fxt",&fxt)) throw cseis_geolib::csException("must specify fxt!\n");
	if (!parObj.getparfloat("dxt",&dxt)) throw cseis_geolib::csException("must specify dxt!\n");
	if (!parObj.getparint("nzt",&nzt)) throw cseis_geolib::csException("must specify nzt!\n");
	if (!parObj.getparfloat("fzt",&fzt)) throw cseis_geolib::csException("must specify fzt!\n");
	if (!parObj.getparfloat("dzt",&dzt)) throw cseis_geolib::csException("must specify dzt!\n");
	if (!parObj.getparint("ns",&ns)) throw cseis_geolib::csException("must specify ns!\n");
	if (!parObj.getparfloat("fs",&fs)) throw cseis_geolib::csException("must specify fs!\n");
	if (!parObj.getparfloat("ds",&ds)) throw cseis_geolib::csException("must specify ds!\n");
	if (!parObj.getparfloat("rscale",&rscale))   rscale = RSCALE_KDMIG;

	ext = NINT(rscale*(fxt+(nxt-1)*dxt))/rscale;
	ezt = NINT(rscale*(fzt+(nzt-1)*dzt))/rscale;
	es = NINT(rscale*(fs+(ns-1)*ds))/rscale;

	/* optional parameters	*/
	if (!parObj.getparint("nxo",&nxo)) nxo = (nxt-1)*2+1;
	if (!parObj.getparfloat("fxo",&fxo)) fxo = fxt;
	if (!parObj.getparfloat("dxo",&dxo)) dxo = dxt*0.5;
	if (!parObj.getparint("nzo",&nzo)) nzo = (nzt-1)*5+1;
	if (!parObj.getparfloat("fzo",&fzo)) fzo = fzt;
	if (!parObj.getparfloat("dzo",&dzo)) dzo = dzt*0.2;
	exo = NINT(rscale*(fxo+(nxo-1)*dxo))/rscale;
	ezo = NINT(rscale*(fzo+(nzo-1)*dzo))/rscale;

	fprintf(jpfp," fxt=%f fxo=%f \n",fxt,fxo);
	fprintf(jpfp," ext=%f exo=%f \n",ext,exo);
	fprintf(jpfp," fzt=%f fzo=%f \n",fzt,fzo);
	fprintf(jpfp," ezt=%f ezo=%f \n",ezt,ezo);
	fprintf(jpfp," es=%f \n",es);
	fprintf(jpfp," \n");
	if(fxt>fxo || ext<exo || fzt>fzo || ezt<ezo) {
		warn("This condition must NOT be satisfied: fxt>fxo || ext<exo || fzt>fzo || ezt<ezo");
		warn("fxt=%f fxo=%f ext=%f exo=%f fzt=%f fzo=%f ezt=%f ezo=%f",
			  fxt,fxo,ext,exo,fzt,fzo,ezt,ezo);
		throw cseis_geolib::csException(" migration output range is out of traveltime table!\n");
	  }

	if (!parObj.getparfloat("v0",&v0)) v0 = 1500;
	if (!parObj.getparfloat("dvz",&dvz)) dvz = 0;
	if (!parObj.getparfloat("angmax",&angmax)) angmax = 60.;
	if  (angmax<0.00001) throw cseis_geolib::csException("angmax must be positive!\n");
	mtmax = 2*dxm*sin(angmax*PI/180.)/(v0*dt);
	if(mtmax<1) mtmax = 1;
	if (!parObj.getparfloat("aperx",&aperx)) aperx = 0.5*nxt*dxt;
	if (!parObj.getparfloat("offmax",&offmax)) offmax = 3000;
	if (!parObj.getparfloat("fmax",&fmax)) fmax = 0.25/dt;
	if (!parObj.getparint("noff",&noff))	noff = 1;
	if (!parObj.getparfloat("off0",&off0)) off0 = 0.;
	if (!parObj.getparfloat("doff",&doff)) doff = 99999;

	if (!parObj.getparint("ls",&ls))	ls = 1;
	if (!parObj.getparint("absoff",&absoff))   absoff=0;
	if (!parObj.getparint("limoff",&limoff))   limoff=0;
	if (!parObj.getparint("ntr",&ntr))	ntr = 100000;
	if (!parObj.getparint("mtr",&mtr))	mtr = 100;
	if (!parObj.getparint("npv",&npv))	npv = 0;
	if(npv){
		if( !parObj.getparstring("tvfile",&tvfile))
			throw cseis_geolib::csException("must specify tvfile!\n");
 		tvfp = fopen(tvfile,"r");
		if( !parObj.getparstring("csfile",&csfile))
			throw cseis_geolib::csException("must specify csfile!\n");
 		csfp = fopen(csfile,"r");
		if( !parObj.getparstring("outfile1",&outfile1))
			outfile1="outfile1";
 		out1fp = fopen(outfile1,"w");
	}

        parObj.checkpars();

	fprintf(jpfp,"\n");
	fprintf(jpfp," Migration parameters\n");
	fprintf(jpfp," ================\n");
	fprintf(jpfp," infile=%s \n",infile);
	fprintf(jpfp," outfile=%s \n",outfile);
	fprintf(jpfp," ttfile=%s \n",ttfile);
	fprintf(jpfp," \n");
	fprintf(jpfp," nzt=%d fzt=%g dzt=%g\n",nzt,fzt,dzt);
	fprintf(jpfp," nxt=%d fxt=%g dxt=%g\n",nxt,fxt,dxt);
 	fprintf(jpfp," ns=%d fs=%g ds=%g\n",ns,fs,ds);
	fprintf(jpfp," \n");
	fprintf(jpfp," nzo=%d fzo=%g dzo=%g\n",nzo,fzo,dzo);
	fprintf(jpfp," nxo=%d fxo=%g dxo=%g\n",nxo,fxo,dxo);
	fprintf(jpfp," \n");
	
	/* compute reference traveltime and slowness  */
	rmax = MAX(es-fxt,ext-fs);
	rmax = MIN(rmax,0.5*offmax+aperx);
	nr = 2+(int)(rmax/dxo);
	tb = ealloc2float(nzt,nr);
	pb = ealloc2float(nzt,nr);
	cs0b = ealloc2float(nzt,nr);
	angb = ealloc2float(nzt,nr);
	timeb(nr,nzt,dxo,dzt,fzt,dvz,v0,tb,pb,cs0b,angb);

	fprintf(jpfp," nt=%d ft=%g dt=%g \n",nt,ft,dt);
 	fprintf(jpfp," dxm=%g fmax=%g\n",dxm,fmax);
 	fprintf(jpfp," noff=%d off0=%g doff=%g\n",noff,off0,doff);
	fprintf(jpfp," v0=%g dvz=%g \n",v0,dvz);
 	fprintf(jpfp," aperx=%g offmax=%g angmax=%g\n",aperx,offmax,angmax);
 	fprintf(jpfp," ntr=%d mtr=%d ls=%d npv=%d\n",ntr,mtr,ls,npv);
	fprintf(jpfp," absoff=%d limoff=%d\n",absoff,limoff);
	if(npv)
 	  fprintf(jpfp," tvfile=%s csfile=%s outfile1=%s\n",
		tvfile,csfile,outfile1);
	fprintf(jpfp," ================\n");
	fflush(jpfp);

	/* allocate space */
	mig = ealloc3float(nzo,nxo,noff);
	ttab = ealloc3float(nzt,nxt,ns);
	tt = ealloc2float(nzt,nxt);
	tsum = ealloc2float(nzt,nxt);
	if(npv){
		tv = ealloc3float(nzt,nxt,ns);
		tvsum = ealloc2float(nzt,nxt);
		cs = ealloc3float(nzt,nxt,ns);
		cssum = ealloc2float(nzt,nxt);
	}
	if(!npv) 
		mig1 = ealloc3float(1,1,noff);
	else
		mig1 = ealloc3float(nzo,nxo,noff);


 	memset((void *) mig[0][0],0,noff*nxo*nzo*sizeof(float)); 
	 if(npv)
		memset((void *) mig1[0][0], 0,
			noff*nxo*nzo*sizeof(float));

 	fprintf(jpfp," input traveltime tables \n");
			     
	/* compute traveltime residual	*/
	for(is=0; is<ns; ++is){
		nseek = (off_t) nxt*nzt*is;
		efseeko(ttfp,nseek*((off_t) sizeof(float)),SEEK_SET);
		fread(ttab[is][0],sizeof(float),nxt*nzt,ttfp);
		s = fs+is*ds;
		resit(nxt,fxt,dxt,nzt,nr,dxo,tb,ttab[is],s);
		if(npv) {
			efseeko(tvfp, nseek*((off_t) sizeof(float)),SEEK_SET);
			fread(tv[is][0],sizeof(float),nxt*nzt,tvfp);
			efseeko(csfp, nseek*((off_t) sizeof(float)),SEEK_SET);
			fread(cs[is][0],sizeof(float),nxt*nzt,csfp);
		}
		
	}

	fprintf(jpfp," start migration ... \n");
	fprintf(jpfp," \n");
	fflush(jpfp);
	

	jtr = 1;
	ktr = 0;

	  fprintf(jpfp," fs=%g es=%g offmax=%g\n",fs,es,offmax);

	do {
		/* determine offset index	*/
	    float as,res;

		if (tr.scalco) { /* if tr.scalco is set, apply value */
			if (tr.scalco>0) {
				sx = tr.sx*tr.scalco;
				gx = tr.gx*tr.scalco;
		   	} else { /* if tr.scalco is negative divide */
				sx = tr.sx/ABS(tr.scalco);
				gx = tr.gx/ABS(tr.scalco);
			}
			
		} else {
			     sx = tr.sx;
			     gx = tr.gx;
		}

		/* GWB 2005.09.22: */
		/* io = (int)((gx-sx-off0)/doff+0.5); */
		offset=gx-sx;
		if( absoff && offset<0 )offset=-offset;
		io = (int)((offset-off0)/doff+0.5);
		if( limoff && (io<0 || io>=noff) ) continue;
		/* end of GWB 2005.09.22 */

	    if(io<0) io = 0;
	    if(io>=noff) io = noff-1;

		/* fprintf(jpfp," read trace jtr=%d: sx=%g gx=%g io=%d ktr=%d\n",jtr,sx,gx,io,ktr); */

	    if(MIN(sx,gx)>=fs && MAX(sx,gx)<=es && 
		 MAX(gx-sx,sx-gx)<=offmax ){
		/*     migrate this trace	*/

		    /* fprintf(jpfp," Good! Condition NOT satisfied\n"); */

	    	as = (sx-fs)/ds;
	    	is = (int)as;
		if(is==ns-1) is=ns-2;
		res = as-is;
		if(res<=0.01) res = 0.0;
		if(res>=0.99) res = 1.0;
		sum2(nxt,nzt,1-res,res,ttab[is],ttab[is+1],tsum);
		if(npv)  {
			sum2(nxt,nzt,1-res,res,tv[is],tv[is+1],tvsum);
			sum2(nxt,nzt,1-res,res,cs[is],cs[is+1],cssum);
		}
		
	    	as = (gx-fs)/ds;
	    	is = (int)as;
		if(is==ns-1) is=ns-2;
		res = as-is;
		if(res<=0.01) res = 0.0;
		if(res>=0.99) res = 1.0;
		sum2(nxt,nzt,1-res,res,ttab[is],ttab[is+1],tt);
		sum2(nxt,nzt,1,1,tt,tsum,tsum);
		if(npv)  {
			sum2(nxt,nzt,1-res,res,tv[is],tv[is+1],tt);
			sum2(nxt,nzt,1,1,tt,tvsum,tvsum);
			sum2(nxt,nzt,1-res,res,cs[is],cs[is+1],tt);
			sum2(nxt,nzt,1,1,tt,cssum,cssum);
		}

		mig2d(tr.data,nt,ft,dt,sx,gx,mig[io],aperx,
		  nxo,fxo,dxo,nzo,fzo,dzo,
		  ls,mtmax,dxm,fmax,angmax,
		  tb,pb,cs0b,angb,nr,tsum,nzt,fzt,dzt,nxt,fxt,dxt,
		  npv,cssum,tvsum,mig1[io]);

		  ktr++;
		  if((jtr-1)%mtr ==0 ){
			fprintf(jpfp," migrated trace %d\n",jtr);
			fflush(jpfp);
	    	}
	    }
	    jtr++;
	} while (fgettr(infp,&tr) && jtr<ntr);

	fprintf(jpfp," migrated %d traces in total\n",ktr);

	memset((void *) &tro, 0, sizeof(segy));
	tro.ns = nzo;
	tro.d1 = dzo;
	tro.dt = 1000*(int)(1000*dt+0.5);
	tro.delrt = 0.0;
	tro.f1 = fzo;
	tro.f2 = fxo;
	tro.d2 = dxo;
	tro.trid = 200;

	scal = 4/sqrt(PI)*dxm/v0;
	for(ixo=0; ixo<nxo; ixo++) {
		for(io=0; io<noff; io++)  {
			memcpy((void *) tro.data,
				 (const void *) mig[io][ixo], nzo*sizeof(float));
			tro.offset = off0+io*doff;
			tro.tracr = 1+ixo;
			tro.tracl = 1+io+noff*ixo;
			tro.cdp = fxo+ixo*dxo;
			tro.cdpt = 1+io;

			for(izo=0; izo<nzo; ++izo)
				tro.data[izo] *=scal;

			/* write out */
			fputtr(outfp,&tro); 

		   if(npv){
			memcpy((void *) tro.data,
				 (const void *) mig1[io][ixo],nzo*sizeof(float));
			for(izo=0; izo<nzo; ++izo)
				tro.data[izo] *=scal;
			/* write out */
			fputtr(out1fp,&tro); 
		    }		 
		}
	}

	fprintf(jpfp," \n");
	fprintf(jpfp," output done\n");
	fflush(jpfp);

	efclose(jpfp);
	efclose(outfp);

	    
	free2float(tsum);
	free2float(tt);
	free2float(pb);
	free2float(tb);
	free2float(cs0b);
	free2float(angb);
	free3float(ttab);
	free3float(mig);
	free3float(mig1);
	if(npv){
		free3float(tv);
		free3float(cs);
		free2float(tvsum);
		free2float(cssum);
	}
	su2cs->setEOF();
	pthread_exit(NULL);
	return retPtr;
}
catch( cseis_geolib::csException& exc ) {
  su2cs->setError("%s",exc.getMessage());
  pthread_exit(NULL);
  return retPtr;
}
}

/* residual traveltime calculation based  on reference   time	*/
  void resit(int nx,float fx,float dx,int nz,int nr,float dr,
		float **tb,float **t,float x0)
{
	int ix,iz,jr;
	float xi,ar,sr,sr0;

	for(ix=0; ix<nx; ++ix){
		xi = fx+ix*dx-x0;
		ar = abs(xi)/dr;
		jr = (int)ar;
		sr = ar-jr;
		sr0 = 1.0-sr;
		if(jr>nr-2) jr = nr-2;
		for(iz=0; iz<nz; ++iz)
			t[ix][iz] -= sr0*tb[jr][iz]+sr*tb[jr+1][iz];
	}
} 

/* lateral interpolation	*/

/* sum of two tables	*/
  void sum2(int nx,int nz,float a1,float a2,float **t1,float **t2,float **t)
{
	int ix,iz;

	for(ix=0; ix<nx; ++ix) 
		for(iz=0; iz<nz; ++iz)
			t[ix][iz] = a1*t1[ix][iz]+a2*t2[ix][iz];
}
 
/* compute  reference traveltime and slowness	*/
	void timeb(int nr,int nz,float dr,float dz,float fz,float a,
	float v0,float **t,float **p,float **cs0,float **ang)
{
	int  ir,iz;
	float r,z,v,rc,oa,temp,rou,zc;


	if( a==0.0) {
		for(ir=0,r=0;ir<nr;++ir,r+=dr)
			for(iz=0,z=fz;iz<nz;++iz,z+=dz){
				rou = sqrt(r*r+z*z);
				if(rou<dz) rou = dz;
				t[ir][iz] = rou/v0;
				p[ir][iz] = r/(rou*v0);
				cs0[ir][iz] = z/rou;
				ang[ir][iz] = asin(r/rou);
			}
	} else {
		oa = 1.0/a; 	zc = v0*oa;
		for(ir=0,r=0;ir<nr;++ir,r+=dr)
			for(iz=0,z=fz+zc;iz<nz;++iz,z+=dz){
				rou = sqrt(r*r+z*z);
				v = v0+a*(z-zc);
				if(ir==0){ 
					t[ir][iz] = log(v/v0)*oa;
					p[ir][iz] = 0.0;
					ang[ir][iz] = 0.0;
					cs0[ir][iz] = 1.0;
				} else {
					rc = (r*r+z*z-zc*zc)/(2.0*r);
					rou = sqrt(zc*zc+rc*rc);
					t[ir][iz] = log((v*(rou+rc))
						/(v0*(rou+rc-r)))*oa;
					p[ir][iz] = sqrt(rou*rou-rc*rc)
						/(rou*v0);
					temp = v*p[ir][iz];
					if(temp>1.0) temp = 1.0;
					ang[ir][iz] = asin(temp);
					cs0[ir][iz] = rc/rou;
				}
			}
	}
}

void filt(float *trace,int nt,float dt,float fmax,int ls,int m,float *trf);

  void mig2d(float *trace,int nt,float ft,float dt,
	float sx,float gx,float **mig,float aperx,
  	int nx,float fx,float dx,float nz,float fz,float dz,
	int ls,int mtmax,float dxm,float fmax,float angmax,
	float **tb,float **pb,float **cs0b,float **angb,int nr,
	float **tsum,int nzt,float fzt,float dzt,int nxt,float fxt,float dxt,
	int npv,float **cssum,float **tvsum,float **mig1)
/*****************************************************************************
Migrate one trace 
******************************************************************************
Input:
*trace		one seismic trace 
nt		number of time samples in seismic trace
ft		first time sample of seismic trace
dt		time sampleing interval in seismic trace
sx,gx		lateral coordinates of source and geophone 
aperx		lateral aperature in migration
nx,fx,dx,nz,fz,dz	dimension parameters of migration region
ls		=1 for line source; =0 for point source
mtmax		number of time samples in triangle filter
dxm		midpoint sampling interval
fmax		frequency-highcut for input trace	 
angmax		migration angle aperature from vertical 	 
tb,pb,cs0b,angb		reference traveltime, lateral slowness, cosine of 
		incident angle, and emergent angle
nr		number of lateral samples in reference quantities
tsum		sum of residual traveltimes from shot and receiver
nxt,fxt,dxt,nzt,fzt,dzt		dimension parameters of traveltime table
npv=0		flag of computing quantities for velocity analysis
cssume		sum of cosine of emergence angles from shot and recerver 
tvsum		sum of  traveltime variations from shot and recerver 

Output:
mig		migrated section
mig1		additional migrated section for velocity analysis if npv>0
*****************************************************************************/
{
	int nxf,nxe,nxtf,nxte,ix,iz,iz0,izt0,nzp,jrs,jrg,jz,jt,mt,jx;
	float xm,x,dis,rxz,ar,srs,srg,srs0,srg0,sigp,z0,rdz,ampd,res0,
		angs,angg,cs0s,cs0g,ax,ax0,pmin,
		odt=1.0/dt,pd,az,sz,sz0,at,td,res,temp;
	float **tmt,**ampt,**ampti,**ampt1=NULL,*tm,*amp,*ampi,*amp1=NULL,
		*tzt,*trf,*zpt;

	tmt = alloc2float(nzt,nxt);
	ampt = alloc2float(nzt,nxt);
	ampti = alloc2float(nzt,nxt);
	tm = alloc1float(nzt);
	tzt = alloc1float(nzt);
	amp = alloc1float(nzt);
	ampi = alloc1float(nzt);
	zpt = alloc1float(nxt);
	trf = alloc1float(nt+2*mtmax);
	if(npv) {
		ampt1 = alloc2float(nzt,nxt);
		amp1 = alloc1float(nzt);
	}

	z0 = (fz-fzt)/dzt;
	rdz = dz/dzt;
	pmin = 1.0/(2.0*dxm*fmax);
	
	filt(trace,nt,dt,fmax,ls,mtmax,trf);

	xm = 0.5*(sx+gx);
	rxz = (angmax==90)?0.0:1.0/tan(angmax*PI/180.);
	nxtf = (xm-aperx-fxt)/dxt;
	if(nxtf<0) nxtf = 0;
	nxte = (xm+aperx-fxt)/dxt+1;
	if(nxte>=nxt) nxte = nxt-1;

	/* compute amplitudes and filter length	*/
	for(ix=nxtf; ix<=nxte; ++ix){
		x = fxt+ix*dxt;
		dis = (xm>=x)?xm-x:x-xm;
		izt0 = ((dis-dxt)*rxz-fzt)/dzt-1;
		if(izt0<0) izt0 = 0;
		if(izt0>=nzt) izt0 = nzt-1;

		ar = (sx>=x)?(sx-x)/dx:(x-sx)/dx;
		jrs = (int)ar;
		if(jrs>nr-2) jrs = nr-2;
		srs = ar-jrs;
		srs0 = 1.0-srs;
		ar = (gx>=x)?(gx-x)/dx:(x-gx)/dx;
		jrg = (int)ar;
		if(jrg>nr-2) jrg = nr-2;
		srg = ar-jrg;
		srg0 = 1.0-srg;
		sigp = ((sx-x)*(gx-x)>0)?1.0:-1.0;
		zpt[ix] = fzt+(nzt-1)*dzt;

		for(iz=izt0; iz<nzt; ++iz){
			angs = srs0*angb[jrs][iz]+srs*angb[jrs+1][iz]; 
			angg = srg0*angb[jrg][iz]+srg*angb[jrg+1][iz]; 
			cs0s = srs0*cs0b[jrs][iz]+srs*cs0b[jrs+1][iz]; 
			cs0g = srg0*cs0b[jrg][iz]+srg*cs0b[jrg+1][iz]; 
			ampd = (cs0s+cs0g)*cos(0.5*(angs-angg));
			if(ampd<0.0) ampd = -ampd;
			ampt[ix][iz] = ampd;

			pd = srs0*pb[jrs][iz]+srs*pb[jrs+1][iz]+sigp 
			     *(srg0*pb[jrg][iz]+srg*pb[jrg+1][iz]);
			if(pd<0.0) pd = -pd;
			temp = pd*dxm*odt;
			if(temp<1) temp = 1.0;
			if(temp>mtmax) temp = mtmax;
			ampti[ix][iz] = ampd/(temp*temp);
			tmt[ix][iz] = temp;
			if(pd<pmin && zpt[ix]>fzt+(nzt-1.1)*dzt) 
				zpt[ix] = fzt+iz*dzt;

		    if(npv){
			if(cssum[ix][iz]<1.0) 
			     ampt1[ix][iz] = 0; 
			else 
			     ampt1[ix][iz] = tvsum[ix][iz]/cssum[ix][iz];
		    }
		}
	}

	nxf = (xm-aperx-fx)/dx+0.5;
	if(nxf<0) nxf = 0;
	nxe = (xm+aperx-fx)/dx+0.5;
	if(nxe>=nx) nxe = nx-1;
	
	/* interpolate amplitudes and filter length along lateral	*/
	for(ix=nxf; ix<=nxe; ++ix){
		x = fx+ix*dx;
		dis = (xm>=x)?xm-x:x-xm;
		izt0 = (dis*rxz-fzt)/dzt;
		if(izt0<0) izt0 = 0;
		if(izt0>=nzt) izt0 = nzt-1;
		iz0 = (dis*rxz-fz)/dz;
		if(iz0<0) iz0 = 0;
		if(iz0>=nz) iz0 = nz-1;

		ax = (x-fxt)/dxt;
		jx = (int)ax;
		ax = ax-jx;
		if(ax<=0.01) ax = 0.;
		if(ax>=0.99) ax = 1.0;
		ax0 = 1.0-ax;
		if(jx>nxte-1) jx = nxte-1;
		if(jx<nxtf) jx = nxtf;

		ar = (sx>=x)?(sx-x)/dx:(x-sx)/dx;
		jrs = (int)ar;
		if(jrs>nr-2) jrs = nr-2;
		srs = ar-jrs;
		srs0 = 1.0-srs;
		ar = (gx>=x)?(gx-x)/dx:(x-gx)/dx;
		jrg = (int)ar;
		if(jrg>nr-2) jrg = nr-2;
		srg = ar-jrg;
		srg0 = 1.0-srg;

		for(iz=izt0; iz<nzt; ++iz){
		    tzt[iz] = ax0*tsum[jx][iz]+ax*tsum[jx+1][iz]
				+srs0*tb[jrs][iz]+srs*tb[jrs+1][iz]
				+srg0*tb[jrg][iz]+srg*tb[jrg+1][iz];

		    amp[iz] = ax0*ampt[jx][iz]+ax*ampt[jx+1][iz];
		    ampi[iz] = ax0*ampti[jx][iz]+ax*ampti[jx+1][iz];
		    tm[iz] = ax0*tmt[jx][iz]+ax*tmt[jx+1][iz];

		    if(npv) 
		    	amp1[iz] = ax0*ampt1[jx][iz]+ax*ampt1[jx+1][iz];

		}

		nzp = (ax0*zpt[jx]+ax*zpt[jx+1]-fz)/dz+1.5;
		if(nzp<iz0) nzp = iz0;
		if(nzp>nz) nzp = nz;

		/* interpolate along depth if operater aliasing 	*/
		for(iz=iz0; iz<nzp; ++iz) {
			az = z0+iz*rdz;
			jz = (int)az;
			if(jz>=nzt-1) jz = nzt-2;
			sz = az-jz;
			sz0 = 1.0-sz;
			td = sz0*tzt[jz]+sz*tzt[jz+1];
			at = (td-ft)*odt+mtmax;
			jt = (int)at;
			if(jt > mtmax && jt < nt+mtmax-1){
			    ampd = sz0*ampi[jz]+sz*ampi[jz+1];
			    mt = (int)(0.5+sz0*tm[jz]+sz*tm[jz+1]);
			    res = at-jt;
			    res0 = 1.0-res;
 			    temp = (res0*(-trf[jt-mt]+2.0*trf[jt]-trf[jt+mt]) 
				+res*(-trf[jt-mt+1]+2.0*trf[jt+1]
				-trf[jt+mt+1]))*ampd;
			    mig[ix][iz] += temp;

			    if(npv) 
				mig1[ix][iz]  += temp
					*(sz0*amp1[jz]+sz*amp1[jz+1]);
			}
		}

		/* interpolate along depth if not operater aliasing 	*/
		for(iz=nzp; iz<nz; ++iz) {
			az = z0+iz*rdz;
			jz = (int)az;
			if(jz>=nzt-1) jz = nzt-2;
			sz = az-jz;
			sz0 = 1.0-sz;
			td = sz0*tzt[jz]+sz*tzt[jz+1];
			at = (td-ft)*odt;
			jt = (int)at;
			if(jt > 0 && jt < nt-1){
			    ampd = sz0*amp[jz]+sz*amp[jz+1];
			    res = at-jt;
			    res0 = 1.0-res;
 			    temp = (res0*trace[jt]+res*trace[jt+1])*ampd; 
			    mig[ix][iz] += temp;
			    if(npv) 
				mig1[ix][iz]  += temp
					*(sz0*amp1[jz]+sz*amp1[jz+1]);
			}
		}

	}

	free2float(ampt);
	free2float(ampti);
	free2float(tmt);
	free1float(amp);
	free1float(ampi);
	free1float(zpt);
	free1float(tm);
	free1float(tzt);
	free1float(trf);
	if(npv) {
		free1float(amp1);
		free2float(ampt1);
	}
}

void filt(float *trace,int nt,float dt,float fmax,int ls,int m,float *trf)
/* Low-pass filter, integration and phase shift for input data	 
   input: 
    trace(nt)	single seismic trace
   fmax	high cut frequency
    ls		ls=1, line source; ls=0, point source
  output:
    trace(nt) 	filtered and phase-shifted seismic trace 
    tracei(nt) 	filtered, integrated and phase-shifted seismic trace 
 */
{
	static int nfft=0, itaper, nw, nwf;
	static float *taper, *amp, *ampi, dw;
	int  it, iw, itemp;
	float temp, ftaper, const2, *rt;
	complex *ct;

	fmax *= 2.0*PI;
	ftaper = 0.1*fmax;
	const2 = sqrt(2.0);

	if(nfft==0) {
	  	/* Set up FFT parameters */
	  	nfft = npfaro(nt+m, 2 * (nt+m));
	  	if (nfft >= SU_NFLTS || nfft >= 720720)
		    	throw cseis_geolib::csException("Padded nt=%d -- too big", nfft);

	  	nw = nfft/2 + 1;
		dw = 2.0*PI/(nfft*dt);

		itaper = 0.5+ftaper/dw;
		taper = ealloc1float(2*itaper+1);
		for(iw=-itaper; iw<=itaper; ++iw){
			temp = (float)iw/(1.0+itaper); 
			taper[iw+itaper] = (1-temp)*(1-temp)*(temp+2)/4;
		}

		nwf = 0.5+fmax/dw;
		if(nwf>nw-itaper-1) nwf = nw-itaper-1;
		amp = ealloc1float(nwf+itaper+1);
		ampi = ealloc1float(nwf+itaper+1);
		amp[0] = ampi[0] = 0.;
		for(iw=1; iw<=nwf+itaper; ++iw){
			amp[iw] = sqrt(dw*iw)/nfft;
			ampi[iw] = 0.5/(1-cos(iw*dw*dt));
		}
	}

	  /* Allocate fft arrays */
	  rt   = ealloc1float(nfft);
	  ct   = ealloc1complex(nw);

	  memcpy(rt, trace, nt*FSIZE);
	  memset((void *) (rt + nt), (int) '\0', (nfft-nt)*FSIZE); 
	  pfarc(1, nfft, rt, ct);

	for(iw=nwf-itaper;iw<=nwf+itaper;++iw){
		itemp = iw-(nwf-itaper);
		ct[iw].r = taper[itemp]*ct[iw].r; 
		ct[iw].i = taper[itemp]*ct[iw].i; 
	}
	for(iw=nwf+itaper+1;iw<nw;++iw){
		ct[iw].r = 0.; 
		ct[iw].i = 0.; 
	}

	 	if(!ls){
		for(iw=0; iw<=nwf+itaper; ++iw){
			/* phase shifts PI/4 	*/
			temp = (ct[iw].r-ct[iw].i)*amp[iw]*const2;
			ct[iw].i = (ct[iw].r+ct[iw].i)*amp[iw]*const2;
			ct[iw].r = temp;
		    }
	} else {
		for(iw=0; iw<=nwf+itaper; ++iw){
			ct[iw].i = ct[iw].i*amp[iw];
			ct[iw].r = ct[iw].r*amp[iw];
		}
	}		  
	  pfacr(-1, nfft, ct, rt);
		
	  /* Load traces back in */
	for (it=0; it<nt; ++it) trace[it] = rt[it];

	  /* Integrate traces   */
	for(iw=0; iw<=nwf+itaper; ++iw){
		ct[iw].i = ct[iw].i*ampi[iw];
		ct[iw].r = ct[iw].r*ampi[iw];
	}
	  pfacr(-1, nfft, ct, rt);
	  for (it=0; it<m; ++it)  trf[it] = rt[nfft-m+it];
	  for (it=0; it<nt+m; ++it)  trf[it+m] = rt[it];

	free1float(rt);
	free1complex(ct);
}




} // END namespace
