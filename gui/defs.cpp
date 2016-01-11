#include "defs.h"
#include <QDebug>
#include <QMessageBox>
#include <QApplication>

static_assert (QT_VERSION >= 0x050501,"written for Qt >= 5.5");

//------------------------------------------------------------------------------

#ifdef QT_NO_EXCEPTIONS
#error needs exception handling
#endif

Exc::Exc(rcstr msg_): msg(msg_) {
}

Exc::~Exc() throw () {
}

void Exc::warn() const {
  qWarning() << msg;
}

void CRITICAL_ERROR(rcstr msg) throw (CriticalError) {
  qCritical() << "Critical: " << msg; // TODO test
  CriticalError(msg).raise();
}

//------------------------------------------------------------------------------

void warn(QObject* parent, rcstr msg) {
  QMessageBox::warning(dynamic_cast<QWidget*>(parent), qAppName(), msg);
}

void warn(QObject* parent, rcstr msg, rcstr more) {
  str s(msg);
  if (!more.isEmpty()) s += ": " + more;
  QMessageBox::warning(dynamic_cast<QWidget*>(parent), qAppName(), s);
}

// eof
