/** \file
 */

#ifndef THEHUB_H
#define THEHUB_H

#include "core_defs.h"
#include <QAction>
#include "models.h"
#include "core_session.h"

//------------------------------------------------------------------------------

class Action: public QAction {
  SUPER(Action,QAction)
public:
  Action(rcstr text, rcstr tip, rcstr iconFile, QObject*);
};

class PushAction: public Action {
  SUPER(PushAction,Action)
public:
  PushAction(rcstr text, rcstr tip, rcstr iconFile, QObject*);
};

class ToggleAction: public Action {
  SUPER(ToggleAction,Action)
public:
  ToggleAction(rcstr text, rcstr tip, rcstr iconFile, QObject*);
};

//------------------------------------------------------------------------------

class TheHub: public QObject {
  SUPER(TheHub,QObject) Q_OBJECT
public:
  TheHub();
 ~TheHub();

private:
  void initActions();
  void configActions();

private:
  core::Session *session;

public:
  bool globalNorm;  // TODO rename this and related to fixedIntensityScale

  model::FileViewModel       fileViewModel;
  model::DatasetViewModel    datasetViewModel;
  model::ReflectionViewModel reflectionViewModel;

public:
  QAction
    *actAddFiles, *actRemoveFile,
    *actLoadCorrectionFile,
    *actLoadSession, *actSaveSession,

    *actExportDiffractogramCurrent,
    *actExportDiffractogramAllSeparateFiles,
    *actExportDiffractogramSingleFile,
    *actExportImagesWithMargins,
    *actExportImagesWithoutMargins,

    *actQuit,

    *actUndo, *actRedo,
    *actCut,  *actCopy, *actPaste,

    *actViewStatusbar,
#ifndef Q_OS_OSX // Mac has its own
    *actFullscreen,
#endif
    *actViewReset,

    *actReflectionSelectRegion,
    *actCalculatePolefigures,
    *actCalculateHistograms,
    *actPreferences,
    *actFitErrorParameters,

    *actAbout,

  // more actions, some not in the menu
    *actSelectPeak, *actReflectionPeak, *actReflectionWidth, *actReflectionAdd, *actReflectionRemove,
    *actImageRotate, *actImageMirror,
    *actImagesLink, *actImageOverlay, *actImagesGlobalNorm, *actImagesEnableCorr,
    *actBackgroundClear, *actBackgroundBackground, *actBackgroundShowFit,
    *actHasBeamOffset,
    *actNormalizationDisable, *actNormalizationMeasureTime, *actNormalizationMonitor, *actNormalizationBackground;

public: // files
  uint numFiles(bool withCorr) const;
  bool hasCorrFile()           const;
  bool isCorrFile(uint index)  const;
  str  fileName(uint index)    const;
  str  filePath(uint index)    const;

  core::shp_File getFile(uint i)  const;
  void remFile(uint i);

  void setSelectedFile(core::shp_File);
  void setSelectedDataset(core::shp_Dataset);

public:
  void load(QFileInfo const&)       THROWS;
  void load(QByteArray const& json) THROWS;
  QByteArray save() const;

  void addFile(rcstr filePath)      THROWS;
  void addFiles(str_lst filePaths)  THROWS;
  void loadCorrFile(rcstr filePath);

  void enableCorrection(bool);

  core::ImageCut const& getImageCut() const;
  void setImageCut(bool topLeft, bool linked, core::ImageCut const&);

  core::Geometry const& getGeometry() const;
  void setGeometry(qreal sampleDetectorSpan, qreal pixSpan, bool hasBeamOffset, QPoint const& middlePixOffset);

  void setBackgroundPolynomialDegree(uint);

private:
  void setImageRotate(core::ImageTransform);

signals:
  void filesChanged();
  void correctionEnabled(bool);

  void fileSelected(core::shp_File);
  void datasetSelected(core::shp_Dataset);

  void displayChange();
  void geometryChanged();

  void backgroundPolynomialDegree(uint);
};

//------------------------------------------------------------------------------
#endif
