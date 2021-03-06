// ************************************************************************** //
//
//  STeCa2:    StressTextureCalculator ver. 2
//
//! @file      typ_hash.h
//! @brief     Veneered QT class.
//!
//! @homepage  http://apps.jcns.fz-juelich.de/steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016
//! @authors   Scientific Computing Group at MLZ Garching
//! @authors   Rebecca Brydon, Jan Burle,  Antti Soininen
//! @authors   Based on the original STeCa by Christian Randau
//
// ************************************************************************** //

#ifndef TYP_HASH_H
#define TYP_HASH_H

#include "def/def_macros.h"
#include "def/def_gsl.h"
#include <QHash>

namespace typ {
//------------------------------------------------------------------------------

template <typename Key, typename T>
class hash : protected QHash<Key,T> {
  CLS(hash) SUPER(QHash<Key COMMA T>)
public:
  using super::clear;
  using super::insert;
  using super::remove;
  using super::find;
  using super::contains;
  using super::value;
  using super::take;
};

template <typename Key, typename Tp>
class owning_hash : protected hash<Key,Tp> {
  CLS(owning_hash) SUPER(hash<Key COMMA Tp>)
public:
 ~owning_hash() {
    clear();
  }

  void clear() {
    for (auto* v : QHash<Key,Tp>::values())
      delete v;
    super::clear();
  }

  void insert(Key const& key, owner_not_null<Tp> t) {
    remove(key);
    super::insert(key, t);
  }

  bool remove(Key const& key) {
    Tp t = take(key);
    delete t;
    return nullptr != t;
  }

  Tp value(Key const& key) {
    return super::value(key);
  }

  using super::contains;

  owner<Tp> take(Key const& key) {
    return owner<Tp>(super::take(key));
  }
};

//------------------------------------------------------------------------------
}
#endif // TYP_MAP_H
