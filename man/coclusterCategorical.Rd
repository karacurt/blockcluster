% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/coclusterCategorical.R
\name{coclusterCategorical}
\alias{coclusterCategorical}
\title{Co-Clustering function for categorical data-sets.}
\usage{
coclusterCategorical(data, semisupervised = FALSE,
  rowlabels = integer(0), collabels = integer(0), model = NULL,
  nbcocluster, strategy = coclusterStrategy(), a = 1, b = 1,
  nbCore = 1)
}
\arguments{
\item{data}{Input data as matrix (or list containing data matrix.)}

\item{semisupervised}{Boolean value specifying whether to perform semi-supervised co-clustering or not. Make sure to provide row and/or
column labels if specified value is true. The default value is false.}

\item{rowlabels}{Integer Vector specifying the class of rows. The class number starts from zero.
Provide -1 for unknown row class.}

\item{collabels}{Integer Vector specifying the class of columns.
The class number starts from zero. Provide -1 for unknown column class.}

\item{model}{This is the name of model. The following models exists for categorical data:
\tabular{rlll}{
    pik_rhol_multi \tab categorical \tab unequal \tab unequal \cr
    pi_rho_multi  \tab categorical \tab equal \tab unequal \cr
}}

\item{nbcocluster}{Integer vector specifying the number of row and column clusters respectively.}

\item{strategy}{Object of class \code{\linkS4class{strategy}}.}

\item{a}{First hyper-parameter in case of Bayesian settings. Default is 1 (no prior).}

\item{b}{Second hyper-parameter in case of Bayesian settings. Default is 1 (no prior).}

\item{nbCore}{number of thread to use (OpenMP must be available), 0 for all cores. Default value is 1.}
}
\value{
Return an object of \code{\linkS4class{BinaryOptions}} or \code{\linkS4class{ContingencyOptions}}
or \code{\linkS4class{ContinuousOptions}} depending on whether the data-type is Binary, Contingency or Continuous
respectively.
}
\description{
This function performs Co-Clustering (simultaneous clustering of rows and columns )
Categorical data-sets using latent block models. It can also be used to perform
semi-supervised co-clustering.
}
\examples{

## Simple example with simulated categorical data
## load data
data(categoricaldata)
## usage of coclusterCategorical function in its most simplest form
out<-coclusterCategorical(categoricaldata,nbcocluster=c(3,2))
## Summarize the output results
summary(out)
## Plot the original and Co-clustered data 
plot(out)

}
