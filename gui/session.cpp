#include "session.h"
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

  setCorrFile(top["corr.file"].toString());

  auto cut = top["cut"].toObject();
  int y1 = cut["top"].toInt();
  int y2 = cut["bottom"].toInt();
  int x1 = cut["left"].toInt();
  int x2 = cut["right"].toInt();

  setImageCut(true,false,Session::imagecut_t(x1,y1,x2,y2));

  auto det = top["detector"].toObject();

  detector.distance     = det["distance"].toDouble();
  detector.pixelSize    = det["pixelsize"].toDouble();
  detector.isBeamOffset = det["isbeamoffset"].toBool();
  WT(detector.isBeamOffset)
  detector.beamOffset.setX(det["offset_x"].toDouble());
  detector.beamOffset.setY(det["offset_y"].toDouble());

  emit sessionLoaded();
}

QByteArray Session::save() const {
  QByteArray json;

  QJsonObject top;

  QJsonObject det;

  det["distance"]     = detector.distance;
  det["pixel size"]   = detector.pixelSize;
  det["isbeamoffset"] = detector.isBeamOffset;
  det["offset_x"]     = detector.beamOffset.x();
  det["offset_y"]     = detector.beamOffset.y();

  top["detector"] = det;

  QJsonArray files;

  for (auto file: dataFiles) {
    files.append(file->getInfo().absoluteFilePath());
  }

  top["files"] = files;
  if (!corrFile.isNull())
    top["corr.file"] = corrFile->getInfo().absoluteFilePath();

  QJsonObject cut;

  cut["top"]    = imageCut.top;
  cut["bottom"] = imageCut.bottom;
  cut["left"]   = imageCut.left;
  cut["right"]  = imageCut.right;

  top["cut"] = cut;

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
  if ((uint)dataFiles.count() == i)
    super::setCorrFile(NULL_STR);
  else
    super::remFile(i);

  if (0==numFiles(true))
    setImageCut(true,false,imagecut_t());

  emit filesChanged();
}

void Session::setCorrFile(rcstr filePath) {
  super::setCorrFile(filePath);
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
  emit imageCutChanged();
}

//void Session::setImageCut(bool topLeft, bool linked, int top, int bottom, int left, int right)
//{
//  setImageCut(topLeft,linked,imagecut_t(top,bottom,left,right));
//}

Session::detector_t::detector_t()
: distance(0), pixelSize(0), isBeamOffset(false), beamOffset() {
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
      str s = session.getFile(row).name();
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
  return coreFile ? coreFile->getDatasets().count() : 0;
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
    return core::Datasets::getAttributeTag(attributeNums[section-2]);
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

core::Dataset &Session::DatasetViewModel::getDataset(int row) const {
  return *coreFile->getDatasets().at(row);
}

// eof
