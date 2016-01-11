// common and useful definitions

#ifndef CORE_DEFS_H
#define CORE_DEFS_H

// #define DEVEL

#include <QtGlobal>

// strings
#include <QString>
#include <QStringBuilder>

typedef QString     str;
typedef str const&  rcstr;
typedef char const* pcstr;

typedef QStringList   str_lst;

#include <QException>

namespace core {

class Exception: public QException {
public:
  Exception(rcstr msg_): msg(msg_) {}

  str msg;
};

#define THROWS throw (Exception)

void raiseError(rcstr msg) THROWS;

#define THROW(msg) raiseError(msg);

}

#include "core_debug.h"

#endif
