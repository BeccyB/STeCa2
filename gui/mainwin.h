/** \file
 */

#ifndef MAINWIN_H
#define MAINWIN_H

#include "defs.h"
#include <QMainWindow>

class Session;
class BoxWidget;

class MainWin: public QMainWindow {
  SUPER(MainWin,QMainWindow) Q_OBJECT
public:
  MainWin();
 ~MainWin();

private:
  void initActions();
  void initMenus();
  void initLayout();
  void initStatus();
  void connectActions();

public:
  void show();
  void close();

  void addFiles();
  void loadCorrectionFile();

  void loadSession();
  void saveSession();

private:
  // where to look for files
  str  sessionDir, dataDir;

private:
  void closeEvent(QCloseEvent*);
  bool onClose();

private:
  Session *session;
  QMenu   *menuFile, *menuEdit, *menuView, *menuImage, *menuOpts, *menuHelp;

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

    *actPreferences,
    *actFitErrorParameters,

    *actPdfManual,
    *actAbout,

  // more actions, some not in the menu
    *actReflectionPeak, *actReflectionReflect, *actReflectionWidth, *actReflectionAdd,
    *actImagesLink, *actImageOverlay, *actImagesGlobalNorm, *actImagesShowRaw,
    *actBackgroundBackground, *actBackgroundEye,
    *actHasBeamOffset;

private:
  BoxWidget *splitFiles, *splitImage, *splitReflections, *splitDiffractogram;

private:
  QByteArray initialState;

  void readSettings();
  void saveSettings();

  void checkActions();

  void viewStatusbar(bool);
  void viewFullscreen(bool);
  void viewReset();
};

#endif
