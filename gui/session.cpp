#include "session.h"
#include "mainwin.h"
#include <QAction>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

Session::Session()
: selectedFile(nullptr), fileViewModel(*this), datasetViewModel() {
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

  setImageCut(true,false,Session::imagecut_t(x1,y1,x2,y2));

  auto det = top["detector"].toObject();

  sampleDetectorSpan = det["distance"].toDouble();
  pixSpan            = det["pixel_size"].toDouble();
  hasBeamOffset      = det["hasbeamoffset"].toBool();
  middlePixXOffset   = det["offset_x"].toDouble();
  middlePixYOffset   = det["offset_y"].toDouble();

  setRotate(core::Image::Transform(top["transform"].toInt()));

  emit sessionLoaded();
}

// TODO mainwin passed to access transformation actions - rethink
QByteArray Session::save() const {
  QByteArray json;

  QJsonObject top;

  QJsonObject det;

  det["distance"]     = sampleDetectorSpan;
  det["pixel_size"]   = pixSpan;
  det["isbeamoffset"] = hasBeamOffset;
  det["offset_x"]     = middlePixXOffset;
  det["offset_y"]     = middlePixYOffset;

  top["detector"] = det;

  QJsonArray files;

  for (auto file: dataFiles) {
    files.append(file->getInfo().absoluteFilePath());
  }

  top["files"] = files;
  if (!corrFile.isNull())
    top["corr.file"] = corrFile->getInfo().absoluteFilePath();

  QJsonObject cut;

  cut["top"]    = (int)imageCut.top;
  cut["bottom"] = (int)imageCut.bottom;
  cut["left"]   = (int)imageCut.left;
  cut["right"]  = (int)imageCut.right;

  top["cut"] = cut;
  top["transform"] = imageTransform.val;

  QJsonDocument doc(top);
  return doc.toJson();
}

void Session::addFile(rcstr filePath) THROWS {
  WaitCursor __;

  super::addFile(filePath);
  emit filesChanged();
}

void Session::addFiles(str_lst filePaths) THROWS {
  WaitCursor __;

  for (auto& filePath: filePaths)
    super::addFile(filePath);
  emit filesChanged();
}

void Session::remFile(uint i) {
  super::remFile(i);

  setSelectedFile(nullptr);
  emit filesChanged();

  if (0==numFiles(true))
    setImageCut(true,false,imagecut_t());
}

void Session::loadCorrFile(rcstr filePath) {
  super::loadCorrFile(filePath);
  emit filesChanged();
}

void Session::setSelectedFile(pcCoreFile file) {
  setSelectedDataset(nullptr);
  emit fileSelected((selectedFile = file));
}

void Session::setSelectedDataset(pcCoreDataset dataset) {
  emit datasetSelected((selectedDataset = dataset));
}

void Session::setImageCut(bool topLeft, bool linked, imagecut_t const& imageCut) {
  super::setImageCut(topLeft,linked,imageCut);
  calcIntensCorrArray();
  emit imageCutChanged();
}

void Session::setMirror(bool on) {
  actImageMirror->setChecked(on);
  super::setMirror(on);
}

void Session::setRotate(core::Image::Transform rot) {
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
  super::setRotate(rot);
}

void Session::nextRotate() {
  setRotate(imageTransform.rotate());
}

//-----------------------------------------------------------------------------

Session::FileViewModel::FileViewModel(Session& session_): session(session_) {
}

int Session::FileViewModel::rowCount(QModelIndex const&) const {
  return session.numFiles(true);
}

QVariant Session::FileViewModel::data(QModelIndex const& index,int role) const {
  auto row = index.row(), cnt = rowCount(index);
  if (row < 0 || row >= cnt) return QVariant();

  bool isCorrectionFile = session.hasCorrFile() && row+1 == cnt;

  switch (role) {
    case IsCorrectionFileRole:
      return isCorrectionFile;
    case Qt::DisplayRole: {
      str s = session.getFile(row).getName();
      static str Corr("Corr: ");
      if (isCorrectionFile) s = Corr + s;
      return s;
    }
    case GetFileRole:
      return QVariant::fromValue<pcCoreFile>(&(session.getFile(row)));
    default:
      return QVariant();
  }
}

//-----------------------------------------------------------------------------

Session::DatasetViewModel::DatasetViewModel()
: coreFile(nullptr), infoItems(nullptr) {
}

int Session::DatasetViewModel::columnCount(const QModelIndex &) const {
  return attributeNums.count() + 2; // 1 for the hidden 0-th column, 1 for the sequence number
}

int Session::DatasetViewModel::rowCount(QModelIndex const&) const {
  return coreFile ? coreFile->numDatasets() : 0;
}

QVariant Session::DatasetViewModel::data(QModelIndex const& index,int role) const {
  if (!coreFile) return QVariant();

  int row = index.row(), col = index.column(), cnt = rowCount(index);
  if (row < 0 || row >= cnt || col < 0 || col-2 >= attributeNums.count()) return QVariant();

  switch (role) {
    case Qt::DisplayRole: {
      switch (col) {
      case 0:
        return QVariant();
      case 1:
        return str().setNum(row+1);
      default:
        return getDataset(row).getAttributeStrValue(attributeNums[col-2]);
      }
    }
    case GetDatasetRole:
      return QVariant::fromValue<pcCoreDataset>(&getDataset(row));
    default:
      return QVariant();
  }
}

QVariant Session::DatasetViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (Qt::Horizontal != orientation || Qt::DisplayRole != role
      || section < 1 || section-2 >= attributeNums.count())
    return QVariant();

  switch (section) {
  case 1:
    return "#";
  default:
    return core::Dataset::getAttributeTag(attributeNums[section-2]);
  }
}

void Session::DatasetViewModel::setCoreFile(pcCoreFile coreFile_) {
  beginResetModel();
  coreFile = coreFile_;
  endResetModel();
}

void Session::DatasetViewModel::setInfoItems(panel::DatasetInfo::InfoItems const* infoItems_) {
  beginResetModel();
  infoItems = infoItems_;
  attributeNums.clear();
  if (infoItems) for_i(infoItems->count()) {
    auto &item = infoItems->at(i);
    if (item.cb->isChecked())
      attributeNums.append(i);
  }
  endResetModel();
}

core::Dataset const& Session::DatasetViewModel::getDataset(int row) const {
  return coreFile->getDataset(row);
}

// eof
