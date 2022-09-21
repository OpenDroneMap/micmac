#include "include/MMVII_all.h"
#ifdef _OPENMP
#include <omp.h>
#endif


/**
   \file cCentralPerspCam.cpp

   \brief implementation  of functionnality for intrincic calibration of 
*/

using namespace NS_SymbolicDerivative;

namespace MMVII
{


class cPixelDomain ;
class cCalibStenPerfect ;
class cPerspCamIntrCalib ;
	
class cPixelDomain : public cDataBoundedSet<tREAL8,2>
{
      public :
           cPixelDomain(const cPt2di &aSz);
           virtual ~ cPixelDomain();
           virtual cPixelDomain *  Dup_PS () const;  ///< default work because deleted in mother class

      private :
           cPt2di     mSz;
};




class cCalibStenPerfect : public cDataInvertibleMapping<tREAL8,2>
{
     public :
         typedef tREAL8               tScal;
         typedef cPtxd<tScal,2>       tPt;
         typedef std::vector<tPt>     tVecPt;

	 cCalibStenPerfect(tScal aFoc,const tPt & aPP);
         cCalibStenPerfect(const cCalibStenPerfect & aPS);  ///< default wouldnt work because deleted in mother class
	 cCalibStenPerfect MapInverse() const;

	 tPt  Value(const tPt& aPt) const override {return mPP + aPt*mF;}
	 const  tVecPt &  Inverses(tVecPt &,const tVecPt & ) const override;
	 const  tVecPt &  Values(tVecPt &,const tVecPt & ) const override;

     private :
         tScal  mF;   ///<  Focal
         tPt    mPP;  ///<  Principal point
};




/** this the class for computing the intric calibration of perspective camera :
 
    The intric calibration compute the maping from a 3D camera coordinate to image coordinates.
    So it is a mapping R3 -> R2,   and as is heritates from cDataMapping<tREAL8,3,2>

    The mapping  is made by compination  of 3 functions :

         * mProjDir R3->R2 , the projection function, it can be stenope(x/z,y/z), fish-eye , 360 degre -> (teta,phi)  ...
	   it belongs to a finite set of  possibility code by enumeration eProjPC;  for each model it has no parameter

	 * dirtortion  R2->R2 , its a function close to identity (at least ideally)

 */

class cPerspCamIntrCalib : public cDataMapping<tREAL8,3,2>
{
	public :
            typedef tREAL8               tScal;
            typedef cPtxd<tScal,2>       tPtOut;
            typedef cPtxd<tScal,3>       tPtIn;
            typedef std::vector<tPtIn>   tVecIn;
            typedef std::vector<tPtOut>  tVecOut;

	    cPerspCamIntrCalib
            (
                  eProjPC        aTypeProj,           ///< type of projection 
		  const cPt3di & aDeg,                ///< degrees of distorstion  Rad/Dec/Univ
		  const std::vector<double> & aVParams,  ///< vector of distorsion
		  const cCalibStenPerfect &,           ///< Calib w/o dist
                  const  cPixelDomain  &,              ///< sz, domaine of validity in pixel
		  const cPt3di & aDegPseudoInv,       ///< degree of inverse approx by least square
		  int aSzBuf                          ///< sz of buffers in computatio,
            );

	    ~cPerspCamIntrCalib();

	    ///  Update parameter of lsq-peudso-inverse distorsion taking into account direct
	    void UpdateLSQDistInv();

	     // const  tVecOut &  Values(tVecOut &,const tVecIn & ) const override;
	    // const  tVecOut &  Inverses(tVecIn &,const tVecOut & ) const;
	private :

            // cSphereBoundedSet<tREAL8,2>          mPNormSpace; // validity domain pixel-normalize (PP/F) space


	        // comon to dir & inverse
	    eProjPC                              mTypeProj;
            int                                  mSzBuf;
	        // parameters for direct projection  DirBundle -> pixel
	    cPt3di                               mDir_Degr;
	    std::vector<cDescOneFuncDist>        mDir_VDesc;  ///< contain a "high" level description of dist params
	    std::vector<tREAL8>                  mDir_Params;    ///< Parameters of distorsion
            cDataMapCalcSymbDer<tREAL8,3,2>*     mDir_Proj;   ///< direct projection  R3->R2
            cDataNxNMapCalcSymbDer<tREAL8,2>*    mDir_Dist;   ///< direct disorstion  R2->R2
	    cCalibStenPerfect                    mCSPerfect;  ///< R2-phgr -> pixels
            cPixelDomain *                       mPixDomain;  ///< validity domain in pixel
                // now for "inversion"  pix->DirBundle
	    cCalibStenPerfect                    mInv_CSP;
	    cDataMappedBoundedSet<tREAL8,2>*     mPhgrDomain;  ///<  validity in F/PP corected space,
	    cPt3di                               mInv_Degr;
	    std::vector<cDescOneFuncDist>        mInv_VDesc;  ///< contain a "high" level description of dist params
	    std::vector<tREAL8>                  mInv_Params;    ///< Parameters of distorsion
            cDataNxNMapCalcSymbDer<tREAL8,2>*    mInvApproxLSQ_Dist;   ///< approximate LSQ invert disorstion  R2->R2
	    cCalculator<tREAL8> *                mInv_BaseFDist;  ///<  base of function for inverse distortion
            cLeastSqCompMapCalcSymb<tREAL8,2,2>* mInv_CalcLSQ;  ///< structure for least square estimation
	    tREAL8                               mThreshJacPI; ///< threshlod for jacobian in pseudo inversion
            // cDataMapCalcSymbDer<tREAL8,3,2>   * mProjInv;
};

/* ******************************************************* */
/*                                                         */
/*                 cPerspCamIntrCalib                      */
/*                                                         */
/* ******************************************************* */

cPerspCamIntrCalib::cPerspCamIntrCalib
(
      eProjPC        aTypeProj,           ///< type of projection 
      const cPt3di & aDegDir,             ///< degrees of distorstion  Rad/Dec/Univ
      const std::vector<double> & aVParams,  ///< vector of constants, or void
      const cCalibStenPerfect & aCSP,           ///< Calib w/o dist
      const  cPixelDomain  & aPixDomain,              ///< sz, domaine of validity in pixel
      const cPt3di & aDegPseudoInv,       ///< degree of inverse approx by least square
      int aSzBuf                          ///< sz of buffers in computation
)  :
	// ------------ global -------------
    mTypeProj           (aTypeProj),
    mSzBuf              (aSzBuf),
	// ------------ direct -------------
    mDir_Degr           (aDegDir),
    mDir_VDesc          (DescDist(aDegDir)),
    mDir_Params         (aVParams),
    mDir_Proj           (nullptr),
    mDir_Dist           (nullptr),
    mCSPerfect          (aCSP),
    mPixDomain          (aPixDomain.Dup_PS()),
	// ------------ inverse -------------
    mInv_CSP            (mCSPerfect.MapInverse()),
    mPhgrDomain         (new cDataMappedBoundedSet<tREAL8,2>(mPixDomain,&mInv_CSP,false,false)),
    mInv_Degr           (aDegPseudoInv),
    mInv_VDesc          (DescDist(mInv_Degr)),
    mInv_Params         (mInv_VDesc.size(),0.0),
    mInvApproxLSQ_Dist  (nullptr),
    mInv_BaseFDist      (nullptr),
    mInv_CalcLSQ        (nullptr),
    mThreshJacPI        (0.5)
{
        // 1 - construct direct parameters
	
    // correct vect param, when first use, parameter can be empty meaning all 0  
    if (mDir_Params.size() != mDir_VDesc.size())
    {
       MMVII_INTERNAL_ASSERT_strong(mDir_Params.empty(),"cPerspCamIntrCalib Bad size for params");
       mDir_Params.resize(mDir_VDesc.size(),0.0);
    }
    
    mDir_Proj = new  cDataMapCalcSymbDer<tREAL8,3,2>
                     (
                          EqCPProjDir(mTypeProj,false,mSzBuf),   // equatio, w/o derivative
                          EqCPProjDir(mTypeProj,true,mSzBuf),    // equation with derivatives
			  std::vector<double>(),                 // parameters, empty here
			  true                                   // equations are "adopted" (i.e will be deleted in destuctor)
                     );

    mDir_Dist = NewMapOfDist(mDir_Degr,mDir_Params,mSzBuf);

        // 2 - construct direct parameters
    mInvApproxLSQ_Dist  = NewMapOfDist(mInv_Degr,mInv_Params,mSzBuf);
    mInv_BaseFDist = EqBaseFuncDist(mInv_Degr,mSzBuf);
    mInv_CalcLSQ   = new cLeastSqCompMapCalcSymb<tREAL8,2,2>(mInv_BaseFDist);

}

void cPerspCamIntrCalib::UpdateLSQDistInv()
{
	/*
    cComputeMapInverse
    (
       mThreshJacPI, ///< Threshold on jacobian to ensure inversability
       cPt2dr(0,0),       ///< Seed point, in input space
       mInv_VDesc.size(),    ///< Approximate number of point (in the biggest size), here +or- less square of minimum
             tSet &,  ///< Set of validity, in output space
             tMap&,   ///< Maping to invert : InputSpace -> OutputSpace
       (*mInv_BaseFDist),  ///< Structure for computing the invert on base of function using least square
       false  // Test
   );
   */

}

cPerspCamIntrCalib::~cPerspCamIntrCalib()
{
     delete mPhgrDomain;	
     delete mPixDomain;	
     delete mDir_Dist;
     delete mDir_Proj;

     delete mInvApproxLSQ_Dist;
     delete mInv_BaseFDist;
     delete mInv_CalcLSQ;
}

void BenchCentralePerspective(cParamExeBench & aParam)
{
    if (! aParam.NewBench("CentralPersp")) return;

    eProjPC  aTypeProj = eProjPC::eFE_EquiSolid;

    cPerspCamIntrCalib aCam
    (
          aTypeProj,
	  cPt3di(3,1,1),
	  std::vector<double>(),
	  cCalibStenPerfect(2000,cPt2dr(1520.0,1030.0)),
	  cPixelDomain(cPt2di(3000,2000)),
	  cPt3di(3,1,1),
	  100
    );

    aParam.EndBench();
}

/* ******************************************************* */
/*                                                         */
/*                    cPixelDomain                         */
/*                                                         */
/* ******************************************************* */

cPixelDomain::~cPixelDomain()
{
}

cPixelDomain::cPixelDomain(const cPt2di &aSz) :
     cDataBoundedSet<tREAL8,2>(cBox2dr(cPt2dr(0,0),ToR(aSz))),
     mSz  (aSz)
{
}

cPixelDomain *  cPixelDomain::Dup_PS () const
{
    return new cPixelDomain(mSz);
}

/* ******************************************************* */
/*                                                         */
/*                 cCalibStenPerfect                        */
/*                                                         */
/* ******************************************************* */

cCalibStenPerfect::cCalibStenPerfect(tScal aFoc,const tPt & aPP) :
    mF   (aFoc),
    mPP  (aPP)
{
}
cCalibStenPerfect::cCalibStenPerfect(const cCalibStenPerfect & aCS) :
    cCalibStenPerfect(aCS.mF,aCS.mPP)
{
}

cCalibStenPerfect cCalibStenPerfect::MapInverse() const
{
    //  aQ= PP+ aP * F  ;  aP = (aQ-PP) /aF
    return  cCalibStenPerfect(  1.0/mF  ,  -mPP/mF  );
}

const  typename cCalibStenPerfect::tVecPt &  cCalibStenPerfect::Values(tVecPt & aVOut,const tVecPt & aVIn) const
{
     const size_t aNbIn = aVIn.size();
     aVOut.resize(aNbIn);

#ifdef _OPENMP
#pragma omp parallel for
#endif
     for (size_t aK=0; aK < aNbIn; aK++) 
     {
	     aVOut[aK] = mPP + aVIn[aK] * mF;
     }
     return aVOut;
}

const  typename cCalibStenPerfect::tVecPt &  cCalibStenPerfect::Inverses(tVecPt & aVOut,const tVecPt & aVIn) const
{
     const size_t aNbIn = aVIn.size();
     aVOut.resize(aNbIn);

#ifdef _OPENMP
#pragma omp parallel for
#endif
     for (size_t aK=0; aK < aNbIn; aK++) 
     {
	     aVOut[aK] = (aVIn[aK] - mPP) / mF;
     }
     return aVOut;
}




}; // MMVII

