/*--------------------------------------------------------------------*/
/*     Copyright (C) 2011-2012  Parmeet Singh Bhatia

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

/*
 * Project:  cocluster
 * created on: Jan 10, 2012
 * Author: Parmeet Singh Bhatia
 *
 **/

/** @file Rcoclustermain.cpp
 *  @brief 
 **/

#include <Rcpp.h>
#include <time.h>
#include <exception>
#include <map>
//CoClust::library
#include "coclust/src/CoClustFacade/CoCluster.h"
//CoClust::Enumeration
#include "coclust/src/enumerations/enumerations.h"
//Package::DataExchangeFiles
#include "IDataExchange.h"
#include "ContinuousDataExchange.h"
#include "BinaryDataExchange.h"
#include "ContingencyDataExchange.h"
#include "CategoricalDataExchange.h"



#ifdef SUPPORT_OPENMP
#include <omp.h>

RcppExport SEXP CoClustmain(SEXP robj)
{
  BEGIN_RCPP
  //Initialize Rcpp object
  Rcpp::S4 CoClustobj(robj);

  //set number of threads equal to number of processors
  omp_set_num_threads(omp_get_num_procs());

  std::map<std::string,DataType> S_DataType;
  S_DataType["binary"] = Binary;
  S_DataType["contingency"] = Contingency;
  S_DataType["continuous"] = Continuous;
  S_DataType["categorical"] = Categorical;
  DataType datatype = S_DataType[Rcpp::as<std::string>(CoClustobj.slot("datatype"))];
  //Various pointers declarations
  IStrategy * p_Strategy_ = NULL;
  IAlgo * p_Algo_ = NULL;
  ICoClustModel * p_Model_ = NULL;
  ICoClustModel * p_FinalModel_ = NULL;
  IInit * p_Init_ = NULL;
  CoCluster* p_CoCluster_ = NULL;
  IDataExchange * p_DataExchange_;

  //set p_DataExchange_ to required case
  switch (datatype) {
    case Binary:
      p_DataExchange_ = new BinaryDataExchange();
      break;
    case Contingency:
      p_DataExchange_ = new ContingencyDataExchange();
      break;
    case Continuous:
      p_DataExchange_ = new ContinuousDataExchange();
      break;
    case Categorical:
      p_DataExchange_ = new CategoricalDataExchange();
      break;
    default:
      break;
  }

  //Get input parameters
  p_DataExchange_->SetInput(CoClustobj);
  //Get data
  p_DataExchange_->DataInput(CoClustobj);
  //instantiate model
  p_DataExchange_->instantiateModel(p_FinalModel_);
  // Set Aparam_ for parallel use
  StrategyParameters Sparam_ = p_DataExchange_->GetStrategyParameters();
  int nbtry = Sparam_.nbtry_;
  Sparam_.nbtry_ = 1;

  //use to update p_FinalModel_ in case the thread model is better then it.
  float Lmax = -RealMax;
  //to measure global success
  bool globalsuccess = false;
  //start measuring time after exchange of data and various initializations
  //double starttime=omp_get_wtime();
#pragma omp parallel shared(Lmax,globalsuccess,Sparam_,nbtry) private(p_Model_,p_CoCluster_,p_Algo_,p_Init_,p_Strategy_)
    {
    //instantiate algorithm and strategy
    p_DataExchange_->instantiateAlgo(p_Algo_,p_Strategy_);
    //instantiate initialization
    p_DataExchange_->instantiateInit(p_Init_);
        // get copy of model for each thread
        p_Model_ = p_FinalModel_->Clone();

        //set cocluster
        p_CoCluster_ = new CoCluster();
        p_CoCluster_->SetStrategy(p_Strategy_);
        p_CoCluster_->SetAlgo(p_Algo_);
        p_CoCluster_->SetModel(p_Model_);
        p_CoCluster_->SetInit(p_Init_);

#pragma omp for schedule(dynamic,1)

      for (int i = 0; i < nbtry; ++i) {
        bool success = p_CoCluster_->run();
#pragma omp critical
        {
          if (Lmax<p_Model_->GetLikelihood()&& success) {
            Lmax = p_Model_->GetLikelihood();
            globalsuccess = true;
            delete p_FinalModel_;
            p_FinalModel_ = p_Model_->Clone();
          }
        }
      }

      delete p_Model_;
      delete p_Algo_;
      delete p_Init_;
      delete p_CoCluster_;

      }

  p_DataExchange_->Output(CoClustobj,p_FinalModel_,globalsuccess);
  //CoClustobj.slot("time") = omp_get_wtime()-starttime;

  //release memory
  delete p_DataExchange_;
  delete p_FinalModel_;

  return CoClustobj;
  END_RCPP
}
#else
RcppExport SEXP CoClustmain(SEXP robj)
{
  BEGIN_RCPP
  //Initialize Rcpp object
  Rcpp::S4 CoClustobj(robj);

  std::map<std::string,DataType> S_DataType;
  S_DataType["binary"] = Binary;
  S_DataType["contingency"] = Contingency;
  S_DataType["continuous"] = Continuous;
  S_DataType["categorical"] = Categorical;

  DataType datatype = S_DataType[Rcpp::as<std::string>(CoClustobj.slot("datatype"))];

  //Various pointers declarations
  IStrategy * p_Strategy_ = NULL;
  IAlgo * p_Algo_ = NULL;
  ICoClustModel * p_Model_ = NULL;
  IInit * p_Init_ = NULL;
  CoCluster* p_CoCluster_ = NULL;
  IDataExchange * p_DataExchange_ = NULL;

  //set p_DataExchange_ to required case
  switch (datatype) {
    case Binary:
      p_DataExchange_ = new BinaryDataExchange();
      break;
    case Contingency:
      p_DataExchange_ = new ContingencyDataExchange();
      break;
    case Continuous:
      p_DataExchange_ = new ContinuousDataExchange();
      break;
    case Categorical:
      p_DataExchange_ = new CategoricalDataExchange();
      break;
    default:
      break;
  }

  //Get input parameters
  p_DataExchange_->SetInput(CoClustobj);
  //Get data
  p_DataExchange_->DataInput(CoClustobj);
  //instantiate algorithm and strategy
  p_DataExchange_->instantiateAlgo(p_Algo_,p_Strategy_);
  //instantiate initialization
  p_DataExchange_->instantiateInit(p_Init_);
  //instantiate model
  p_DataExchange_->instantiateModel(p_Model_);

  //create cocluster object and run coclustering
  p_CoCluster_ = new CoCluster();
  p_CoCluster_->SetStrategy(p_Strategy_);
  p_CoCluster_->SetAlgo(p_Algo_);
  p_CoCluster_->SetModel(p_Model_);
  p_CoCluster_->SetInit(p_Init_);
  bool success = p_CoCluster_->run();
  p_DataExchange_->Output(CoClustobj,p_Model_,success);

  //release memory
  delete p_DataExchange_;
  delete p_Init_;
  delete p_Algo_;
  delete p_Model_;
  delete p_CoCluster_;

  return CoClustobj;
  END_RCPP
}
#endif