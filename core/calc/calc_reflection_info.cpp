// ************************************************************************** //
//
//  STeCa2:    StressTextureCalculator ver. 2
//
//! @file      calc_reflection_info.cpp
//!
//! @homepage  http://apps.jcns.fz-juelich.de/steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016
//! @authors   Scientific Computing Group at MLZ Garching
//! @authors   Rebecca Brydon, Jan Burle,  Antti Soininen
//! @authors   Based on the original STeCa by Christian Randau
//
// ************************************************************************** //

#include "calc_reflection_info.h"

namespace calc {
//------------------------------------------------------------------------------

using namespace typ;
using namespace data;

/* NOTE Invalid output parameters are set to NaNs. However, some analysis
 * programs
 * require -1 as unknown value; thus, NaN parameter values should be output
 * as -1 when output is written for these programs (polefigure!).
 */

str_lst ReflectionInfo::dataTags() {
  static str_lst tags;
  if (tags.isEmpty()) {
    for_i (uint(eReflAttr::NUM_REFL_ATTR))
      tags.append(reflStringTag(i));

    tags.append(Metadata::attributeTags());
  }

  return tags;
}

str const ReflectionInfo::reflStringTag(uint attr) {
  switch (eReflAttr(attr)) {
  case eReflAttr::ALPHA:
    return "α";
  case eReflAttr::BETA:
    return "β";
  case eReflAttr::GAMMA1:
    return "γ1";
  case eReflAttr::GAMMA2:
    return "γ2";
  case eReflAttr::INTEN:
    return "inten";
  case eReflAttr::SIGMA_INTEN:
    return "σinten";
  case eReflAttr::TTH:
    return "2θ";
  case eReflAttr::SIGMA_TTH:
    return "σ2θ";
  case eReflAttr::FWHM:
    return "fwhm";
  case eReflAttr::SIGMA_FWHM:
    return "σfwhm";
  default:
    NEVER;
    return nullptr;
  }
}

cmp_vec ReflectionInfo::dataCmps() {
  static cmp_vec cmps;
  if (cmps.isEmpty()) {
    cmps = cmp_vec { cmp_real, cmp_real, cmp_real, cmp_real,
                     cmp_real, cmp_real, cmp_real,
                     cmp_real, cmp_real, cmp_real };
    cmps.append(Metadata::attributeCmps());
  }

  return cmps;
}

ReflectionInfo::ReflectionInfo()
: ReflectionInfo(shp_Metadata(),
                 qQNaN(), qQNaN(), Range(), inten_t(qQNaN()), qQNaN(), qQNaN(),
                 qQNaN(), qQNaN(), qQNaN())
{
}

ReflectionInfo::ReflectionInfo(shp_Metadata md, deg alpha, deg beta,
                               Range::rc rgeGamma,
                               inten_t inten, qreal intenError,
                               deg tth, deg tthError,
                               qreal fwhm, qreal fwhmError)
: md_(md), alpha_(alpha), beta_(beta), rgeGamma_(rgeGamma)
, inten_(inten), intenError_(intenError)
, tth_(tth), tthError_(tthError)
, fwhm_(fwhm), fwhmError_(fwhmError)
{
}

ReflectionInfo::ReflectionInfo(shp_Metadata md, deg alpha, deg beta,
                               Range::rc rgeGamma)
: ReflectionInfo(md, alpha, beta, rgeGamma, inten_t(qQNaN()), qQNaN(), qQNaN(), qQNaN(), qQNaN(), qQNaN())
{
}

ReflectionInfo::ReflectionInfo(deg alpha, deg beta, Range::rc rgeGamma,
  inten_t inten, qreal intenError, deg tth, deg tthError, qreal fwhm, qreal fwhmError)
: ReflectionInfo(shp_Metadata(), alpha, beta, rgeGamma, inten, intenError, tth, tthError, fwhm, fwhmError)
{
}

ReflectionInfo::ReflectionInfo(deg alpha, deg beta)
: ReflectionInfo(alpha, beta, Range(), inten_t(qQNaN()), qQNaN(), qQNaN(), qQNaN(), qQNaN(), qQNaN())
{
}

row_t ReflectionInfo::data() const {
  row_t row{QVariant(alpha()), QVariant(beta()),
            QVariant(rgeGamma().min), QVariant(rgeGamma().max),
            QVariant(inten()), QVariant(intenError()),
            QVariant(tth()),   QVariant(tthError()),
            QVariant(fwhm()),  QVariant(fwhmError())};

  row.append(md_ ? md_->attributeValues() : Metadata::attributeNaNs());
  return row;
}

//------------------------------------------------------------------------------

ReflectionInfos::ReflectionInfos() {
  invalidate();
}

void ReflectionInfos::append(ReflectionInfo::rc info) {
  super::append(info);
  invalidate();
}

qreal ReflectionInfos::averageInten() const {
  if (qIsNaN(avgInten_)) {
    avgInten_ = 0;
    uint cnt  = 0;

    for (auto const& info : *this) {
      qreal inten = info.inten();
      if (qIsFinite(inten)) {
        avgInten_ += inten;
        ++cnt;
      }
    }

    if (cnt) avgInten_ /= cnt;
  }

  return avgInten_;
}

Range::rc ReflectionInfos::rgeInten() const {
  if (!rgeInten_.isValid()) {
    for_i (count())
      rgeInten_.extendBy(at(i).inten());
  }

  return rgeInten_;
}

void ReflectionInfos::invalidate() {
  avgInten_ = qQNaN();
  rgeInten_.invalidate();
}

//------------------------------------------------------------------------------
}

// eof