/*--------------------------------------------------------------------*/
/*     Copyright (C) 2011-2015  <MODAL team @INRIA,Lille & U.M.R. C.N.R.S. 6599 Heudiasyc, UTC>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as
 published by the Free Software Foundation; either version 2 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public
 License along with this program; if not, write to the
 Free Software Foundation, Inc.,
 59 Temple Place,
 Suite 330,
 Boston, MA 02111-1307
 USA

 Contact : parmeet.bhatia@inria.fr , bhatia.parmeet@gmail.com
 */


/** @file ContinuousLBModelequalsigma.h
 *  @brief Declares concrete model class ContinuousLBModelequalsigma derived from ICoClustModel.
 **/

#ifndef CONTINUOUSLBMODELEQUALSIGMA_H_
#define CONTINUOUSLBMODELEQUALSIGMA_H_

/** @brief Concrete model class for continuous data.
 * This class does presume equal variance among various  co-clusters.
 */
#include "ICoClustModel.h"

class ContinuousLBModelequalsigma: public ICoClustModel
{
  public:
    ContinuousLBModelequalsigma( MatrixReal const& m_Dataij,ModelParameters const& Mparam);
    ContinuousLBModelequalsigma(MatrixReal const& m_Dataij,VectorInteger const & rowlabels,
                       VectorInteger const & collabels,ModelParameters const& Mparam);
    /** cloning */
    virtual ContinuousLBModelequalsigma* clone(){return new ContinuousLBModelequalsigma(*this);}
    virtual void logSumRows(MatrixReal & _m_sum);
    virtual void logSumCols(MatrixReal & _m_sum);
    virtual void mStepFull();
    virtual bool emRows();
    virtual bool cemRows();
    virtual bool emCols();
    virtual bool cemCols();
    virtual bool semRows();
    virtual bool semCols();
    virtual void parameterStopCriteria();
    virtual STK::Real estimateLikelihood();
    virtual bool cemInitStep();
    virtual void finalizeOutput();
    virtual void consoleOut();
    virtual void modifyThetaStart();
    virtual void copyThetaStart();
    virtual void copyThetaMax();
    virtual void modifyThetaMax();
    MatrixReal const& arrangedDataClusters();
    virtual ~ContinuousLBModelequalsigma(){};

    /**Return means ContinuousLBModelequalsigma::m_Mukl_*/
    MatrixReal const& mean() const;
    /**Return Sigma ContinuousLBModelequalsigma::m_Sigma2kl_*/
    STK::Real sigma2() const;
  protected:
    MatrixReal const& m_Dataij_;
    MatrixReal m_ClusterDataij_;
    STK::Real dimprod_;
    MatrixReal m_Dataij2_;
    MatrixReal m_Mukl_; //,m_Mukl2_;
    STK::Real Sigma2_,Sigma2start_,Sigma2max_;
    MatrixReal m_Muklold1_,m_Muklold2_,m_Muklstart_,m_Muklmax_;
    MatrixReal m_Vjk1_,m_Vjk2_;
    MatrixReal m_Uil1_,m_Uil2_;

    //M-steps
    void mStepRows();
    void mStepCols();

    // Functions used to operate on data in intermediate steps when running the Initialization
    bool initCEMCols();
    void selectRandomRowsFromData(MatrixReal &);
    void generateRandomMean(const MatrixReal & , MatrixReal &);
};

inline MatrixReal const& ContinuousLBModelequalsigma::mean() const
{ return m_Mukl_;}

inline STK::Real ContinuousLBModelequalsigma::sigma2() const
{ return Sigma2_;}

inline void ContinuousLBModelequalsigma::mStepRows()
{
  if(!Mparam_.fixedproportions_) { v_logPiek_=(v_Tk_/nbSample_).log();}

  m_Mukl_  = (m_Tik_.transpose()*m_Uil1_)/(v_Tk_*v_Rl_.transpose());
  //m_Mukl2_ = m_Mukl_.square();
  Sigma2_  = ((m_Tik_.transpose()*m_Uil2_).sum()-v_Tk_.dot(m_Mukl_.square()*v_Rl_))/dimprod_;
}

inline void ContinuousLBModelequalsigma::mStepCols()
{
  if(!Mparam_.fixedproportions_) { v_logRhol_=(v_Rl_/nbVar_).log();}

  m_Mukl_  = (m_Vjk1_.transpose()*m_Rjl_)/(v_Tk_*v_Rl_.transpose());
//  m_Mukl2_ = m_Mukl_.square();
  Sigma2_  = ((m_Vjk2_.transpose()*m_Rjl_).sum()-v_Tk_.dot(m_Mukl_.square()*v_Rl_))/dimprod_;
}
#endif /* CONTINUOUSLBMODELEQUALSIGMA_H_ */
