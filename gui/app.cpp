#include "app.h"
#include "../manifest.h"
#include "mainwin.h"
#include <QStyleFactory>
#include <QMessageBox>
#include <iostream>

//------------------------------------------------------------------------------

App::App(int &argc, char *argv[]): super(argc,argv) {
  setApplicationName(APPLICATION_NAME);
  setApplicationVersion(APPLICATION_VERSION);
  setOrganizationName(ORGANIZATION_NAME);
  setOrganizationDomain(ORGANIZATION_DOMAIN);

#if defined(Q_OS_OSX)
  setStyle(QStyleFactory::create("Macintosh"));
#elif defined(Q_OS_WIN)
  setStyle(QStyleFactory::create("Windows"));
#else
  setStyle(QStyleFactory::create("Fusion"));
#endif
}

static QtMessageHandler oldHandler;
static MainWin *pMainWin;

static void messageHandler(QtMsgType type, QMessageLogContext const& ctx, rcstr msg) {
  switch (type) {
  case QtDebugMsg:
    std::cerr << "TR " << msg.toStdString() // TR for TRace
              << "\t[" << ctx.function << ']' << std::endl;
    break;
  case QtWarningMsg:
    QMessageBox::warning(pMainWin, qAppName(), msg);
    break;
  default:
    oldHandler(type,ctx,msg);
    break;
  }
}

// TODO icon
int App::exec() {
  MainWin mainWin;
  mainWin.show();

  pMainWin = &mainWin;
  oldHandler = qInstallMessageHandler(messageHandler);
  int res = super::exec();
  qInstallMessageHandler(nullptr);

  return res;
}

bool App::notify(QObject* receiver, QEvent* event) {
  try {
    return super::notify(receiver, event);
  } catch(Exception const& e) {
    qWarning("%s", e.msg.toLocal8Bit().constData());
  } catch(std::exception const& e) {
    qWarning("Error: %s", e.what());
  }

  return false;
}

//------------------------------------------------------------------------------
// eof
