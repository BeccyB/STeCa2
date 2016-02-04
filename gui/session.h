/** \file
 */

#ifndef SESSION_H
#define SESSION_H

#include "defs.h"
#include "core_session.h"
#include "models.h"

/// As core::Session, with models and signals
class Session: public QObject, public core::Session {
  SUPER(Session,core::Session) Q_OBJECT
public:
  Session();
 ~Session();

  void load(QFileInfo const&)       THROWS;
  void load(QByteArray const& json) THROWS;
  QByteArray save() const;

  void addFile(rcstr filePath)      THROWS;
  void addFiles(str_lst filePaths)  THROWS;

  void remFile(uint i); ///< Remove the i-th file, INCLUDING the correction file

  void loadCorrFile(rcstr filePath);

  void setSelectedFile(core::shp_File);
  void setSelectedDataset(core::shp_Dataset);

  void setImageCut(bool topLeft, bool linked, core::ImageCut const&);
  void setGeometry(qreal sampleDetectorSpan, qreal pixSpan, bool hasBeamOffset, QPoint const& middlePixOffset);

  QAction *actImageRotate, *actImageMirror;

  void setImageMirror(bool);
  void setImageRotate(core::ImageTransform);
  void nextImageRotate();

// Session is the only one emitting signals (besides Qt gui widgets)
signals:
  void geometryChanged();
  void imageCutChanged();

  void corrFileSet(core::shp_File);   // may be null

  void fileAdded(core::shp_File);     // not null
  void fileRemoved(core::shp_File);   // not null
  void fileSelected(core::shp_File);  // may be null

  void filesChanged();                // emited after corrFileSet, fileAdded, fileRemoved

  void datasetSelected(core::shp_Dataset);  // may be null

public:
  model::FileViewModel    fileViewModel;
  model::DatasetViewModel datasetViewModel;
};

#endif
