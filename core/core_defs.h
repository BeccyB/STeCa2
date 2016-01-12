// common and useful definitions

#ifndef CORE_DEFS_H
#define CORE_DEFS_H

#define DEVEL

#include <QtGlobal>
#include <QSharedPointer>

// strings
#include <QString>
#include <QStringBuilder>

typedef QString     str;
typedef str const&  rcstr;
typedef char const* pcstr;

typedef QStringList   str_lst;

// error handling
#include <QException>

class Exception: public QException {
public:
  Exception(rcstr msg_): msg(msg_) {}

  str msg;
};

#define THROWS throw (Exception)
#define THROW(msg) raiseError(msg)
#define RUNTIME_CHECK(test,msg)  if (!(test)) THROW(msg)

void raiseError(rcstr msg) THROWS;

// debug support
#include "core_debug.h"

#endif
