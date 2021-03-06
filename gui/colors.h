// ************************************************************************** //
//
//  STeCa2:    StressTextureCalculator ver. 2
//
//! @file      colors.h
//! @brief     Colors, scales, and palletes.
//!
//! @homepage  http://apps.jcns.fz-juelich.de/steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016
//! @authors   Scientific Computing Group at MLZ Garching
//! @authors   Rebecca Brydon, Jan Burle,  Antti Soininen
//! @authors   Based on the original STeCa by Christian Randau
//
// ************************************************************************** //

#ifndef COLORS_H
#define COLORS_H

#include "def/defs.h"

#include <QRgb>

namespace gui {
//------------------------------------------------------------------------------

QRgb intenImage(qreal inten, qreal maxInten = 1);
QRgb intenGraph(qreal inten, qreal maxInten = 1);
QRgb heatmapColor(qreal value);

//------------------------------------------------------------------------------
}
#endif // COLORS_H
