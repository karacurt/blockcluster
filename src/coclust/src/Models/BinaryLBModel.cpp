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


/** @file BinaryLBModel.cpp
 *  @brief Implements concrete model class BinaryLBModel derived from ICoClustModel.
 **/

#include <limits.h>
#include <math.h>
#include "BinaryLBModel.h"
#ifndef RPACKAGE
using namespace cimg_library;
#endif
BinaryLBModel::BinaryLBModel( MatrixBinary const&  m_Dataij,ModelParameters const& Mparam,int a,int b)
                           : ICoClustModel(Mparam) , m_Dataij_(m_Dataij)
{
  a_ = a;
  b_ = b;
  dimprod_ = nbSample_*nbVar_;
};

BinaryLBModel::BinaryLBModel(MatrixBinary const&  m_Dataij,VectorInteger const & rowlabels,
                             VectorInteger const & collabels,ModelParameters const& Mparam,int a,int b)
                            : ICoClustModel(Mparam,rowlabels,collabels)
                            , a_(a), b_(b), dimprod_(nbSample_*nbVar_)
                            , m_Dataij_(m_Dataij)
{}

bool BinaryLBModel::cemInitStep()
{
#ifdef COVERBOSE
  std::cout<<"Initializing Model Parameters.."<<std::endl;
#endif

  if (initCEMRows())
  {
    if (initCEMCols())
    {
      m_Alphakl1_ = m_Alphakl_;
      m_Alphakl1old_.resize(Mparam_.nbrowclust_,Mparam_.nbcolclust_) = 0;
      m_Alphaklold_.resize(Mparam_.nbrowclust_,Mparam_.nbcolclust_) = 0;
      m_akl_.resize(Mparam_.nbrowclust_,Mparam_.nbcolclust_) = 0;
      m_Uil_.resize(nbSample_,Mparam_.nbcolclust_) = 0;
      m_Vjk_.resize(nbVar_,Mparam_.nbrowclust_) = 0;
      v_logPiek_ = std::log(1.0/Mparam_.nbrowclust_)*(STK::Const::Vector<STK::Real>(Mparam_.nbrowclust_));
      v_logRhol_ = std::log(1.0/Mparam_.nbcolclust_)*(STK::Const::Vector<STK::Real>(Mparam_.nbcolclust_));

#ifdef COVERBOSE
  std::cout<<"Initialization over."<<std::endl;
#endif
      return true;
    }
  }
  return false;

}

void BinaryLBModel::finalizeOutput()
{
  commonFinalizeOutput();
  // Calculate summary Matrix (Class mean)
  for ( int k = 0; k < Mparam_.nbrowclust_; ++k)
  {
    for ( int l = 0; l < Mparam_.nbcolclust_; ++l)
    {
      m_akl_(k,l) =  (m_Alphakl_(k,l)>=.5) ? 1 : 0;
    }
  }
  // Calculate probability for Summary Matrix
  m_epsilonkl_ = m_akl_.cast<STK::Real>().prod(1-m_Alphakl_) + (1-m_akl_.cast<STK::Real>()).prod(m_Alphakl_);

}

void BinaryLBModel::consoleOut()
{
#ifdef COVERBOSE
  std::cout <<"Output Model parameter:"<<"\nakl:\n"<<m_akl_
            <<"\nepsilonkl:\n"<<m_epsilonkl_
            <<"\npiek: "<< v_Piek_.transpose()
            <<"\nRhol: "<<v_Rhol_.transpose()<<"\n";
  std::cout<<"ICL: "<<iclCriteriaValue()<<"\n";
#endif
}


//Compute Bernoulli log-sum for all rows
void BinaryLBModel::logSumRows(MatrixReal & m_sum)
{
  m_sum = m_Uil_*(((((m_Alphakl_+RealMin)/((1-m_Alphakl_)+RealMin)).log())).transpose())
        + STK::Const::Vector<STK::Real>(nbSample_)*((v_logPiek_+((1-m_Alphakl_+RealMin).log())*v_Rl_).transpose());
}

//Compute Bernoulli log-sum for all columns
void BinaryLBModel::logSumCols(MatrixReal & m_sum)
{
  m_sum = m_Vjk_*( ((m_Alphakl_+RealMin)/(1-m_Alphakl_+RealMin)).log())
        + STK::Const::VectorX(nbVar_)
          *( (v_logRhol_+(( (1-m_Alphakl_)+RealMin).log()).transpose()*v_Tk_).transpose());
}

//Run EM algorithm on data Matrix m_Uil_
bool BinaryLBModel::emRows()
{
  //Initializations
  m_Uil_=m_Dataij_.cast<STK::Real>()*m_Rjl_;
  for ( int itr = 0; itr < Mparam_.nbiterations_int_; ++itr)
  {
    //E-step
    if(!eStepRows()) return false;
    //M-step
    m_Alphaklold_ = m_Alphakl_;
    mStepRows();
    //Termination check
    if((((m_Alphakl_-m_Alphaklold_).abs()/(m_Alphakl_+RealMin)).sum())<Mparam_.epsilon_int_)
    { break;}
  }
  return true;
}


//Run CEM algorithm on data Matrix m_Uil_
bool BinaryLBModel::cemRows()
{
  //Initializations
  m_Uil_=m_Dataij_.cast<STK::Real>()*m_Rjl_;

  for ( int itr = 0; itr < Mparam_.nbiterations_int_; ++itr)
  {
    //CE-step
    if(!ceStepRows()) return false;
    //M-step
    m_Alphaklold_ = m_Alphakl_;
    mStepRows();
    //Termination check
    if((((m_Alphakl_-m_Alphaklold_).abs()/(m_Alphakl_+RealMin)).sum())<Mparam_.epsilon_int_)
    { break;}
  }
  return true;
}

// Run EM algorithm on data matrix m_Vjk_
bool BinaryLBModel::emCols()
{
  //Initializations
  m_Vjk_=m_Dataij_.cast<STK::Real>().transpose()*m_Tik_;
  for ( int itr = 0; itr < Mparam_.nbiterations_int_; ++itr)
  {
    if(!eStepCols()) return false;
     //M-step
    m_Alphaklold_ = m_Alphakl_;
    mStepCols();
    //Termination check
    if((((m_Alphakl_-m_Alphaklold_).abs()/(m_Alphakl_+RealMin)).sum())<Mparam_.epsilon_int_)
    { break;}
  }
  // Update Alpha for outer loop
  m_Alphakl1old_ = m_Alphakl1_;
  m_Alphakl1_ = m_Alphakl_;
  return true;
}

// Run CEM algorithm on data matrix m_Vjk_
bool BinaryLBModel::cemCols()
{
  //Initializations
  m_Vjk_=m_Dataij_.cast<STK::Real>().transpose()*m_Tik_;

  for ( int itr = 0; itr < Mparam_.nbiterations_int_; ++itr)
  {
    //CE-step
    if(!ceStepCols()) return false;
    //M-step
    m_Alphaklold_ = m_Alphakl_;
    mStepCols();
    //Termination check
    if((((m_Alphakl_-m_Alphaklold_).abs()/(m_Alphakl_+RealMin)).sum())<Mparam_.epsilon_int_)
    { break;}
  }
  // Update Alpha for outer loop
  m_Alphakl1old_ = m_Alphakl1_;
  m_Alphakl1_ = m_Alphakl_;
  return true;
}

bool BinaryLBModel::semRows()
{
  m_Uil_ = m_Dataij_.cast<STK::Real>()*m_Rjl_;

  if(!seStepRows()) return false;

  //M-step : update row proportions and model parameters
  mStepRows();
  return true;
}

bool BinaryLBModel::semCols()
{
  m_Vjk_=m_Dataij_.cast<STK::Real>().transpose()*m_Tik_;

  if(!seStepCols()) return false;

  //M-step: Update column proportions and model parameters
  mStepCols();

  return true;
}

STK::Real BinaryLBModel::estimateLikelihood()
{
	likelihood_ = ( v_Tk_.transpose()*
	                ( m_Alphakl_.prod((m_Alphakl_+RealMin).log())
	                + (1-m_Alphakl_).prod( (1-m_Alphakl_+RealMin).log() )
	                )*v_Rl_
	                + v_Tk_.dot(v_logPiek_) + v_Rl_.dot(v_logRhol_)
	                - (m_Tik_.prod( (RealMin + m_Tik_).log()) ).sum()
	                - (m_Rjl_.prod( (RealMin + m_Rjl_).log()) ).sum()
	              )/dimprod_;
	return likelihood_;
}

//Compute change in Alpha and set the terminate variable accordingly
void BinaryLBModel::parameterStopCriteria()
{
  STK::Real relativechange = (((m_Alphakl1_-m_Alphakl1old_).abs()/(m_Alphakl1_+RealMin)).sum());
  if(relativechange<Mparam_.epsilon_)
    stopAlgo_ = true;
  else
    stopAlgo_ = false;
}

MatrixBinary const& BinaryLBModel::arrangedDataClusters()
{
  arrangedDataCluster(m_ClusterDataij_,m_Dataij_);
  return m_ClusterDataij_;
}

#ifndef RPACKAGE
void BinaryLBModel::displayCluster()
{
  CImg<unsigned char>  cluster(nbVar_,nbSample_,1,1,0);
  CImg<unsigned char>  data(nbVar_,nbSample_,1,1,0);
  MatrixBinary m_ClusterDataij_ = arrangedDataClusters();

  // Assign value to images
  for ( int i = 0; i < nbSample_; ++i)
  {
    for ( int j = 0; j < nbVar_; ++j)
    {
      cluster(j,i) = (m_ClusterDataij_(i,j) == 0) ? (unsigned char)(0)
                                                  : (unsigned char)(255);
      data(j,i) = (m_Dataij_(i,j) == 0) ? (unsigned char)(0)
                                        : (unsigned char)(255);
    }
  }

  //Display data and cluster
  CImgDisplay data_disp(data,"Original Data"), cluster_disp(cluster,"Co-Clusters");
  while (!data_disp.is_closed() && !cluster_disp.is_closed()) {
    data_disp.wait();
    cluster_disp.wait();
  }

  data.save("data.jpg");
  cluster.save("cluster.jpg");
}
#endif

//Compute Bernoulli log-sum for all rows
void BinaryLBModel::initBernoulliLogSumRows(MatrixReal & _m_sum)
{
  //Array2DReal m_Alphakltmp = m_Alphakl_+RealMin;
  _m_sum = m_Uil_* ( (m_Alphakl_+RealMin) / (1-m_Alphakl_+RealMin) ).log().transpose()
         + STK::Const::Vector<STK::Real>(Mparam_.nbrowdata_)
           * ( (1-m_Alphakl_+RealMin).log() * v_Rl_).transpose();
}

//Compute Bernoulli log-sum for all columns
void BinaryLBModel::initBernoulliLogSumCols(MatrixReal & _m_sum)
{
  _m_sum = m_Vjk_*(((((m_Alphakl_+RealMin)/(1-m_Alphakl_+RealMin)).log())))+
      STK::Const::Vector<STK::Real>(Mparam_.nbcoldata_)*(v_Tk_.transpose()*(((1-m_Alphakl_+RealMin).log())));
}

bool BinaryLBModel::initCEMRows()
{
  // Initialization of various parameters
  int cols=std::min(100,int(nbVar_));
  m_Uil_.resize(nbSample_,cols) = 0;
  selectRandomColsFromData(m_Uil_,cols);
  m_Vjk_.resize(nbVar_,Mparam_.nbrowclust_) = 0;
  v_Rl_  = STK::Const::Vector<STK::Real>(cols);
  v_Tk_.resize(Mparam_.nbrowclust_) = 0;
  m_Tik_.resize(nbSample_,Mparam_.nbrowclust_) = 0;
  m_Rjl_.resize(nbVar_,Mparam_.nbcolclust_) = 0;
  m_Alphakl_.resize(Mparam_.nbrowclust_,cols) = 0;
  generateRandomBernoulliParameterRows(m_Alphakl_,cols);
  //Determine row partition using CEM algorithm with equal proportions
  MatrixReal m_sumik(nbSample_,Mparam_.nbrowclust_);
  int maxIndex;
  std::pair<int,int> Label_pair;
  for(int i=0;i<knownLabelsRows_.size();i++)
  {
    Label_pair = knownLabelsRows_[i];
    m_Tik_(Label_pair.first,Label_pair.second) = 1;
  }
  for ( int itr = 0; itr < Mparam_.nbinititerations_; ++itr)
  {
    initBernoulliLogSumRows(m_sumik);
    for ( int i=0; i<UnknownLabelsRows_.size();i++)
    {
      m_sumik.row(UnknownLabelsRows_[i]).maxElt(maxIndex);
      m_Tik_.row(UnknownLabelsRows_[i]).setZeros();
      m_Tik_(UnknownLabelsRows_[i],maxIndex)=1;
    }
    // check empty class
    if( (empty_cluster_ = finalizeStepRows()) )
    {
      Error_msg_  = "In BinaryLBModel::initCEMRows(). Class size too small while estimating model.\n";
#ifdef COVERBOSE
      std::cout << Error_msg_;
#endif
      return false;
    }
    // M-step
    m_Alphaklold_ = m_Alphakl_;
    m_Alphakl_ = (m_Tik_.transpose()*m_Uil_)/(v_Tk_*(v_Rl_.transpose()));

    if((((m_Alphakl_-m_Alphaklold_).abs()/(m_Alphakl_+RealMin)).sum())<Mparam_.initepsilon_)
    { break;}
  }
  return true;
}

bool BinaryLBModel::initCEMCols()
{
  //Determine row partition using CEM algorithm with equal proportions
  MatrixReal m_sumjl(nbVar_, Mparam_.nbcolclust_);
  int maxIndex;
  m_Vjk_=m_Dataij_.cast<STK::Real>().transpose()*m_Tik_;
  m_Alphakl_.resize(Mparam_.nbrowclust_,Mparam_.nbcolclust_);
  v_Rl_.resize(Mparam_.nbcolclust_);
  generateRandomBernoulliParameterCols(m_Alphakl_);
  std::pair<int,int> Label_pair;
  for ( int j=0;j<knownLabelsCols_.size();j++)
  {
    Label_pair = knownLabelsCols_[j];
    m_Rjl_(Label_pair.first,Label_pair.second)=1;
  }
  for ( int itr = 0; itr < Mparam_.nbinititerations_; ++itr)
  {
    // CE-step
    initBernoulliLogSumCols(m_sumjl);
    for ( int j=0;j< UnknownLabelsCols_.size();j++)
    {
      m_sumjl.row(UnknownLabelsCols_[j]).maxElt(maxIndex);
      m_Rjl_.row(UnknownLabelsCols_[j]).setZeros();
      m_Rjl_(UnknownLabelsCols_[j],maxIndex)=1;
    }
    // check empty class
    if( (empty_cluster_ = finalizeStepCols()) )
    {
      Error_msg_  = "In BinaryLBModel::initCEMCols(). Class size too small while estimating model.\n";
#ifdef COVERBOSE
      std::cout << Error_msg_;
#endif
      return false;
    }
    // M-step
    m_Alphaklold_ = m_Alphakl_;
    m_Alphakl_ = (m_Vjk_.transpose()*m_Rjl_)/ (v_Tk_*(v_Rl_.transpose()));
    if((((m_Alphakl_-m_Alphaklold_).abs()/(m_Alphakl_+RealMin)).sum())<Mparam_.initepsilon_)
    { break;}
  }
  return true;
}

void BinaryLBModel::selectRandomColsFromData(MatrixReal& _m_il,int cols)
{
  if(cols==Mparam_.nbcoldata_)
  { _m_il=m_Dataij_.cast<STK::Real>();}
  else
  {
    //random shuffle Algorithm
    VectorInteger _v_temp = randSample(nbVar_,cols);
    for ( int l = 0; l < cols; ++l)
    { _m_il.col(l)=m_Dataij_.cast<STK::Real>().col(_v_temp[l]);}
  }
}

void BinaryLBModel::generateRandomBernoulliParameterRows(MatrixReal& _m_kl,int cols)
{
  int index;
  STK::Real epsilon = 0.1;
  VectorInteger _v_temp = randSample(nbSample_,Mparam_.nbrowclust_);
  for ( int k = 0; k < Mparam_.nbrowclust_; ++k)
  {
    index=_v_temp[k];
    //index=k;
    for ( int l = 0; l < cols; ++l)
    { _m_kl(k,l)=epsilon*(1.0-m_Uil_(index,l))+m_Uil_(index,l)*(1.0-epsilon);}
  }
}

void BinaryLBModel::generateRandomBernoulliParameterCols(MatrixReal& _m_kl)
{
  int index;
  VectorInteger _v_temp = randSample(nbVar_,Mparam_.nbcolclust_);
  for ( int l = 0; l < Mparam_.nbcolclust_; ++l)
  {
    index=_v_temp[l];
    //index=l;
    for ( int k = 0; k < Mparam_.nbrowclust_; ++k)
    { _m_kl(k,l)=m_Vjk_(index,k)/v_Tk_[k];}
  }
}

void BinaryLBModel::modifyThetaStart()
{
  m_Alphaklstart_ = m_Alphakl_;
  m_epsilonklstart_ = m_epsilonkl_;
  v_logPiekstart_ = v_logPiek_;
  v_logRholstart_ = v_logRhol_;
  m_Rjlstart_ = m_Rjl_;
  m_Tikstart_ = m_Tik_;
}

void BinaryLBModel::copyThetaStart()
{
  m_Alphakl_ = m_Alphaklstart_;
  m_epsilonkl_ = m_epsilonklstart_;
  v_logPiek_ = v_logPiekstart_;
  v_logRhol_ = v_logRholstart_;
  m_Rjl_ = m_Rjlstart_;
  m_Tik_ = m_Tikstart_;

  //initialization
  v_Rl_ = STK::sum(m_Rjl_);
  m_Alphakl1_ = m_Alphakl_;
}

void BinaryLBModel::copyThetaMax()
{
  m_Alphakl_ = m_Alphaklmax_;
  m_epsilonkl_ = m_epsilonklmax_;
  v_logPiek_ = v_logPiekmax_;
  v_logRhol_ = v_logRholmax_;
  m_Tik_ = m_Tikmax_;
  m_Rjl_ = m_Rjlmax_;
  likelihood_ = Lmax_;
}

void BinaryLBModel::modifyThetaMax()
{
  m_Alphaklmax_ = m_Alphakl_;
  m_epsilonklmax_ = m_epsilonkl_;
  v_logPiekmax_ = v_logPiek_;
  v_logRholmax_ = v_logRhol_;
  m_Rjlmax_ = m_Rjl_;
  m_Tikmax_ = m_Tik_;
  Lmax_ = likelihood_;
}


void BinaryLBModel::mStepFull()
{
  if(!Mparam_.fixedproportions_)
  {
    v_logPiek_=(v_Tk_/nbSample_).log();
    v_logRhol_=(v_Rl_/nbVar_).log();
  }
  m_Alphakl_ = (m_Tik_.transpose()*m_Dataij_.cast<STK::Real>()*m_Rjl_)/(v_Tk_*v_Rl_.transpose());
}


void BinaryLBModel::mStepRows()
{
  if(!Mparam_.fixedproportions_)
  { v_logPiek_=((v_Tk_+a_-1)/(nbSample_+Mparam_.nbrowclust_*(a_-1))).log();}

  m_Alphakl_ = (((m_Tik_.transpose())*m_Uil_)+b_-1)/((v_Tk_*(v_Rl_.transpose()))+2*(b_-1));
  m_Alphakl_ = m_Alphakl_.max(0.).min(1.);
}

void BinaryLBModel::mStepCols()
{
  if(!Mparam_.fixedproportions_)
  { v_logRhol_=((v_Rl_+a_-1)/(nbVar_+Mparam_.nbcolclust_*(a_-1))).log();}

  m_Alphakl_ = ((m_Vjk_.transpose()*m_Rjl_)+b_-1)/((v_Tk_*v_Rl_.transpose())+2*(b_-1));
  m_Alphakl_ = m_Alphakl_.max(0.).min(1.);
}


STK::Real BinaryLBModel::iclCriteriaValue(){
  STK::Real criteria = 0.0;

  criteria+= lgamma(Mparam_.nbrowclust_*a_)+lgamma(Mparam_.nbcolclust_*a_)
      -(Mparam_.nbrowclust_+Mparam_.nbcolclust_)*lgamma(a_)
      +Mparam_.nbrowclust_*Mparam_.nbcolclust_*(lgamma(2*b_)-2*lgamma(b_))
      -lgamma(Mparam_.nbrowdata_+Mparam_.nbrowclust_*a_)
      -lgamma(Mparam_.nbcoldata_+Mparam_.nbcolclust_*a_);

  for (int k = 0; k < Mparam_.nbrowclust_; ++k) {
    criteria+= lgamma(a_+ (v_Zi_== k).count());
  }

  for (int l = 0; l < Mparam_.nbcolclust_; ++l) {
    criteria+= lgamma(a_+ (v_Wj_==l).count());
  }

  STK::ArrayXXi temp0(Mparam_.nbrowclust_,Mparam_.nbcolclust_);
  STK::ArrayXXi temp1(Mparam_.nbrowclust_,Mparam_.nbcolclust_);
  MatrixBinary m_tempdata = (m_Dataij_==0);
  temp0 = (m_Zik_.transpose()*m_tempdata.cast<int>()*m_Wjl_)+b_;
  temp1 = (m_Zik_.transpose()*m_Dataij_.cast<int>()*m_Wjl_)+b_;
  for (int k = 0; k < Mparam_.nbrowclust_; ++k)
  {
    for (int l = 0; l < Mparam_.nbcolclust_; ++l)
    { criteria+=lgamma(temp0(k,l))+lgamma(temp1(k,l));}
  }

  for (int k = 0; k < Mparam_.nbrowclust_; ++k)
  {
    for (int l = 0; l < Mparam_.nbcolclust_; ++l)
    { criteria-= lgamma(((v_Zi_== k).count())*((v_Wj_==l).count())+2*b_);}
  }

  return criteria;
}
