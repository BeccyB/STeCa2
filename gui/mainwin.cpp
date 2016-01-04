#include "mainwin.h"
#include "app.h"
#include "settings.h"
#include "split_files.h"
#include "split_info.h"
#include "panels/images.h"
#include "panels/image.h"
#include "panels/reflections.h"
#include "panels/diffractogram.h"

#include <QCloseEvent>
#include <QMenuBar>
#include <QSplitter>
#include <QStatusBar>

#include <QFileDialog>

MainWin::MainWin() {
  dataFilesDir = QDir::homePath();

  initActions();
  initMenus();
  initLayout();
  initStatus();
  connectActions();

  readSettings();
}

MainWin::~MainWin() {
}

void MainWin::initActions() {
  typedef QKeySequence QKey;

  auto action = [&](pcstr text, bool checkable, pcstr iconFile, QKey shortcut) {
    ASSERT(text)
    pcstr tip = text;
    auto act = new QAction(text,this);
    act->setToolTip(tip);
    act->setCheckable(checkable);
    if (iconFile && *iconFile) act->setIcon(QIcon(iconFile));
    act->setShortcut(shortcut);
    return act;
  };

  auto simple = [&](pcstr text, pcstr iconFile = nullptr, QKey shortcut = QKey::UnknownKey) {
    ASSERT(text)
    return action(text,false,iconFile,shortcut);
  };

  auto toggle = [&](pcstr text, pcstr iconFile = nullptr, QKey shortcut = QKey::UnknownKey) {
    return action(text,true,iconFile,shortcut);
  };

  Keys keys;

  actAddFiles           = simple("Add files...",          ":/icon/add",     keys.keyAddFiles);
  actRemoveFile         = simple("Remove selected file",  ":/icon/remove", keys.keyDeleteFile);
  actSetCorrectionFile  = simple("Set correction file...","",               keys.keySetCorrectionFile);
  actOpenSession        = simple("Open session...");
  actSaveSession        = simple("Save session...");

  actExportDiffractogramCurrent           = simple("Current only...");
  actExportDiffractogramAllSeparateFiles  = simple("All to separate files...");
  actExportDiffractogramSingleFile        = simple("All to a single file...");

  actExportImagesWithMargins              = simple("With margins...");
  actExportImagesWithoutMargins           = simple("Without margins...");

  actQuit  = simple("Quit",  "", QKey::Quit);

  actUndo  = simple("Undo",  "", QKey::Undo);
  actRedo  = simple("Redo",  "", QKey::Redo);
  actCut   = simple("Cut",   "", QKey::Cut);
  actCopy  = simple("Copy",  "", QKey::Copy);
  actPaste = simple("Paste", "", QKey::Paste);

  actViewFiles     = toggle("Files",     "", keys.keyViewFiles);
  actViewInfo      = toggle("Info",      "", keys.keyViewInfo);
  actViewStatusbar = toggle("Statusbar", "", keys.keyViewStatusbar);
#ifndef Q_OS_OSX
  actFullscreen    = toggle("Fullscreen", "", keys.keyFullscreen);
#endif
  actViewReset     = simple("Reset");

  actPreferences        = simple("Preferences...");
  actFitErrorParameters = simple("Fit error parameters...");

  actPdfManual  = simple("Pdf manual (German)");
  actAbout      = simple("About...");

  actReflectionPeak     = simple("Peak",      ":/icon/peak");
  actReflectionReflect  = simple("Reflect",   ":/icon/reflect");
  actReflectionWidth    = simple("Width",     ":/icon/width");
  actReflectionAdd      = simple("Width",     ":/icon/add");

  actImagesCombine      = simple("Combine...");
  actImagesLink         = simple("UpDown",    ":/icon/link");
  actImagesEye          = simple("UpDown",    ":/icon/eye");
  actImagesUpDown       = simple("UpDown",    ":/icon/updown");
  actImagesLeftRight    = simple("LeftRight", ":/icon/leftright");
  actImagesTurnRight    = simple("TurnRight", ":/icon/turnright");
  actImagesTurnLeft     = simple("TurnLeft",  ":/icon/turnleft");

  actBackgroundBackground = simple("Background",    ":/icon/background");
  actBackgroundEye        = simple("BackgroundEye", ":/icon/eye");
}

void MainWin::initMenus() {
  auto separator = [&]() {
    auto act = new QAction(this);
    act->setSeparator(true);
    return act;
  };

  auto *mbar = menuBar();

  menuFile = mbar->addMenu("&File");
  menuEdit = mbar->addMenu("&Edit");
  menuView = mbar->addMenu("&View");
  menuOpts = mbar->addMenu("&Options");
  menuHelp = mbar->addMenu("&Help");

  menuFile->addActions({
    actAddFiles, actRemoveFile,
    separator(),
    actSetCorrectionFile,
    separator(),
    actOpenSession, actSaveSession,
  });

  QMenu *menuExportDiffractograms = new QMenu("Export diffractograms");
  menuExportDiffractograms->addActions({
    actExportDiffractogramCurrent,
    actExportDiffractogramAllSeparateFiles,
    actExportDiffractogramSingleFile,
  });

  menuFile->addAction(separator());
  menuFile->addMenu(menuExportDiffractograms);

  QMenu *menuExportImages = new QMenu("Export images");
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
    actViewFiles, actViewInfo,
    separator(),
    actViewStatusbar,
  #ifndef Q_OS_OSX
    actFullscreen,
  #endif
    separator(),
    actViewReset,
  });

  menuOpts->addActions({
    actPreferences, actFitErrorParameters,
  });

  menuHelp->addActions({
    actPdfManual,
  #ifndef Q_OS_OSX // Mac puts About into the Apple menu
    separator(),
  #endif
    actAbout,
  });
}

void MainWin::initLayout() {
  auto spl0 = new QSplitter(Qt::Horizontal);
  spl0->setChildrenCollapsible(false);

  auto spl1 = new QSplitter(Qt::Vertical);
  spl1->setChildrenCollapsible(false);

  auto spl2 = new QSplitter(Qt::Horizontal);
  spl2->setChildrenCollapsible(false);

  auto spl3 = new QSplitter(Qt::Horizontal);
  spl3->setChildrenCollapsible(false);

  setCentralWidget(spl0);

  spl0->addWidget(new SplitFiles(*this));
  spl0->addWidget(spl1);
  spl0->addWidget(new SplitInfo(*this));

  spl1->addWidget(spl2);
  spl1->addWidget(spl3);

  spl2->addWidget(new panel::Images(*this));
  spl2->addWidget(new panel::Image(*this));
  spl3->addWidget(new panel::Reflections(*this));
  spl3->addWidget(new panel::Diffractogram(*this));
/*
  hSplitter->addWidget(new PanelImageList(*this));
  hSplitter->addWidget(new PanelImages(*this));

  hSplitter = new QSplitter(Qt::Horizontal);
  spl0->addWidget(hSplitter);

  auto w = new QWidget;
  auto v = vbox();
  w->setLayout(v);

  hSplitter->addWidget(w);
  v->addWidget(new PanelBackground(*this));
  v->addWidget(new PanelReflections(*this));

  auto panelDiffractogram = new PanelDiffractogram(*this);
  panelDiffractogram->setStretchFactors(1,0);
  hSplitter->addWidget(panelDiffractogram);
*/

//  auto h = hbox();
//  imageWidget->setLayout(h);
//  h->addWidget()

//  auto w = new QWidget;
//  setWidget(w);

//  auto v = vbox();
//  w->setLayout(v);

  /*
  // panes
  v1->addWidget(new Files());
  v1->addWidget(new Detector());
  v1->addWidget(new Background());
  v1->addWidget(new Reflections());

  auto *v2h = new HBox;
  v2->addLayout(v2h,1);

  v2h->addWidget(new Images());
  v2h->addWidget(new Image());

  v2h = new HBox;
  v2->addLayout(v2h,1);
  v2h->addWidget(new Diffractogram());

  v3->addWidget(new ImageInfo());
  v3->addWidget(new ReflectionInfo());
  v3->addWidget(new Normalization());

  auto *v3h = new HBox;
  v3->addLayout(v3h);

  v3h->addWidget(new PushButton("Pole figure..."));
  v3h->addWidget(new PushButton("Diagram..."));
*/
}

void MainWin::initStatus() {
  statusBar();
}

void MainWin::connectActions() {
  auto onTrigger = [&](QAction* action, void (MainWin::*fun)()) {
    QObject::connect(action, &QAction::triggered, this, fun);
  };

  auto onToggle = [&](QAction* action, void (MainWin::*fun)(bool)) {
    QObject::connect(action, &QAction::toggled, this, fun);
  };

  auto NOT_YET = [&](QAction* action) {
    QObject::connect(action, &QAction::triggered, []() { notYet(); });
  };

  onTrigger(actAddFiles,           &MainWin::addFiles);
  onTrigger(actSetCorrectionFile,  &MainWin::setCorrectionFile);

  NOT_YET(actOpenSession);
  NOT_YET(actSaveSession);

  NOT_YET(actExportDiffractogramCurrent);
  NOT_YET(actExportDiffractogramAllSeparateFiles);
  NOT_YET(actExportDiffractogramSingleFile);
  NOT_YET(actExportImagesWithMargins);
  NOT_YET(actExportImagesWithoutMargins);

  onTrigger(actQuit, &MainWin::close);

  NOT_YET(actUndo);
  NOT_YET(actRedo);
  NOT_YET(actCut);
  NOT_YET(actCopy);
  NOT_YET(actPaste);

  NOT_YET(actPreferences);
  NOT_YET(actFitErrorParameters);

  NOT_YET(actPdfManual);
  NOT_YET(actAbout);

  onToggle(actViewFiles,     &MainWin::viewFiles);
  onToggle(actViewInfo,      &MainWin::viewInfo);
  onToggle(actViewStatusbar, &MainWin::viewStatusbar);
#ifndef Q_OS_OSX
  onToggle(actFullscreen,    &MainWin::viewFullscreen);
#endif
  onTrigger(actViewReset,    &MainWin::viewReset);
}

void MainWin::show() {
  super::show();
  checkActions();
}

void MainWin::close() {
  super::close();
}

void MainWin::addFiles() {
  str_lst fileNames = QFileDialog::getOpenFileNames(this,
      "Add files", dataFilesDir, "Data files (*.dat);;All files (*.*)");

  if (!fileNames.isEmpty()) {
    // remember the directory for the next time
    dataFilesDir = QFileInfo(fileNames.first()).canonicalPath();
    app->coreProxy.addFiles(fileNames);
  }
}

void MainWin::setCorrectionFile() {
  str fileName = QFileDialog::getOpenFileName(this,
      "Set correction file", dataFilesDir, "Data files (*.dat);;All files (*.*)");

  if (!fileName.isEmpty()) {
    // remember the directory for the next time
    dataFilesDir = QFileInfo(fileName).canonicalPath();
    app->coreProxy.setCorrectionFile(fileName);
  }
}

void MainWin::closeEvent(QCloseEvent* event) {
  if (onClose())
    event->accept();
  else
    event->ignore();
}

bool MainWin::onClose() {
  saveSettings();
  return true;
}

static str const GROUP_MAINWIN("MainWin");
static str const VALUE_GEOMETRY("geometry");
static str const VALUE_STATE("state");

void MainWin::readSettings() {
  if (initialState.isEmpty()) initialState = saveState();
  Settings s;
  s.beginGroup(GROUP_MAINWIN);
  restoreGeometry(s.value(VALUE_GEOMETRY).toByteArray());
  restoreState(s.value(VALUE_STATE).toByteArray());
  s.endGroup();
}

void MainWin::saveSettings() {
  Settings s;
  s.beginGroup(GROUP_MAINWIN);
  s.setValue(VALUE_GEOMETRY,	saveGeometry());
  s.setValue(VALUE_STATE,		saveState());
  s.endGroup();
}

void MainWin::checkActions() {
//  actViewFiles->setChecked(filesDock->isVisible());
//  actViewInfo->setChecked(infoDock->isVisible());
  actViewStatusbar->setChecked(statusBar()->isVisible());
#ifndef Q_OS_OSX
  actFullscreen->setChecked(isFullScreen());
#endif
}

void MainWin::viewFiles(bool on) {
//  filesDock->setVisible(on);
  actViewFiles->setChecked(on);
}

void MainWin::viewInfo(bool on) {
//  infoDock->setVisible(on);
  actViewInfo->setChecked(on);
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
  viewFiles(true);
  viewInfo(true);
  viewStatusbar(true);
  viewFullscreen(false);
}

// eof
