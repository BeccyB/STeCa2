#include "mainwin.h"
#include "session.h"
#include "settings.h"
#include "mainwin_parts.h"

#include <QCloseEvent>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>

#include <QFileDialog>
#include <QDir>
#include <QAction>

#ifdef DEVELOPMENT_REBECCA
#include "io/core_io.h"
#endif
//------------------------------------------------------------------------------

MainWin::MainWin(): session(new Session(*this)){
  sessionDir = dataDir = QDir::homePath();

  initActions();
  initMenus();
  initLayout();
  initStatus();
  connectActions();

  readSettings();
  session->doReadSettings();
}

MainWin::~MainWin() {
  delete session;
}

void MainWin::initActions() {
  typedef QKeySequence QKey;

  auto action = [this](pcstr text, pcstr tip, bool checkable, pcstr iconFile, QKey shortcut) {
    ASSERT(text)
    ASSERT(tip)
    auto act = new QAction(text,this);
    act->setToolTip(tip);
    act->setCheckable(checkable);
    if (iconFile && *iconFile) act->setIcon(QIcon(iconFile));
    act->setShortcut(shortcut);
    return act;
  };

  auto simple = [&action](pcstr text, pcstr tip = "", pcstr iconFile = nullptr, QKey shortcut = QKey::UnknownKey) {
    ASSERT(text)
    return action(text,tip,false,iconFile,shortcut);
  };

  auto toggle = [&action](pcstr text, pcstr tip = "", pcstr iconFile = nullptr, QKey shortcut = QKey::UnknownKey) {
    return action(text,tip,true,iconFile,shortcut);
  };

  Keys keys;

  actAddFiles           = simple("Add files...",            "Add files", ":/icon/add", keys.keyAddFiles);
  actRemoveFile         = simple("Remove selected file",    "Remove selected file", ":/icon/rem", keys.keyDeleteFile);
  actLoadCorrectionFile = simple("Load correction file...", "Load correction file", "",          keys.keyLoadCorrectionFile);
  actImagesEnableCorr   = toggle("Enable correction file",  "Enable correction by correction file", ":/icon/eye");   // TODO different icon
  actLoadSession        = simple("Load session...",         "Test");
  actSaveSession        = simple("Save session...",         "Test");

  actExportDiffractogramCurrent           = simple("Current only...");
  actExportDiffractogramAllSeparateFiles  = simple("All to separate files...");
  actExportDiffractogramSingleFile        = simple("All to a single file...");

  actExportImagesWithMargins              = simple("With margins...");
  actExportImagesWithoutMargins           = simple("Without margins...");

  actQuit  = simple("Quit", "",  "", QKey::Quit);

  actUndo  = simple("Undo",  "", "", QKey::Undo);
  actRedo  = simple("Redo",  "", "", QKey::Redo);
  actCut   = simple("Cut",   "", "", QKey::Cut);
  actCopy  = simple("Copy",  "", "", QKey::Copy);
  actPaste = simple("Paste", "", "", QKey::Paste);

  actViewStatusbar = toggle("Statusbar",  "", "", keys.keyViewStatusbar);
#ifndef Q_OS_OSX
  actFullscreen    = toggle("Fullscreen", "", "", keys.keyFullscreen);
#endif
  actViewReset     = simple("Reset", "");

  actPreferences          = simple("Preferences...");
  actFitErrorParameters   = simple("Fit error parameters...");


  actAbout      = simple("About...");

  actReflectionPeak       = simple("Peak",    "Peak",      ":/icon/peak");
  actReflectionReflect    = simple("Reflect", "Reflect",      ":/icon/reflect");
  actReflectionWidth      = simple("Width",   "Width",      ":/icon/width");
  actReflectionAdd        = simple("Width",   "Width",      ":/icon/add");

  actImagesLink           = toggle("Link",         "Use the same value for all cuts", ":/icon/link");
  actImageOverlay         = toggle("overlay",      "Show cut", ":/icon/eye");
  actImagesGlobalNorm     = toggle("global norm.", "Display data using a global intensity scale", ":/icon/eye");    // TODO different icon
  actImageRotate          = simple("Rotate",       "Rotate 90° clockwise", ":/icon/rotate0", keys.keyRotateImage);
  actImageMirror          = toggle("Mirror",       "Mirror image", ":/icon/mirror_horz");

  actBackgroundBackground = toggle("Background",    "Select regions for background fitting", ":/icon/background");
  actBackgroundEye        = simple("BackgroundEye", "Show background fit", ":/icon/eye");

  actHasBeamOffset        = toggle("Beam centre offset", "Enable beam center offset (for X-ray instruments)", ":/icon/eye"); // TODO icon

  // TODO where to best put these actions updates?
  connect(session, &Session::corrFileSet, [this](core::shp_File file) {
    bool on  = !file.isNull();
    auto act = actImagesEnableCorr;
    act->setChecked(on);
    act->setEnabled(on);
  });

  connect(actImageMirror, &QAction::toggled, [this](bool on) {
    session->setImageMirror(on);
  });

  connect(actImageRotate, &QAction::triggered, [this]() {
    session->nextImageRotate();
  });
}

void MainWin::initMenus() {
  auto separator = [this]() {
    auto act = new QAction(this);
    act->setSeparator(true);
    return act;
  };

  auto *mbar = menuBar();

  menuFile  = mbar->addMenu("&File");
  menuEdit  = mbar->addMenu("&Edit");
  menuView  = mbar->addMenu("&View");
  menuImage = mbar->addMenu("&Image");
  menuOpts  = mbar->addMenu("&Options");
  menuHelp  = mbar->addMenu("&Help");

  menuFile->addActions({
    actAddFiles, actRemoveFile,
    separator(),
    actLoadCorrectionFile,
    separator(),
    actLoadSession, actSaveSession,actLoadSession
  });

  QMenu *menuExportDiffractograms = new QMenu("Export diffractograms",this);
  menuExportDiffractograms->addActions({
    actExportDiffractogramCurrent,
    actExportDiffractogramAllSeparateFiles,
    actExportDiffractogramSingleFile,
  });

  menuFile->addAction(separator());
  menuFile->addMenu(menuExportDiffractograms);

  QMenu *menuExportImages = new QMenu("Export images",this);
  menuExportImages->addActions({
    actExportImagesWithMargins,
    actExportImagesWithoutMargins,
  });

  menuFile->addMenu(menuExportImages);

  menuFile->addActions({
  #ifndef Q_OS_OSX  // Mac puts Quit into the Apple menu
    separator(),
  #endif
    actQuit,
  });

  menuEdit->addActions({
    actUndo, actRedo,
    separator(),
    actCut, actCopy, actPaste,
  });

  menuView->addActions({
    actViewStatusbar,
  #ifndef Q_OS_OSX
    actFullscreen,
  #endif
    separator(),
    actViewReset,
  });

  menuImage->addActions({
    actImageRotate, actImageMirror,
  });

  menuOpts->addActions({
    actPreferences, actFitErrorParameters,
  });

  menuHelp->addActions({
  #ifndef Q_OS_OSX // Mac puts About into the Apple menu
    separator(),
  #endif
    actAbout,
  });
}

void MainWin::initLayout() {
  addDockWidget(Qt::LeftDockWidgetArea, new panel::DockFiles(*this,*session));
  addDockWidget(Qt::LeftDockWidgetArea, new panel::DockDatasets(*this,*session));
  addDockWidget(Qt::RightDockWidgetArea,new panel::DockDatasetInfo(*this,*session));

  auto splMain = new QSplitter(Qt::Vertical);
  splMain->setChildrenCollapsible(false);
  setCentralWidget(splMain);

  auto splImages = new QSplitter(Qt::Horizontal);
  splImages->setChildrenCollapsible(false);

  auto splReflections = new QSplitter(Qt::Horizontal);
  splReflections->setChildrenCollapsible(false);

  splMain->addWidget(splImages);
  splMain->addWidget(splReflections);

  splImages->addWidget(new panel::SplitImage(*this,*session));

  splReflections->addWidget(new panel::SplitFitting(*this,*session));
  splReflections->addWidget(new panel::SplitDiffractogram(*this,*session));
  splReflections->setStretchFactor(0,1);
  splReflections->setStretchFactor(1,3);
}

void MainWin::initStatus() {
  statusBar();
}

void MainWin::connectActions() {
  auto onTrigger = [this](QAction* action, void (thisCls::*fun)()) {
    QObject::connect(action, &QAction::triggered, this, fun);
  };

  auto onToggle = [this](QAction* action, void (thisCls::*fun)(bool)) {
    QObject::connect(action, &QAction::toggled, this, fun);
  };

  auto notYet = [this](QAction* action) {
    action->setEnabled(false);
  };

  onTrigger(actAddFiles, &thisCls::addFiles);
  actRemoveFile->setEnabled(false);
  connect(session, &Session::fileSelected, this, [this](core::shp_File file) {
    actRemoveFile->setEnabled(nullptr!=file);
  });

  onTrigger(actLoadCorrectionFile,  &thisCls::loadCorrectionFile);

  onTrigger(actLoadSession, &thisCls::loadSession);
  onTrigger(actSaveSession, &thisCls::saveSession);

  notYet(actExportDiffractogramCurrent);
  notYet(actExportDiffractogramAllSeparateFiles);
  notYet(actExportDiffractogramSingleFile);
  notYet(actExportImagesWithMargins);
  notYet(actExportImagesWithoutMargins);

  onTrigger(actQuit, &thisCls::close);

  notYet(actUndo);
  notYet(actRedo);
  notYet(actCut);
  notYet(actCopy);
  notYet(actPaste);

  notYet(actPreferences);
  notYet(actFitErrorParameters);

  notYet(actAbout);

  onToggle(actViewStatusbar, &thisCls::viewStatusbar);
#ifndef Q_OS_OSX
  onToggle(actFullscreen,    &thisCls::viewFullscreen);
#endif
  onTrigger(actViewReset,    &thisCls::viewReset);
}

void MainWin::show() {
  super::show();
  checkActions();

#ifdef DEVELOPMENT_JAN
#ifdef Q_OS_OSX
  session->load(QFileInfo("/Users/igb/P/+scg/STeCa/data/q.ste"));
#else
  session->load(QFileInfo("/home/jan/q.ste"));
#endif
#endif

#ifdef DEVELOPMENT_REBECCA
  session->load(QFileInfo("/home/rebecca/SCG/STeCa-Data/2.ste"));
  //core::io::saveTiffs(*session->getFile(0),"/home/rebecca/SCG/STeCa-Data/savedtiffs/test");
#endif
}

void MainWin::close() {
  super::close();
}

void MainWin::addFiles() {
  str_lst fileNames = QFileDialog::getOpenFileNames(this,
    "Add files", dataDir, "Data files (*.dat);;All files (*.*)");

  if (!fileNames.isEmpty()) {
    // remember the directory for the next time
    dataDir = QFileInfo(fileNames.first()).absolutePath();
    session->addFiles(fileNames);
  }
}

void MainWin::loadCorrectionFile() {
  str fileName = QFileDialog::getOpenFileName(this,
      "Set correction file", dataDir, "Data files (*.dat);;All files (*.*)");

  if (!fileName.isEmpty()) {
    // remember the directory for the next time
    dataDir = QFileInfo(fileName).absolutePath();
    session->loadCorrFile(fileName);
  }
}

static str const STE(".ste");

void MainWin::loadSession() {
  str fileName = QFileDialog::getOpenFileName(this,
      "Load session", sessionDir, "Session files (*"%STE%");;All files (*.*)");
  if (fileName.isEmpty()) return;

  QFileInfo fileInfo(fileName);
  sessionDir = fileInfo.absolutePath();
  session->load(fileInfo);
}

void MainWin::saveSession() {
  str fileName = QFileDialog::getSaveFileName(this,
      "Save session", sessionDir, "Session files (*"%STE%");;All files (*.*)");
  if (fileName.isEmpty()) return;

  if (!fileName.endsWith(STE)) fileName += STE;
  QFileInfo info(fileName);
  sessionDir = info.absolutePath();

  QFile file(info.filePath());
  RUNTIME_CHECK(file.open(QIODevice::WriteOnly), "File cannot be opened");

  auto written = file.write(session->save());
  RUNTIME_CHECK(written >= 0, "Could not write session");
}

void MainWin::closeEvent(QCloseEvent* event) {
  onClose();
  event->accept();
}

void MainWin::onClose() {
  session->doSaveSettings();
  saveSettings();
}

static str GROUP_MAINWIN("MainWin");
static str KEY_GEOMETRY("geometry");
static str KEY_STATE("state");

void MainWin::readSettings() {
  if (initialState.isEmpty()) initialState = saveState();
  Settings s(GROUP_MAINWIN);
  restoreGeometry(s.value(KEY_GEOMETRY).toByteArray());
  restoreState(s.value(KEY_STATE).toByteArray());
}

void MainWin::saveSettings() {
  Settings s(GROUP_MAINWIN);
  s.setValue(KEY_GEOMETRY, saveGeometry());
  s.setValue(KEY_STATE,    saveState());
}

void MainWin::checkActions() {
  actViewStatusbar->setChecked(statusBar()->isVisible());
#ifndef Q_OS_OSX
  actFullscreen->setChecked(isFullScreen());
#endif
}

void MainWin::viewStatusbar(bool on) {
  statusBar()->setVisible(on);
  actViewStatusbar->setChecked(on);
}

void MainWin::viewFullscreen(bool on) {
  if (on) showFullScreen(); else showNormal();
#ifndef Q_OS_OSX
  actFullscreen->setChecked(on);
#endif
}

void MainWin::viewReset() {
  restoreState(initialState);
  viewStatusbar(true);
  viewFullscreen(false);
}

//------------------------------------------------------------------------------
// eof
