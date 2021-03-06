// ************************************************************************** //
//
//  STeCa2:    StressTextureCalculator ver. 2
//
//! @file      typ_fun.h
//! @brief     Functions
//!
//! @homepage  http://apps.jcns.fz-juelich.de/steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016
//! @authors   Scientific Computing Group at MLZ Garching
//! @authors   Rebecca Brydon, Jan Burle,  Antti Soininen
//! @authors   Based on the original STeCa by Christian Randau
//
// ************************************************************************** //

#ifndef TYP_FUN_H
#define TYP_FUN_H

#include "def/defs.h"
#include "typ/typ_factory.h"
#include "typ/typ_json.h"
#include "typ/typ_range.h"

namespace typ {
//------------------------------------------------------------------------------
// Abstract function

class Function {
  CLS(Function)
public:
  class Factory : public typ::Factory<Function> {
    CLS(Factory) SUPER(typ::Factory<Function>)
  public:
    owner_not_null<Function*> make(JsonObj::rc) THROWS;
  };

protected:
  static Factory factory_;

public:
  static void addFactoryMaker(rcstr key, owner_not_null<Factory::MakerBase*>);
  static void initFactory();

  static owner_not_null<Function*> make(JsonObj::rc) THROWS;

public:
  class Parameter final {
    CLS(Parameter)
  public:
    Parameter();

    qreal value() const { return value_; }
    qreal error() const { return error_; }

    Range valueRange() const;  // allowed range of values
    void  setValueRange(qreal min, qreal max);

    void  setValue(qreal value, qreal error);

  public:
    JsonObj saveJson() const;
    void loadJson(JsonObj::rc) THROWS;

  protected:
    struct KEY { // for Json
      static str const
        VALUE, RANGE;
    };

  private:
    qreal value_, error_;

    // allowed range of values
    Range range_;
  };

public:
  Function();
  virtual ~Function() {}

  virtual uint parameterCount() const = 0;

  virtual Parameter&    parameterAt(uint) = 0;
  virtual Parameter::rc parameterAt(uint i) const {
    return const_cast<Function*>(this)->parameterAt(i);
  }

  // evaluate the function y = f(x), with given (parValues) or own parameters
  virtual qreal y(qreal x, qreal const* parValues = nullptr) const = 0;

  // partial derivative / parameter, with given (parValues) or own parameters
  virtual qreal dy(qreal x, uint parIndex,
                   qreal const* parValues = nullptr) const = 0;
public:
  virtual JsonObj saveJson() const;
  virtual void loadJson(JsonObj::rc) THROWS;

protected:
  struct KEY { // for Json
    static str const
      TYPE,
      SUM_FUNCTIONS;
  };
};

//------------------------------------------------------------------------------
// abstract function with parameters

class SimpleFunction : public Function {
  CLS(SimpleFunction) SUPER(Function)
public:
  SimpleFunction();

  void       setParameterCount(uint);
  uint       parameterCount() const;
  Parameter& parameterAt(uint);

  virtual void reset();

public:
  JsonObj saveJson() const;
  void loadJson(JsonObj::rc) THROWS;

protected:
  vec<Parameter> parameters_;
  qreal parValue(uint parIndex, qreal const* parValues) const;
  void  setValue(uint parIndex, qreal val);
};

//------------------------------------------------------------------------------
// concrete function that is a sum of other functions

class SumFunctions final : public Function {
  CLS(SumFunctions) SUPER(Function)
public:
  SumFunctions();
 ~SumFunctions();

  void addFunction(owner_not_null<Function*>);

  // aggregate parameter list for all added functions
  uint       parameterCount() const;
  Parameter& parameterAt(uint);

  qreal y(qreal x, qreal const* parValues = nullptr) const;
  qreal dy(qreal x, uint parIndex, qreal const* parValues = nullptr) const;

public:
  JsonObj saveJson() const;
  void loadJson(JsonObj::rc) THROWS;

protected:
  // summed functions
  vec<Function*> functions_;
  // the aggregate parameter list
  vec<Parameter*> allParameters_;
  // look up the original function for a given aggregate parameter index
  vec<Function*> function4parIndex_;
  // the starting index of parameters of a summed function, given the aggregate
  // parameter index
  uint_vec firstParIndex4parIndex_;
};

//------------------------------------------------------------------------------
}
#endif // TYP_FUN_H
