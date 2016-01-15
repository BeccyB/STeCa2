#ifndef SESSION_H
#define SESSION_H

#include "defs.h"
#include "core.h"
#include <QAbstractListModel>
#include <QAbstractTableModel>

// A proxy for access to core::Session
// Models and signals are also here

class Session: public QObject {
  Q_OBJECT
  QSharedPointer<core::Session> coreSession;
public:
  Session();
 ~Session();

  void load(QFileInfo const&)       THROWS;
  void load(QByteArray const& json) THROWS;
  QByteArray save() const;

  void addFile(rcstr filePath)      THROWS;
  void addFiles(str_lst filePaths)  THROWS;
  void remFile(uint i);
  uint numFiles(bool withCorr=false);
  core::File const& getFile(uint i);
  str  fileName(uint i);
  bool hasCorrFile();
  void setCorrFile(rcstr filePath);

  void setSelectedFile(pcCoreFile);
  void setSelectedDataset(pcCoreDataset);

  core::Dataset const& getDataset(uint i);

  struct imagecut_t {
    imagecut_t(int top = 0, int bottom = 0, int left = 0, int right = 0);
    int top, bottom, left, right;
  };

  imagecut_t const& getImageCut() const;

  void setImageCut(bool topLeft, bool linked, imagecut_t const&);
  void setImageCut(bool topLeft, bool linked, int top, int bottom, int left, int right);

private:
  pcCoreFile    selectedFile;
  pcCoreDataset selectedDataset;

  imagecut_t    imageCut;

signals:
  void filesChanged();
  void fileSelected(pcCoreFile);
  void datasetSelected(pcCoreDataset);
  void imageCutChanged();
  void infoItemsChanged();

public:
  class FileViewModel: public QAbstractListModel {
    SUPER(FileViewModel,QAbstractListModel)
  public:
    FileViewModel(Session&);

    enum { GetFileRole = Qt::UserRole, IsCorrectionFileRole };

    int rowCount(QModelIndex const&)      const;
    QVariant data(QModelIndex const&,int) const;

    Session &session;
  };

  FileViewModel fileViewModel;

public:
  class DatasetViewModel: public QAbstractTableModel {
    SUPER(DatasetViewModel,QAbstractTableModel)
  public:
    DatasetViewModel(Session&);

    enum { GetDatasetRole = Qt::UserRole };

    int columnCount(QModelIndex const&)   const;
    int rowCount(QModelIndex const&)      const;
    QVariant data(QModelIndex const&,int) const;

    Session &session; // TODO hide

    void setNumAttributes(int);

  private:
    int numAttributes;
  };

  DatasetViewModel datasetViewModel;
};

#endif
