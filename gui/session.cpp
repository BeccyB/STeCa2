#include "session.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

Session::Session(): coreSession(new core::Session)
, selectedFile(nullptr)
, fileListModel(*this), datasetListModel(*this) {
}

Session::~Session(){
}

void Session::load(QFileInfo const& fileInfo) THROWS {
  QFile file(fileInfo.absoluteFilePath());
  RUNTIME_CHECK(file.open(QIODevice::ReadOnly), "File cannot be opened");
  load(file.readAll());
}

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
}

QByteArray Session::save() const {
  QByteArray json;

  QJsonArray files;

  for (auto file: coreSession->getDataFiles()) {
    files.append(file->getInfo().absoluteFilePath());
  }

  QJsonObject top;
  top["files"] = files;
  top["corr.file"] = coreSession->getCorrFile().getInfo().absoluteFilePath();


  QJsonDocument doc(top);
  return doc.toJson();
}

void Session::addFile(rcstr filePath) {
  WaitCursor __;

  coreSession->addFile(filePath);
  emit filesChanged();
}

void Session::addFiles(str_lst filePaths) {
  WaitCursor __;

  for (auto& filePath: filePaths)
    coreSession->addFile(filePath);
  emit filesChanged();
}

void Session::remFile(uint i) {
  auto &files = coreSession->getDataFiles();
  if ((uint)files.count() == i)
    coreSession->setCorrFile(str::null);
  else
    coreSession->remFile(i);
  emit filesChanged();
}

uint Session::numFiles(bool withCorr) {
  return coreSession->getDataFiles().count()
      + (withCorr && hasCorrFile() ? 1 : 0);
}

core::File const& Session::getFile(uint i) {
  auto &files = coreSession->getDataFiles();
  return ((uint)files.count() == i)
    ? coreSession->getCorrFile()
    : *files.at(i);
}

str Session::fileName(uint i) {
  return getFile(i).name();
}

bool Session::hasCorrFile() {
  return coreSession->hasCorrFile();
}

void Session::setCorrFile(rcstr filePath) {
  coreSession->setCorrFile(filePath);
  emit filesChanged();
}

void Session::setSelectedFile(pcCoreFile file) {
  setSelectedDataset(nullptr);
  emit fileSelected((selectedFile = file));
}

void Session::setSelectedDataset(pcCoreDataset dataset) {
  emit datasetSelected((selectedDataset = dataset));
}

core::Dataset const&Session::getDataset(uint i) {
  pcCoreFile file = selectedFile;
  ASSERT(file)
  return *(file->getDatasets().at(i));
}

//-----------------------------------------------------------------------------

Session::FileListModel::FileListModel(Session& session_): session(session_) {
}

int Session::FileListModel::rowCount(QModelIndex const&) const {
  return session.numFiles(true);
}

QVariant Session::FileListModel::data(QModelIndex const& index,int role) const {
  auto row = index.row(), cnt = rowCount(index);
  if (row < 0 || row >= cnt) return QVariant();

  bool isCorrectionFile = session.hasCorrFile() && row+1 == cnt;

  switch (role) {
    case IsCorrectionFileRole:
      return isCorrectionFile;
    case Qt::DisplayRole: {
      str s = session.fileName(row);
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

Session::DatasetListModel::DatasetListModel(Session& session_): session(session_) {
}

int Session::DatasetListModel::rowCount(QModelIndex const&) const {
  pcCoreFile file = session.selectedFile;
  return file ? file->getDatasets().count() : 0;
}

QVariant Session::DatasetListModel::data(QModelIndex const& index,int role) const {
  pcCoreFile file = session.selectedFile;
  if (!file) return QVariant();

  auto row = index.row(), cnt = rowCount(index);
  if (row < 0 || row >= cnt) return QVariant();

  switch (role) {
    case Qt::DisplayRole: {
      str s = str("%1 ").arg(row) % file->getDatasets().at(row)->getComment();
      return s;
    }
    case GetFileRole:
      return QVariant::fromValue<pcCoreDataset>(&(session.getDataset(row)));
    default:
      return QVariant();
  }
}

// eof
