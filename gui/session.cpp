#include "session.h"
#include "mainwin.h"
#include <QAction>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>

class WaitCursor {
public:
  WaitCursor() {
    QApplication::setOverrideCursor(Qt::WaitCursor);
  }

 ~WaitCursor() {
    QApplication::restoreOverrideCursor();
  }
};

Session::Session()
: fileViewModel(*this), datasetViewModel(*this) {
}

Session::~Session(){
}

void Session::load(QFileInfo const& fileInfo) THROWS {
  QFile file(fileInfo.absoluteFilePath());
  RUNTIME_CHECK(file.open(QIODevice::ReadOnly), "File cannot be opened");
  load(file.readAll());
}

// TODO merge load / save, DRY
void Session::load(QByteArray const& json) THROWS {
  QJsonParseError parseError;
  QJsonDocument doc(QJsonDocument::fromJson(json,&parseError));
  RUNTIME_CHECK(QJsonParseError::NoError==parseError.error, "Error parsing file");

  WaitCursor __;

  while (numFiles(true)>0) remFile(0);

  auto top   = doc.object();
  auto files = top["files"].toArray();

  for (auto file: files) {
    addFile(file.toString());
  }

  loadCorrFile(top["corr.file"].toString());

  auto cut = top["cut"].toObject();
  int y1 = qMax(0,cut["top"].toInt());
  int y2 = qMax(0,cut["bottom"].toInt());
  int x1 = qMax(0,cut["left"].toInt());
  int x2 = qMax(0,cut["right"].toInt());

  setImageCut(true,false,core::ImageCut(x1,y1,x2,y2));

  auto det = top["detector"].toObject();

  setDetectorGeometry(
    det["distance"].toDouble(),
    det["pixel_size"].toDouble());

  setBeamGeometry(
    det["hasbeamoffset"].toBool(),
    QPoint(det["offset_x"].toDouble(),det["offset_y"].toDouble()));

  setImageRotate(core::ImageTransform(top["transform"].toInt()));
}

QByteArray Session::save() const {

  auto const &g = getGeometry();
  QJsonObject det {
    { "distance",     g.sampleDetectorSpan  },
    { "pixel_size",   g.pixSpan             },
    { "isbeamoffset", g.hasBeamOffset       },
    { "offset_x",     g.middlePixOffset.x() },
    { "offset_y",     g.middlePixOffset.y() },
  };

  QJsonArray files;
  for_i(numFiles()) {
    files.append(getFile(i)->getInfo().absoluteFilePath());
  }

  auto const &ic = getImageCut();
  QJsonObject cut {
    { "top",    (int)ic.top     },
    { "bottom", (int)ic.bottom  },
    { "left",   (int)ic.left    },
    { "right",  (int)ic.right   },
  };

  QJsonObject top {
    { "detector",   det                 },
    { "cut",        cut                 },
    { "transform",  imageTransform.val  },
    { "files",      files               },
    { "corr.file",  hasCorrFile() ? getCorrFile()->getInfo().absoluteFilePath() : "" },
  };

  return QJsonDocument(top).toJson();
}

void Session::addFile(rcstr filePath) THROWS {
  WaitCursor __;

  core::shp_File file = super::addFile(filePath);
  if (!file.isNull()) {
    emit fileAdded(file);
    emit filesChanged();
  }
}

void Session::addFiles(str_lst filePaths) THROWS {
  WaitCursor __;

  for (auto &filePath: filePaths)
    addFile(filePath);
}

void Session::remFile(uint i) {
  if (hasCorrFile() && numFiles(true) == i+1) {
    super::remCorrFile();
    emit corrFileSet(core::shp_File());
    emit filesChanged();
  } else {
    auto file = super::remFile(i);
    emit fileRemoved(file);
    emit filesChanged();
  }

  if (0==numFiles(true)) {
    setSelectedDataset(core::shp_Dataset());
    setImageCut(true,false,core::ImageCut());
  }
}

void Session::loadCorrFile(rcstr filePath) {
  auto file = super::loadCorrFile(filePath);
  emit corrFileSet(file);
  emit filesChanged();
}

void Session::setSelectedFile(core::shp_File file) {
  emit fileSelected(file);
}

void Session::setSelectedDataset(core::shp_Dataset dataset) {
  emit datasetSelected(dataset);
}

void Session::setImageCut(bool topLeft, bool linked, core::ImageCut const& imageCut) {
  super::setImageCut(topLeft,linked,imageCut);
  calcIntensCorrArray();
  emit geometryChanged();
}

void Session::setDetectorGeometry(qreal sampleDetectorSpan, qreal pixSpan) {
  super::setDetectorGeometry(sampleDetectorSpan,pixSpan);
  emit geometryChanged();
}

void Session::setBeamGeometry(bool hasBeamOffset, QPoint const& middlePixOffset) {
  super::setBeamGeometry(hasBeamOffset,middlePixOffset);
  emit geometryChanged();
}

void Session::setImageMirror(bool on) {
  actImageMirror->setChecked(on);
  super::setImageMirror(on);
  emit geometryChanged();
}

void Session::setImageRotate(core::ImageTransform rot) {
  pcstr rotateIconFile, mirrorIconFile;
  switch (rot.val & 3) {
  case 0:
    rotateIconFile = ":/icon/rotate0";
    mirrorIconFile = ":/icon/mirror_horz";
    break;
  case 1:
    rotateIconFile = ":/icon/rotate1";
    mirrorIconFile = ":/icon/mirror_vert";
    break;
  case 2:
    rotateIconFile = ":/icon/rotate2";
    mirrorIconFile = ":/icon/mirror_horz";
    break;
  case 3:
    rotateIconFile = ":/icon/rotate3";
    mirrorIconFile = ":/icon/mirror_vert";
    break;
  }

  actImageRotate->setIcon(QIcon(rotateIconFile));
  actImageMirror->setIcon(QIcon(mirrorIconFile));
  super::setImageRotate(rot);
  emit geometryChanged();
}

void Session::nextImageRotate() {
  setImageRotate(imageTransform.nextRotate());
}

void Session::doSaveSettings() {
  emit saveSettings();
}

void Session::doReadSettings() {
  emit readSettings();
}

// eof
