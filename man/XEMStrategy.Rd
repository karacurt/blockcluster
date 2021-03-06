% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/RCocluster.R
\name{XEMStrategy}
\alias{XEMStrategy}
\title{An EM strategy to obtain a good optimum.}
\description{
In Co-clustering, there could be many local optimal where the algorithm may
get struck resulting in sub-optimum results. Hence we applied a strategy
called XEM strategy to run the EM algorithm. The various steps are defined
as follows:
}
\details{
\describe{
\item{Step-1, "xem" step:}{Do several runs of: "initialization followed by
short run of algorithm (few iterations/high tolerance)". This parameter is
named as "nbxem" in \code{\link{coclusterStrategy}} function. Default value
is 5. We call this step as xem step.}
\item{Step-2, "XEM" step:}{Select the best result of step 1 and make long run
of Algorithm(high iterations/low tolerance).We call this step as XEM step.}
\item{Step-3}{Repeat step 1 and 2 several times and select the best result.
The number of repetitions can be modified via parameter "nbtry" of
\code{\link{coclusterStrategy}} function. Default value is 2. } 
}
}
