#ifndef SESSION_H
#define SESSION_H

#include "defs.h"
#include "core.h"
#include <QAbstractListModel>

// A proxy for access to core::Session
// Models and signals also here

class Session: public QObject {
  Q_OBJECT
  QSharedPointer<core::Session> coreSession;
public:
  Session();
 ~Session();

  void load(QFileInfo const&)       THROWS;
  void load(QByteArray const& json) THROWS;
  QByteArray save() const;

  void addFile(rcstr filePath);
  void addFiles(str_lst filePaths);
  void remFile(uint i);
  uint numFiles(bool withCorr=false);
  core::File const& getFile(uint i);
  str  fileName(uint i);
  bool hasCorrFile();
  void setCorrFile(rcstr filePath);

  void setSelectedFile(pcCoreFile);
  void setSelectedDataset(pcCoreDataset);

  core::Dataset const& getDataset(uint i);

private:
  pcCoreFile    selectedFile;
  pcCoreDataset selectedDataset;

signals:
  void filesChanged();
  void fileSelected(pcCoreFile);
  void datasetSelected(pcCoreDataset);

public:
  class FileListModel: public QAbstractListModel {
    SUPER(FileListModel,QAbstractListModel)
  public:
    FileListModel(Session&);

    enum { GetFileRole = Qt::UserRole, IsCorrectionFileRole };

    int rowCount(QModelIndex const&)      const;
    QVariant data(QModelIndex const&,int) const;

    Session &session;
  };

  FileListModel fileListModel;

public:
  class DatasetListModel: public QAbstractListModel {
    SUPER(DatasetListModel,QAbstractListModel)
  public:
    DatasetListModel(Session&);

    enum { GetFileRole = Qt::UserRole };

    int rowCount(QModelIndex const&)      const;
    QVariant data(QModelIndex const&,int) const;

    Session &session;
  };

  DatasetListModel datasetListModel;
};

#endif
