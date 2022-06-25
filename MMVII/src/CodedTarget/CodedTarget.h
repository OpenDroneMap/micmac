#ifndef _CODED_TARGET_H_
#define _CODED_TARGET_H_
#include "include/MMVII_all.h"
#include "include/MMVII_SetITpl.h"


namespace MMVII
{
template<class TypeEl>
   double IndBinarity(const  cDataIm2D<TypeEl> & aDIm,const cPt2di & aP0,const std::vector<cPt2di> & aVectVois);

template<class TypeEl> cIm2D<TypeEl> ImBinarity(const  cDataIm2D<TypeEl> & aDIm,double aR0,double aR1,double Epsilon);

std::vector<cPt2dr> VecDir(const  std::vector<cPt2di>&  aVectVois);
template<class TypeEl> double Starity
                              (
                                  const  cImGrad<TypeEl> & aImGrad,
                                  const cPt2dr & aP0,
                                  const  std::vector<cPt2di>&  aVectVois ,
                                  const  std::vector<cPt2dr>&  aVecDir,
                                  double Epsilon
                              );


template<class TypeEl> cIm2D<TypeEl> ImStarity(const  cImGrad<TypeEl> & aImGrad,double aR0,double aR1,double Epsilon);



template<class TypeEl> cIm2D<TypeEl> ImSymetricity(cIm2D<TypeEl> anImIn,double aR0,double aR1,double Epsilon);




namespace  cNS_CodedTarget
{

typedef cSetISingleFixed<tU_INT4>  tBinCodeTarg;
typedef std::vector<tBinCodeTarg> tVSetICT;
typedef cIm2D<tU_INT1>     tImTarget;
typedef cDataIm2D<tU_INT1> tDataImT;


/*  *********************************************************** */
/*                                                              */
/*                      cParamCodedTarget                       */
/*                                                              */
/*  *********************************************************** */

class cSetCodeOf1Circle
{
    public :
      cSetCodeOf1Circle(const std::vector<int> & aCards,int aN);
      int  NbSub() const;
      const tBinCodeTarg & CodeOfNum(int aNum) const;
      int N() const;
    private :
      std::vector<int>   mVCards;
      int      mN;
      tVSetICT mVSet ;  //   All the binary code of one target 
};


class cCodesOf1Target
{
   public :
      cCodesOf1Target(int aNum);

      void AddOneCode(const tBinCodeTarg &);
      void  Show();
      const tBinCodeTarg & CodeOfNumC(int) const;
      int   Num() const;
   private :
      int                        mNum;
      std::vector<tBinCodeTarg>  mCodes;
};




class cParamCodedTarget
{
    public :
       cParamCodedTarget();
       void InitFromFile(const std::string & aNameFile);

       int &     NbRedond();  // Redundancy = number of repetition of a pattern in a circle
       int &     NbCircle();  // Redundancy = number of repetition of a pattern in a circle
       double &  RatioBar();  // Ratio on codin bar
       void      Finish();

       int NbCodeAvalaible() const;         // Number of different code we can generate
       int BaseForNum() const;         // Base used for converting integer to string
       cCodesOf1Target CodesOfNum(int);     // One combinaison of binary code
       tImTarget  MakeIm(const cCodesOf1Target &);  // Generate the image of 1 combinaison
       tImTarget  MakeImCodeExt(const cCodesOf1Target &);  // Generate the image of 1 combinaison

       void AddData(const cAuxAr2007 & anAux);

       bool CodeBinOfPts(double aRho,double aTeta,const cCodesOf1Target & aSetCodesOfT,double aRho0,double aThRho);



    private :

       cPt2dr    Pix2Norm(const cPt2di &) const;
       cPt2dr    Norm2PixR(const cPt2dr &) const;
       cPt2di    Norm2PixI(const cPt2dr &) const;

       bool      mCodeExt;   // if true the code is not at the center but code bar at periphery
       int       mNbRedond;  // Redundancy = number of repetition of a pattern in a circle
       int       mNbCircle;  // Number of circles encoding information

       double    mThTargetC;  // Thickness of central target
       double    mThStars;  //   Thickness of "star" pattern
       double    mThBlCircExt;  //   Thickness of External Black circle
       double    mThBrdWhiteInt;    // Thickness of white internal border
       double    mThBrdBlack;    // Thickness of black border
       double    mThBrdWhiteExt;    // Thickness of black border
       // double    mThBrdWhiteExt;    // Thickness of black border
       double    mThTxt;            // Thickness of text

       double    mScaleTopo;         // Scale used to create identifiable center 4 toto
       int       mNbPixelBin;        // Number of pixel  Binary image

       std::vector<double> mTetasQ;  // Tetas of first quarter

       double    mThRing ;      // Thickness of each ring of star : mThStars/mNbCircle

       double    mRhoEndTargetC;  // Rho when central targe ends
       double    mRhoEndStar;      // Rho when ends stars pattern
       double    mRhoEndBlackCircle;      // Rho when ends external black circle
       double    mRhoEnBrdWhiteInt;   // Rho where ends interior white border
       double    mRhoEndBrdBlack;   // Rho where ends black border
       double    mRhoEndBrdWhiteExt;   // Rho where ends white border
       double    mRhoEndTxt;   // Rho where ends text


       cPt2di    mSzBin;
       cPt2dr    mMidle;
       double    mScale;  // Sz of Pixel in normal coord

       std::vector<cSetCodeOf1Circle>     mVecSetOfCode;
       cDecomposPAdikVar                  mDecP;
};

void AddData(const  cAuxAr2007 & anAux,cParamCodedTarget & aPCT);


};
};
#endif // _CODED_TARGET_H_

