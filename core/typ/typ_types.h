// ************************************************************************** //
//
//  STeCa2:    StressTextureCalculator ver. 2
//
//! @file      typ_types.h
//! @brief     Basic core data types
//!
//! @homepage  http://apps.jcns.fz-juelich.de/steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016
//! @authors   Scientific Computing Group at MLZ Garching
//! @authors   Rebecca Brydon, Jan Burle,  Antti Soininen
//! @authors   Based on the original STeCa by Christian Randau
//
// ************************************************************************** //

#ifndef TYP_TYPES_H
#define TYP_TYPES_H

#include "typ/typ_angles.h"
#include "typ/typ_vec.h"

namespace typ {
//------------------------------------------------------------------------------

typedef deg   tth_t;
typedef float inten_t;
typedef float fwhm_t;

typedef typ::vec<inten_t> inten_vec;

enum class eNorm {
  NONE,
  DELTA_MONITOR_COUNT, DELTA_TIME, BACKGROUND,
};

//------------------------------------------------------------------------------
}
#endif // TYP_TYPES_H