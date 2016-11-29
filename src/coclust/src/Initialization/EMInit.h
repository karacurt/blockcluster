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

/** @file emInit.h
 *  @brief Declares EM initialization class emInit derived from IInit.
 **/

#ifndef EMINIT_H_
#define EMINIT_H_

/** @brief This class provides functionalities for EM initialization. It is  derived from IInit
 * abstract class.
 *
 */
#include "IInit.h"

class emInit: public IInit
{
  public:
    inline emInit(){};
    virtual bool run();
    inline virtual ~emInit(){};
};

inline bool emInit::run()
{
  return p_Model_->emInitStep();
}
#endif /* EMINIT_H_ */