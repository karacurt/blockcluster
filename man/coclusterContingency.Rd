% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/coclusterContingency.R
\name{coclusterContingency}
\alias{coclusterContingency}
\title{Co-Clustering function.}
\usage{
coclusterContingency(data, semisupervised = FALSE,
  rowlabels = integer(0), collabels = integer(0), model = NULL,
  nbcocluster, strategy = coclusterStrategy(), nbCore = 1)
}
\arguments{
\item{data}{Input data as matrix (or list containing data matrix, numeric vector for row effects and numeric 
vector column effects in case of contingency data with known row and column effects.)}

\item{semisupervised}{Boolean value specifying whether to perform semi-supervised
co-clustering or not. Make sure to provide row and/or column labels if
specified value is true. The default value is false.}

\item{rowlabels}{Integer Vector specifying the class of rows. The class number starts from zero. Provide -1 for unknown row class.}

\item{collabels}{Integer Vector specifying the class of columns. The class number starts from zero. Provide -1 for unknown column class.}

\item{model}{This is the name of model. The following models exists for Poisson data:
\tabular{rlll}{
    pik_rhol_unknown(default) \tab contingency \tab unequal \tab N.A \cr
    pi_rho_unknown \tab contingency \tab equal \tab N.A \cr
    pik_rhol_known \tab contingency \tab unequal \tab N.A \cr
    pi_rho_known \tab contingency \tab equal \tab N.A \cr
}}

\item{nbcocluster}{Integer vector specifying the number of row and column clusters respectively.}

\item{strategy}{Object of class \code{\linkS4class{strategy}}.}

\item{nbCore}{number of thread to use (OpenMP must be available), 0 for all cores. Default value is 1.}
}
\value{
Return an object of \code{\linkS4class{BinaryOptions}} or \code{\linkS4class{ContingencyOptions}}
or \code{\linkS4class{ContinuousOptions}} depending on whether the data-type is Binary, Contingency or Continuous
respectively.
}
\description{
This function performs Co-Clustering (simultaneous clustering of rows and columns )
for Contingency data-sets using latent block models.It can also be used to
perform semi-supervised co-clustering.
}
\examples{

## Simple example with simulated contingency data
## load data
data(contingencydataunknown)
## usage of coclusterContingency function in its most simplest form
strategy = coclusterStrategy( nbinititerations = 5, nbxem = 2, nbiterations_int = 2
                            , nbiterationsxem = 10, nbiterationsXEM = 100, epsilonXEM=1e-5)
out<-coclusterContingency( contingencydataunknown, nbcocluster=c(2,3), strategy = strategy)
## Summarize the output results
summary(out)
## Plot the original and Co-clustered data 
plot(out)

}
