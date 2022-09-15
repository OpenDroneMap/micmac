#include "include/MMVII_all.h"

#include "include/SymbDer/SymbolicDerivatives.h"
#include "include/SymbDer/SymbDer_GenNameAlloc.h"


using namespace NS_SymbolicDerivative;
using namespace MMVII;

namespace MMVII
{

   /* ======  header of header  ====== */

/**  class are not templated, because there is no evidence that there would be any benefit, but just 
 * in case, keep the possibility of templating more easily if required */

typedef tREAL8  tCoordDevTri;
typedef  cTriangulation3D<tCoordDevTri> tTriangulation3D;
typedef  cTriangle<tCoordDevTri,3>      tTri3D;
typedef  cIsometry3D<tCoordDevTri>      tIsom3D;
typedef  cSimilitud3D<tCoordDevTri>     tSim3D;
typedef  cPtxd<tCoordDevTri,3>          tPt3D;
typedef  cPtxd<tCoordDevTri,2>          tPt2D;
typedef cTriangle<int,2> tTriPix;
typedef cResolSysNonLinear<tCoordDevTri> tSys;
typedef cDenseVect<tCoordDevTri>         tDenseV;
typedef cCalculator<tCoordDevTri>        tCalc;

class cSomFace3D;
class cSomDevT3D;
class cFaceDevT3D;
class cDevTriangu3d;

   /* ======================= */
   /* ======  header   ====== */
   /* ======================= */

/** There is thing common to submit and faces, put in mother class cSomFace3D */

class cSomFace3D
{
     public :
        void SetReached(int aNumStepReach) { mNumStepReach=aNumStepReach;}
        bool IsReached()  const {return mNumStepReach >= 0;}

        cSomFace3D (cDevTriangu3d * aDevTri,int aNum) : 
             mDevTri       (aDevTri),
             mNumStepReach (-1) ,
             mNumObj       (aNum)
        {
        }
        int  NumObj() const {return mNumObj;} ///< accessor
     protected :
        cDevTriangu3d  *   mDevTri;        ///< the global object for triangulation-development
        int                mNumStepReach;  ///<  num of the step where object was reachedd by progressive devlopment
        int                mNumObj;        ///<  id of the object
};

class cSomDevT3D : public cSomFace3D
{
    public :
        cSomDevT3D(cDevTriangu3d * aDevTri,int aNum,const tPt3D & aP3);
	/// add one more measur, for growing process
	void  AddPt2(const tPt2D & aPt,tCoordDevTri aWeight);
	///  set point for
	void  SetPt2(const tPt2D & aPt);
	const tPt2D & Pt2() const;
	const tPt3D & Pt3() const;
        int  NumX() const;               
        int  NumY() const;               
    private :
        tPt3D         mPt3;
        tPt2D         mPt2;
	tCoordDevTri  mSomWInit;  ///< sum of weight on initialization, because a submit can be reache by several faces in on step

};

class cFaceDevT3D : public cSomFace3D
{
   public :
      cFaceDevT3D (cDevTriangu3d * aDevTri,int aNumF,cPt3di aIndSom);
      int IndKthSom(int aK) const;

      tCoordDevTri  DistortionDist(tCoordDevTri & aSomDiff,tCoordDevTri & aSomDist) const;
      
   private :
      cPt3di           mIndSoms;
};


/** Class that effectively compute the "optimal" devlopment of a surface
 * Separate from cAppliMeshDev to be eventually reusable
 */

class cDevTriangu3d
{
      public :
          typedef typename cTriangulation<tCoordDevTri,3>::tFace tFace;

          static constexpr int NO_STEP = -1;

          cDevTriangu3d(tTriangulation3D &,tCoordDevTri aFactRand);
          ~cDevTriangu3d();

          /// Export devloped surface as ply file
	  void ExportDev(const std::string &aName) const;
	  
          /// Show statistics on geometry preservation : distance & angles
	  tCoordDevTri GlobDistortiontDist() const;


	  const tTriangulation3D & Tri() const;  ///< accessor
          cSomDevT3D&   KthSom(size_t aK) ; ///< access to mVSoms

          void TestGroundTruth2D(const tTriangulation3D &);  ///<  Test with a ground truth

      private :
	  cDevTriangu3d(const cDevTriangu3d &) = delete;
	  void  AddOneFace(int aKFace,bool IsFaceC);
          void  OneIterationCompens();


	  // tPt3D


	  int               mNumCurStep;       ///< num of iteration in devlopment, +- dist to center face
	  const tTriangulation3D & mTri;       ///< reference to the 3D-triangulation to devlop
          tCoordDevTri             mFactRand;
          const cGraphDual &       mDualGr;    ///< reference to the dual graph
          int               mIndexFC;          ///< Index of centerface
	  cPt3di            mFaceC;            ///< Center face
          int               mNumGen;           ///< Num Gen
          
          std::vector<cSomDevT3D>   mVSoms;
          std::vector<cFaceDevT3D>  mVFaces;
          std::vector<size_t>       mVReachedFaces;
          std::vector<size_t>       mVReachedSoms;
          tSys *                    mSys;          // Non linear sys to compensate rotations
          tCalc *                   mCalcCD;       // calculator for distance conservation
          std::vector<int>          mV3FrozenVar;  // vector contain the index of 3 frozen var 
          tDenseV                   mSolInit;      // solution with first face to create sys
};



/* ******************************************************* */
/*                                                         */
/*                    cSomDevT3D                           */
/*                                                         */
/* ******************************************************* */

cSomDevT3D::cSomDevT3D(cDevTriangu3d * aDevTri,int aNum,const tPt3D & aP3):
    cSomFace3D    (aDevTri,aNum),
    mPt3          (aP3),
    mPt2          (0.0,0.0),  
    mSomWInit     (0.0)
{
}

void  cSomDevT3D::AddPt2(const tPt2D & aPt,tCoordDevTri aWeight)
{
     mPt2 = (mPt2*mSomWInit + aPt*aWeight) / (mSomWInit+aWeight);
     mSomWInit += aWeight;
}

void  cSomDevT3D::SetPt2(const tPt2D & aPt)
{
     mPt2 = aPt;
}


const tPt2D & cSomDevT3D::Pt2() const
{
   MMVII_INTERNAL_ASSERT_tiny(mSomWInit>0,"Bad  cSomFace3D::Pt2");  
   return mPt2;
}
const tPt3D & cSomDevT3D::Pt3() const { return mPt3; }


int  cSomDevT3D::NumX() const {return mNumObj * 2;}
int  cSomDevT3D::NumY() const {return NumX() + 1;}

/* ******************************************************* */
/*                                                         */
/*                    cFaceDevT3D                          */
/*                                                         */
/* ******************************************************* */

cFaceDevT3D::cFaceDevT3D(cDevTriangu3d * aDevTri,int aNumF,cPt3di aIndSom) :
    cSomFace3D    (aDevTri,aNumF),
    mIndSoms      (aIndSom)
{
}

int cFaceDevT3D::IndKthSom(int aK) const {return mIndSoms[aK];}

tCoordDevTri cFaceDevT3D::DistortionDist(tCoordDevTri& aSomDif,tCoordDevTri& aSomDist ) const
{
    for (int aK=0 ; aK<3 ; aK++)
    {
       cSomDevT3D&  aS1 =   mDevTri->KthSom(mIndSoms[aK]);
       cSomDevT3D&  aS2 =   mDevTri->KthSom(mIndSoms[(aK+1)%3]);
       
       tCoordDevTri aD2 = Norm2(aS1.Pt2()-aS2.Pt2());
       tCoordDevTri aD3 = Norm2(aS1.Pt3()-aS2.Pt3());
       aSomDif += std::abs(aD2-aD3);
       aSomDist += aD2+aD3;
    }

    return aSomDif / aSomDist;
}


/* ******************************************************* */
/*                                                         */
/*                    cDevTriangu3d                        */
/*                                                         */
/* ******************************************************* */

// NS_SymbolicDerivative::cCalculator<double> * EqConsDist(bool WithDerive,int aSzBuf);
//typeded cCalculator<tCoordDevTri>        tCalc;

cDevTriangu3d::cDevTriangu3d(tTriangulation3D & aTri,tCoordDevTri aFactRand) :
     mNumCurStep  (0),
     mTri         (aTri),
     mFactRand    (aFactRand),
     mDualGr      (mTri.DualGr()),
     mIndexFC     (NO_STEP),
     mFaceC       (NO_STEP,NO_STEP,NO_STEP),
     mNumGen      (0),
     mSys         (nullptr),
     mCalcCD      (EqConsDist(true,1)),
     mSolInit     (mTri.NbPts()*2,eModeInitImage::eMIA_Null)
{
   //  generate topology
   aTri.MakeTopo();

   // create the vector of points
   for (size_t aKPt=0 ; aKPt<mTri.NbPts() ; aKPt++)
   {
      mVSoms.push_back(cSomDevT3D(this,aKPt,mTri.KthPts(aKPt)));
   }

   // create the vector of faces
   for (size_t aKF=0 ; aKF<mTri.NbFace() ; aKF++)
   {
       mVFaces.push_back(cFaceDevT3D(this,aKF,mTri.KthFace(aKF)));
   }

   // add fisrt face
   mIndexFC = mTri.IndexCenterFace();
   AddOneFace(mIndexFC,true);

   //  initialise the system
   mSys = new tSys(eModeSSR::eSSR_LsqNormSparse,mSolInit);


   size_t aIndNewF0 = 0;

   // iterate as long as we found new soms
   while (aIndNewF0!=mVReachedFaces.size())
   {
        mNumGen++;
        size_t aIndNewF1 = mVReachedFaces.size();  // memorize size to avoid doing everything in one step

	// parse all face that where reached in previous step
        for (size_t aIndFace=aIndNewF0 ; aIndFace<aIndNewF1 ; aIndFace++)
        {
            std::vector<int> aVN;
	    int aOldF = mVReachedFaces.at(aIndFace);
            mDualGr.GetFacesNeighOfFace(aVN,aOldF); // get all face touching old one
            for (const auto & aNewF : aVN) // parse "new" face
            {
                 cFaceDevT3D & aFace = mVFaces.at(aNewF);
                 if (! aFace.IsReached())  // are they really new ?
                 {
                     AddOneFace(aNewF,false);
                 }
            }
        }

        for (int aKIter=0 ; aKIter<3 ; aKIter++)
        {
            OneIterationCompens();
        }

        aIndNewF0 = aIndNewF1;
   }

   for (int aKIter=0 ; aKIter<6 ; aKIter++)
   {
       OneIterationCompens();
   }


   MMVII_INTERNAL_ASSERT_tiny(mVReachedFaces.size()==mTri.NbFace(),"in Dev : Pb in reached face");  // Check firt point is 0
   MMVII_INTERNAL_ASSERT_tiny(mVReachedSoms.size() ==mTri.NbPts (),"in Dev : Pb in reached face");  // Check firt point is 0
}


cDevTriangu3d::~cDevTriangu3d()
{
    delete mSys;
    delete mCalcCD;
}

const tTriangulation3D & cDevTriangu3d::Tri() const {return  mTri;}
cSomDevT3D&   cDevTriangu3d::KthSom(size_t aKSom) {return mVSoms.at(aKSom);}

void  cDevTriangu3d::AddOneFace(int aKFace,bool IsFaceC)
{
   cFaceDevT3D & aFace = mVFaces.at(aKFace);
   aFace.SetReached(mNumGen);  // mark the face as reached
   mVReachedFaces.push_back(aKFace);  // put it its number in heap of reached one

   //  compute the sum that are reached by this new face
   int aNbNewS = 0;  // compute number of sum newly reached, for check
   int aIndK0=-1;   // compute, if any, the number of the first submit reached
   for (int aK3=0 ; aK3<3; aK3++)
   {
       cSomDevT3D & aSom = mVSoms.at(aFace.IndKthSom(aK3));
       if (! aSom.IsReached())
       {
           aSom.SetReached(mNumGen);
	   mVReachedSoms.push_back(aSom.NumObj());
	   aIndK0=aK3;
           aNbNewS++;
       }
   }
   tTri3D  aTri3D = mTri.KthTri(aKFace);  // 3D triangle

   // If central face, compute a rotation triangle -> plane 0XY
   if (IsFaceC)
   {
      MMVII_INTERNAL_ASSERT_tiny((aNbNewS==3),"Bad size for AddOneFace"); // little check

      int aIndLS = aTri3D.IndexLongestSeg(); // init on longer side for better stability on fix-var
      tIsom3D  aIsometry =  tIsom3D::FromTriOut(aIndLS,aTri3D).MapInverse();   // get rotation Tri-> Plane 0XY

      // compute value for the 3 submit
      for (int aK=0; aK<3 ; aK++)
      {
	  tPt3D aP1 = aTri3D.PtCirc(aK);  // point on triangle
	  tPt3D aQ1 = aIsometry.Value(aP1);  // 3D but on plane
	  int aIndSom =   aFace.IndKthSom(aK);
          cSomDevT3D & aSom =  mVSoms.at(aIndSom);
	  aSom.AddPt2(Proj(aQ1),1.0);

          // register value in initial solution of the least square sys
          mSolInit(aSom.NumX()) = aQ1.x();
          mSolInit(aSom.NumY()) = aQ1.y();

          // ==  memorize index that will be used for handling the gauge on rotation ===
               // if first submit fix translation (fix x and y)
          if (aK==aIndLS)
          {
              mV3FrozenVar.push_back(aSom.NumX());
              mV3FrozenVar.push_back(aSom.NumY());
          }
               // if second submit , fix rotation (fix y)
          if (aK==((aIndLS+1)%3) )
          {
              mV3FrozenVar.push_back(aSom.NumY());
          }

        // if  we are not in mode release, then make some litle check
	  if (The_MMVII_DebugLevel>=The_MMVII_DebugLevel_InternalError_tiny )
	  {
             tPt3D aP2 = aTri3D.PtCirc(aK+1);
             tPt3D aQ2 = aIsometry.Value(aP2);
             MMVII_INTERNAL_ASSERT_bench(std::abs(aQ1.z())<1e-10,"z-init in dev surf"); // check on plane Z=0
             // chek isometry
             MMVII_INTERNAL_ASSERT_bench(std::abs(Norm2(aP1-aP2) - Norm2(aQ1-aQ2))<1e-10,"dist-init in dev surf"); 
	     if (aK==aIndLS)
	     {
                 MMVII_INTERNAL_ASSERT_bench(Norm2(aQ1)<1e-10,"z-init in dev surf");  // Check firt point is 0
                 MMVII_INTERNAL_ASSERT_bench(std::abs(aQ2.y())<1e-10,"z-init in dev surf"); // check  first seg on OX
	     }
 	  }
      }
   }
   // else compute a  similitude triangle->plane that maintain the position of the 2 point already developped
   else
   {
      MMVII_INTERNAL_ASSERT_tiny(aNbNewS<=1,"Bad size for AddOneFace");// must be, and btw cannot handle if not
      if (aNbNewS)  // if 1 new som has been reached
      {
          int aIndK1 = (aIndK0+1)%3; // compute num in tri submit after
          tPt2D   aP1 = mVSoms.at(aFace.IndKthSom(aIndK1)).Pt2();  // compute point afte

          int aIndK2 = (aIndK0+2)%3; // compute num in tri submit before
          tPt2D   aP2 = mVSoms.at(aFace.IndKthSom(aIndK2)).Pt2();  // compute point before

          tSim3D  aSim = tSim3D::FromTriInAndSeg(aP1,aP2,aIndK1,aTri3D); // compute similitude
          tPt2D   aPDev =  Proj(aSim.Value(aTri3D.Pt(aIndK0))); // get image of point by this similitude

          if (mFactRand>0)
          {
              cSegment2DCompiled<tCoordDevTri> aSeg12(aP1,aP2);
              tCoordDevTri  aDist = aSeg12.DistLine(aPDev);
              aPDev += cPt2dr::PRandC() * (mFactRand * aDist);
          }

	  cSomDevT3D & aS0 = mVSoms.at(aFace.IndKthSom(aIndK0));
          aS0.AddPt2(aPDev,1.0);
	  // point in globalsys must be initialized, we us2 aS0.Pt2() in case the submit was initialized by severalface
	  mSys->SetCurSol(aS0.NumX(),aS0.Pt2().x());
	  mSys->SetCurSol(aS0.NumY(),aS0.Pt2().y());
	  //tCoordDevTri aSomDiff=0,aSomDist=0;
          //StdOut() << "   DISTort " << aFace.DistortionDist(aSomDiff,aSomDist)  << "\n";
          // StdOut() <<  aSim.Value(aTri3D.Pt(aIndK1)) << " " <<  aSim.Value(aTri3D.Pt(aIndK2)) << "\n";
          // getchar();
      }
   }

}


void  cDevTriangu3d::OneIterationCompens()
{
    // 1 - fix the submit that are still not devlopped, else they are uncnstrained
    for (const auto & aSom : mVSoms)
    {
        if (!aSom.IsReached())
        {
            mSys->AddEqFixVar(aSom.NumX(),0.0,1.0);
            mSys->AddEqFixVar(aSom.NumY(),0.0,1.0);
        }
    }

    // 2 - gauge fixing, as distance conservation is globally  ambiguous up to a rotation
    for (const auto & aInd : mV3FrozenVar)
    {
        mSys->AddEqFixVar(aInd,0.0,1.0);
    }
    
    // 3 - add the equation for distance conservation

    for (const auto & aKF : mVReachedFaces)
    {
        const cFaceDevT3D& aFace = mVFaces.at(aKF);

        for (int aK3=0 ; aK3<3 ; aK3++)
        {
            const cSomDevT3D & aS1 = mVSoms.at(aFace.IndKthSom(aK3)); // a som
            const cSomDevT3D & aS2 = mVSoms.at(aFace.IndKthSom((aK3+1)%3));  // its sucessor

	    std::vector<int>  aVInd{aS1.NumX(),aS1.NumY(),aS2.NumX(),aS2.NumY()};  // Ind Uk  x1,y1,x2,y2
	    std::vector<tCoordDevTri>  aVObs{Norm2(aS1.Pt3()-aS2.Pt3())};          // we force dist-dev=dist-3D
	    mSys->CalcAndAddObs(mCalcCD,aVInd,aVObs);
        }
    }
    
    // 4 - sove and transfert
     const tDenseV  & aSol = mSys->SolveUpdateReset() ;
     for (auto & aSom : mVSoms)
     {
        if (aSom.IsReached())
        {
            // StdOut() << "HHH " << aSom.NumX() << " " << aSom.NumY() << " " <<  cPt2dr(aSol(aSom.NumX()),aSol(aSom.NumY())) << "\n";
            aSom.SetPt2(cPt2dr(aSol(aSom.NumX()),aSol(aSom.NumY())));
        }
     }
}


void  cDevTriangu3d::ExportDev(const std::string &aName) const
{
     std::vector<tPt3D>  aVPlan3;
     for (const auto & aSom :  mVSoms)
        aVPlan3.push_back(TP3z0(aSom.Pt2()));

     tTriangulation3D aTriPlane(aVPlan3,mTri.VFaces());
     aTriPlane.WriteFile(aName,true);
}

tCoordDevTri cDevTriangu3d::GlobDistortiontDist() const
{
    tCoordDevTri aSomDiff=0,aSomDist=0;
    for (const auto & aKF : mVReachedFaces)
    {
        const cFaceDevT3D& aFace = mVFaces.at(aKF);
        aFace.DistortionDist(aSomDiff,aSomDist)  ;
    }

    return aSomDiff / aSomDist;
}

void cDevTriangu3d::TestGroundTruth2D(const tTriangulation3D & aDevGT)
{
    std::vector<tPt2D>  aVDev;  // vector of devloped pt2d
    std::vector<tPt2D>  aVGT;   // vector of ground truth pt2d

    MMVII_INTERNAL_ASSERT_bench(mTri.NbPts()==aDevGT.NbPts(),"Mesh dev");

    // 1 - compute vector of 2d points
    tCoordDevTri aSomDInit=0.0; // som distance before computing rotatio,
    for (size_t aK=0 ; aK<mTri.NbPts() ; aK++)
    {
       aVDev.push_back(mVSoms.at(aK).Pt2());
       aVGT.push_back(Proj(aDevGT.KthPts(aK)));
       aSomDInit += Norm2(aVDev.back()-aVGT.back());
    }
    // StdOut() << "SOMD0 " << aSomDInit  << "\n";

    // 2 - compute best rotation for maping of vector to the other
    tCoordDevTri aDistAdjusted=1e6;
    cRot2D<tCoordDevTri>  aRot = cRot2D<tCoordDevTri>::StdGlobEstimate(aVDev,aVGT,&aDistAdjusted); 

    // 3 - Now check   R(Dev) = GT
    for (size_t aK=0 ; aK<mTri.NbPts() ; aK++)
    {
       tCoordDevTri aD = Norm2(aRot.Value(aVDev[aK])-aVGT[aK]);
       MMVII_INTERNAL_ASSERT_bench(aD<1e-10,"Mesh dev");
    }
}	

/* ******************************************************* */
/*                                                         */
/*                    cAppliMeshDev                        */
/*                                                         */
/* ******************************************************* */


/**  A basic application for clipping 3d data ,  almost all the job is done in
 * libraries so it essentially interface to command line */

class cAppliMeshDev : public cMMVII_Appli
{
     public :

        cAppliMeshDev(const std::vector<std::string> & aVArgs,const cSpecMMVII_Appli & aSpec);

     private :
        int Exe() override;
        cCollecSpecArg2007 & ArgObl(cCollecSpecArg2007 & anArgObl) override ;
        cCollecSpecArg2007 & ArgOpt(cCollecSpecArg2007 & anArgOpt) override ;

           // --- Mandatory ----
        std::string mNameCloudIn;
           // --- Optionnal ----
        std::string mNameCloudOut;
        bool        mBinOut;
        double      mFactRand;
           // --- Internal ----

};

cAppliMeshDev::cAppliMeshDev(const std::vector<std::string> & aVArgs,const cSpecMMVII_Appli & aSpec) :
   cMMVII_Appli     (aVArgs,aSpec),
   mFactRand        (0.0)
{
}


cCollecSpecArg2007 & cAppliMeshDev::ArgObl(cCollecSpecArg2007 & anArgObl) 
{
 return anArgObl
	  <<   Arg2007(mNameCloudIn,"Name of input cloud/mesh", {eTA2007::FileDirProj,eTA2007::FileCloud})
   ;
}

cCollecSpecArg2007 & cAppliMeshDev::ArgOpt(cCollecSpecArg2007 & anArgOpt)
{
   return anArgOpt
           << AOpt2007(mNameCloudOut,CurOP_Out,"Name of output file")
           << AOpt2007(mBinOut,CurOP_OutBin,"Generate out in binary format",{eTA2007::HDV})
           << AOpt2007(mFactRand,"FactRand","Factor of randomization (for bench)",{eTA2007::HDV,eTA2007::Tuning})
   ;
}


int  cAppliMeshDev::Exe()
{
   InitOutFromIn(mNameCloudOut,"Dev_"+mNameCloudIn);

   tTriangulation3D  aTri(mNameCloudIn);
   cDevTriangu3d aDev(aTri,mFactRand);
   aDev.ExportDev(mNameCloudOut);


   return EXIT_SUCCESS;
}



/* =============================================== */
/*                                                 */
/*                       ::                        */
/*                                                 */
/* =============================================== */

tMMVII_UnikPApli Alloc_MeshDev(const std::vector<std::string> &  aVArgs,const cSpecMMVII_Appli & aSpec)
{
   return tMMVII_UnikPApli(new cAppliMeshDev(aVArgs,aSpec));
}
cSpecMMVII_Appli  TheSpecMeshDev
(
     "MeshDev",
      Alloc_MeshDev,
      "Clip a point cloud/mesh  using a region",
      {eApF::Cloud},
      {eApDT::Ply},
      {eApDT::Ply},
      __FILE__
);

void BenchMeshDev(cParamExeBench & aParam)
{
   if (! aParam.NewBench("MeshDev")) return;

   std::string aDir =  cMMVII_Appli::CurrentAppli().InputDirTestMMVII() + "Ply" + StringDirSeparator();

   for (int aK=0 ; aK< 2 ; aK++)
   {
       tTriangulation3D  aTri3D(aDir+"Cyl3D.ply");
       tTriangulation3D  aTri2D(aDir+"Cyl2D.ply");

       cDevTriangu3d aDev(aTri3D,0.1);
       tCoordDevTri  aDist = aDev.GlobDistortiontDist();
       MMVII_INTERNAL_ASSERT_bench(aDist<1e-10,"Mesh dev");
       aDev.TestGroundTruth2D(aTri2D);
   }

   aParam.EndBench();
}


};
