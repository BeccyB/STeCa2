// ************************************************************************** //
//
//  STeCa2:    StressTextureCalculator ver. 2
//
//! @file      core_session.cpp
//!
//! @homepage  http://apps.jcns.fz-juelich.de/steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016
//! @authors   Scientific Computing Group at MLZ Garching
//! @authors   Rebecca Brydon, Jan Burle,  Antti Soininen
//! @authors   Based on the original STeCa by Christian Randau
//
// ************************************************************************** //

#include "session.h"
#include "typ/typ_matrix.h"
#include "fit/fit_fun.h"
#include <qmath.h>

namespace core {
//------------------------------------------------------------------------------

using namespace typ;
using namespace data;
using namespace calc;

Session::Session() : angleMapCache_(360) {
  clear();
  fit::initFactory();
}

void Session::clear() {
  while (0 < numFiles())
    remFile(0);

  remCorrFile();
  corrEnabled_ = false;

  bgPolyDegree_ = 0;
  bgRanges_.clear();

  reflections_.clear();

  norm_ = eNorm::NONE;

  angleMapCache_.clear();
}

shp_File Session::file(uint i) const {
  return files_.at(i);
}

bool Session::hasFile(rcstr fileName) {
  QFileInfo fileInfo(fileName);
  for (auto& file : files_)
    if (fileInfo == file->fileInfo())
      return true;
  return false;
}

void Session::addFile(shp_File file) THROWS {
  setImageSize(file->datasets().imageSize());
  // all ok
  files_.append(file);
}

void Session::remFile(uint i) {
  files_.remove(i);
  updateImageSize();
}

void Session::setCorrFile(shp_File file) THROWS {
  if (file.isNull()) {
    remCorrFile();
  } else {
    auto& datasets = file->datasets();

    setImageSize(datasets.imageSize());
    corrImage_ = datasets.foldedImage();

    // all ok
    corrFile_    = file;
    corrEnabled_ = true;
  }
}

void Session::remCorrFile() {
  corrFile_.clear();
  corrImage_.clear();
  corrEnabled_ = false;
  updateImageSize();
}

void Session::tryEnableCorr(bool on) {
  corrEnabled_ = on && hasCorrFile();
}

void Session::collectDatasetsFromFiles(uint_vec fileNums, pint combineBy) {

  collectedFromFiles_ = fileNums;
  collectedDatasets_.clear();
  collectedDatasetsTags_.clear();

  vec<shp_OneDataset> datasetsFromFiles;
  for (uint i : collectedFromFiles_)
    for (auto& dataset : files_.at(i)->datasets())
      datasetsFromFiles.append(dataset);

  if (datasetsFromFiles.isEmpty())
    return;

  shp_Dataset cd(new Dataset);
  uint i = 0;

  auto appendCd = [this, &cd, &combineBy, &i]() {
    uint cnt = cd->count();
    if (cnt) {
      str tag = str::number(i + 1);
      i += cnt;

      if (combineBy > 1)
        tag += '-' + str::number(i);

      collectedDatasets_.appendHere(cd);
      collectedDatasetsTags_.append(tag);

      cd = shp_Dataset(new Dataset);
    }
  };

  uint by = combineBy;
  for (auto& dataset : datasetsFromFiles) {
    cd->append(shp_OneDataset(dataset));
    if (1 >= by--) {
      appendCd();
      by = combineBy;
    }
  }

  appendCd();  // the remaining ones
}

void Session::updateImageSize() {
  if (0 == numFiles() && !hasCorrFile())
    imageSize_ = size2d(0, 0);
}

void Session::setImageSize(size2d::rc size) THROWS {
  RUNTIME_CHECK(!size.isEmpty(), "bad image size");
  if (imageSize_.isEmpty())
    imageSize_ = size;  // the first one
  else if (imageSize_ != size)
    THROW("inconsistent image size");
}

size2d Session::imageSize() const {
  return imageTransform_.isTransposed() ? imageSize_.transposed() : imageSize_;
}

void Session::setImageTransformMirror(bool on) {
  imageTransform_ = imageTransform_.mirror(on);
}

void Session::setImageTransformRotate(ImageTransform::rc rot) {
  imageTransform_ = imageTransform_.rotateTo(rot);
}

void Session::setImageCut(bool topLeftFirst, bool linked, ImageCut::rc cut) {
  auto size = imageSize_;

  if (size.isEmpty())
    imageCut_ = ImageCut();
  else {
    auto limit = [linked](uint& m1, uint& m2, uint maxTogether) {
      if (linked && m1 + m2 >= maxTogether) {
        m1 = m2 = qMax((maxTogether - 1) / 2, 0u);
      } else {
        m1 = qMax(qMin(m1, maxTogether - m2 - 1), 0u);
        m2 = qMax(qMin(m2, maxTogether - m1 - 1), 0u);
      }
    };

    // make sure that cut values are valid; in the right order
    uint left = cut.left, top = cut.top, right = cut.right, bottom = cut.bottom;

    if (topLeftFirst) {
      limit(top, bottom, size.h);
      limit(left, right, size.w);
    } else {
      limit(bottom, top, size.h);
      limit(right, left, size.w);
    }

    imageCut_ = ImageCut(left, top, right, bottom);
  }
}

void Session::setGeometry(preal detectorDistance, preal pixSize,
                          bool isMidPixOffset, IJ::rc midPixOffset) {

  geometry_.detectorDistance = detectorDistance;
  geometry_.pixSize          = pixSize;
  geometry_.isMidPixOffset   = isMidPixOffset;
  geometry_.midPixOffset     = midPixOffset;
}

IJ Session::midPix() const {
  auto halfSize = imageSize().scaled(.5);
  IJ mid(to_i(halfSize.w), to_i(halfSize.h));

  if (geometry_.isMidPixOffset) {
    IJ::rc off = geometry_.midPixOffset;
    mid.i += off.i;
    mid.j += off.j;
  }

  return mid;
}

shp_AngleMap Session::angleMap(OneDataset::rc one) const {
  AngleMap::Key key(geometry_, imageSize_, imageCut_, midPix(), one.midTth());
  shp_AngleMap map = angleMapCache_.value(key);
  if (map.isNull())
    map = angleMapCache_.insert(key, shp_AngleMap(new AngleMap(key)));
  return map;
}

shp_AngleMap Session::angleMap(Session::rc session, OneDataset::rc one) {
  return session.angleMap(one);
}

shp_ImageLens Session::imageLens(
    Image::rc image, Datasets::rc datasets, bool trans, bool cut) const {
  return shp_ImageLens(
      new ImageLens(*this, image, corrEnabled_ ? &corrImage_ : nullptr,
                    datasets, trans, cut, imageCut_, imageTransform_));
}

shp_DatasetLens Session::datasetLens(Dataset::rc dataset, Datasets::rc datasets, eNorm norm,
                                     bool trans, bool cut) const {
  return shp_DatasetLens(new DatasetLens(*this, dataset, datasets, norm,
                         trans, cut, imageTransform_, imageCut_));
}

// Calculates the polefigure coordinates alpha and beta with regards to
// sample orientation and diffraction angles.
// tth: Center of reflection's 2theta interval.
// gma: Center of gamma slice.
void calculateAlphaBeta(Dataset::rc dataset, tth_t tth, gma_t gma,
                        deg& alpha, deg& beta) {
  // Sample rotations.
  rad omg = dataset.omg().toRad();
  rad phi = dataset.phi().toRad();
  rad chi = dataset.chi().toRad();

  // Rotate a unit vector initially parallel to the y axis with regards to the
  // angles. As a result, the vector is a point on a unit sphere
  // corresponding to the location of a polefigure point.
  // Note that the rotations here do not correspond to C. Randau's dissertation.
  // The rotations given in [J. Appl. Cryst. (2012) 44, 641-644] are incorrect.
  vector3d rotated =
      matrix3d::rotationCWz(phi)
    * matrix3d::rotationCWx(chi)
    * matrix3d::rotationCWz(omg)
    * matrix3d::rotationCWx(gma.toRad())
    * matrix3d::rotationCCWz(tth.toRad() / 2)
    * vector3d(0, 1, 0);

  // Extract alpha (latitude) and beta (longitude).
  rad alphaRad = acos(rotated._2);
  rad betaRad  = atan2(rotated._0, rotated._1);

  // If alpha is in the wrong hemisphere, mirror both alpha and beta over the
  // center of a unit sphere.
  if (alphaRad > M_PI_2) {  // REVIEW - seems like that happens only for a very narrow ring
    alphaRad = qAbs(alphaRad - M_PI);
    betaRad  = betaRad + (betaRad < 0 ? M_PI : -M_PI);
  }

  // Keep beta between 0 and 2pi.
  if (betaRad < 0) betaRad = betaRad + 2 * M_PI;

  alpha = alphaRad.toDeg();
  beta  = betaRad.toDeg();
}

Curve Session::makeCurve(DatasetLens::rc lens, gma_rge::rc rgeGma) const {
  Curve curve = lens.makeCurve(rgeGma);
  curve.subtract(fit::Polynom::fromFit(bgPolyDegree_, curve, bgRanges_));

  return curve;
}

// Fits reflection to the given gamma sector and constructs a ReflectionInfo.
ReflectionInfo Session::makeReflectionInfo(
    DatasetLens::rc lens, Reflection::rc reflection, gma_rge::rc gmaSector) const {
  Curve curve = makeCurve(lens, gmaSector);

  scoped<fit::PeakFunction*> peakFunction(reflection.peakFunction().clone());

  peakFunction->fit(curve);

  Range::rc rgeTth = peakFunction->range();
  deg     alpha, beta;

  Dataset::rc dataset = lens.dataset();
  calculateAlphaBeta(dataset, rgeTth.center(), gmaSector.center(), alpha,
                     beta);

  peak_t peak      = peakFunction->fittedPeak();
  fwhm_t fwhm      = peakFunction->fittedFWHM();
  peak_t peakError = peakFunction->peakError();
  fwhm_t fwhmError = peakFunction->fwhmError();

  shp_Metadata metadata = dataset.metadata();

  return rgeTth.contains(peak.x)
             ? ReflectionInfo(metadata, alpha, beta, gmaSector,
                              inten_t(peak.y), inten_t(peakError.y),
                              tth_t(peak.x),   tth_t(peakError.x),
                              fwhm_t(fwhm),    fwhm_t(fwhmError))
             : ReflectionInfo(metadata, alpha, beta, gmaSector);
}

/* Gathers ReflectionInfos from Datasets.
 * Either uses the whole gamma range of the datasets (if gammaSector is
 * invalid), or user limits the range.
 * Even though the betaStep of the equidistant polefigure grid is needed here,
 * the returned infos won't be on the grid. REVIEW gammaStep separately?
 */
ReflectionInfos Session::makeReflectionInfos(
    Datasets::rc datasets, Reflection::rc reflection,
    gma_t gmaStep, gma_rge::rc rgeGma, Progress* progress)
{
  ReflectionInfos infos;

  for (auto& dataset : datasets) {
    if (progress)
      progress->step();

    auto lens = datasetLens(*dataset, datasets, norm_, true, true);
    Range rge = lens->rgeGma(); // REVIEW at mid tth?
    if (rgeGma.isValid())
      rge = rge.intersect(rgeGma);

    if (rge.isEmpty())
      continue;

    qreal step = gmaStep;
    for_i (rge.numSlices(step)) {
      qreal min = rge.min + i * step;
      gma_rge gmaStripe(min, min + step);
      auto  refInfo = makeReflectionInfo(*lens, reflection, gmaStripe);
      if (!qIsNaN(refInfo.inten()))
        infos.append(refInfo);
    }
  }

  return infos;
}

void Session::setBgRanges(Ranges::rc ranges) {
  bgRanges_ = ranges;
}

bool Session::addBgRange(Range::rc range) {
  return bgRanges_.add(range);
}

bool Session::remBgRange(Range::rc range) {
  return bgRanges_.rem(range);
}

void Session::setBgPolyDegree(uint degree) {
  bgPolyDegree_ = degree;
}

void Session::addReflection(shp_Reflection reflection) {
  EXPECT(!reflection.isNull())
  reflections_.append(reflection);
}

void Session::remReflection(uint i) {
  reflections_.remove(i);
}

void Session::setNorm(eNorm norm) {
  norm_ = norm;
}

qreal Session::calcAvgBackground(Dataset::rc dataset) const {
  auto lens = datasetLens(dataset, dataset.datasets(), eNorm::NONE, true, true);

  Curve gmaCurve = lens->makeCurve();
  auto bgPolynom = fit::Polynom::fromFit(bgPolyDegree_, gmaCurve, bgRanges_);
  return bgPolynom.avgY(lens->rgeTth());
}

qreal Session::calcAvgBackground(Datasets::rc datasets) const {
  TakesLongTime __;

  qreal bg = 0;

  for (auto& dataset : datasets)
    bg += calcAvgBackground(*dataset);

  return bg / datasets.count();
}

//------------------------------------------------------------------------------
}
// eof
