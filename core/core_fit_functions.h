/** \file
 * Functions for data fitting (was: approximation)
 */

#ifndef CORE_FIT_FUNCTIONS_H
#define CORE_FIT_FUNCTIONS_H

#include <core_types.h>

namespace core { namespace fit {
//------------------------------------------------------------------------------
/// Abstract function

class Function {
public:
  class Parameter final {
  public:
    Parameter();

    qreal getValue() const  { return value; }
    Range getRange() const; ///< the allowed range

    void setRange(qreal min,qreal max);

    /// checks whether a new value/error pair would pass the constraints
    bool  checkValue(qreal value, qreal error=0);
    /// conditionally sets the new value/error pair
    bool  setValue(qreal value, qreal error=0, bool force=false);

  private:
    qreal value;

    // constraints; TODO maybe not all needed?

    /// allowed range of values
    /// if !isValid() -> means the same as <value,value>, i.e. fixed value
    Range range;

    /// maximum change allowed; NaN -> no check
    qreal maxDelta;
    qreal maxDeltaPercent;

    /// maximum error allowed; NaN -> no check
    qreal maxError;
    qreal maxErrorPercent;
  };

public:
  Function();
  virtual ~Function();

  virtual uint parameterCount() const   = 0;

  virtual Parameter& getParameter(uint) = 0;
  virtual Parameter const& getParameter(uint i) const {
    return const_cast<Function*>(this)->getParameter(i);
  }

  /// evaluate the function y = f(x), with given (parameterValues) or own parameters
  virtual qreal y(qreal x, qreal const* parameterValues = nullptr) const = 0;

  /// partial derivative / parameter, with given (parameterValues) or own parameters
  virtual qreal dy(qreal x, uint parameterIndex, qreal const* parameterValues = nullptr) const = 0;
};

#ifndef QT_NO_DEBUG
// debug prints
QDebug& operator<<(QDebug&,Function const&);
#endif

//------------------------------------------------------------------------------
/// abstract function with parameters

class SimpleFunction: public Function {
  SUPER(SimpleFunction,Function)
public:
  SimpleFunction();

  void setParameterCount(uint);
  uint parameterCount() const;
  Parameter& getParameter(uint);

protected:
  QVector<Parameter> parameters;
  qreal parValue(uint parIndex, qreal const* parameterValues) const;
  void  setValue(uint parIndex, qreal val);
};

//------------------------------------------------------------------------------
/// concrete function that is a sum of other functions

class SumFunctions final: public Function {
  SUPER(SumFunctions,Function)
public:
  SumFunctions();
 ~SumFunctions();

  /// add a (new) Function instance, take its ownership (for delete)
  void addFunction(Function*);

  /// aggregate parameter list for all added functions
  uint parameterCount() const;
  Parameter& getParameter(uint);

  qreal y(qreal x, qreal const* parameterValues = nullptr) const;
  qreal dy(qreal x, uint parameterIndex, qreal const* parameterValues = nullptr) const;

protected:
  /// summed functions
  QVector<Function*>  functions;
  /// the aggregate parameter list
  QVector<Parameter*> parameters;
  /// look up the original function for a given aggregate parameter index
  QVector<Function*>  function_parIndex;
  /// the starting index of parameters of a summed function, given the aggregate parameter index
  QVector<uint> firstParIndex_parIndex;
};

//------------------------------------------------------------------------------
/// a polynomial

class Polynomial: public SimpleFunction {
  SUPER(Polynomial,SimpleFunction)
public:
  Polynomial(uint degree = 0);

  uint getDegree() const;
  void setDegree(uint);

  qreal y(qreal x, qreal const* parameterValues = nullptr) const;
  qreal dy(qreal x, uint parameterIndex, qreal const* parameterValues = nullptr) const;
};

//------------------------------------------------------------------------------
/// Abstract peak function

class PeakFunction: public SimpleFunction {
  SUPER(PeakFunction,SimpleFunction)
public:
  PeakFunction();

public:
//  virtual qreal getArea()                           = 0;
//  virtual qreal getAreaErrorAbsolute()              = 0;
//  virtual qreal getAreaErrorRelativInPercent()      = 0;
//  virtual void  setFWHM(qreal value)                = 0;
//  virtual void  setFWHMIsFixed(bool isFixed)        = 0;
//  virtual void  setFWHMAbsoluteLimit(qreal min, qreal max) = 0;
//  virtual qreal getFWHM()                           = 0;
//  virtual qreal getFWHMErrorAbsolute()              = 0;
//  virtual qreal getFWHMErrorRelativInPercent()      = 0;
//  virtual bool  getFWHMIsFixed()                    = 0;
//  virtual qreal getFWHMAbsoluteLimitMin()           = 0;
//  virtual qreal getFWHMAbsoluteLimitMax()           = 0;
//  virtual void  setMax(qreal value)                 = 0;
//  virtual void  setMaxIsFixed(bool)                 = 0;
//  virtual void  setMaxAbsoluteLimit(qreal min, qreal max) = 0;
//  virtual qreal getMax()                            = 0;
//  virtual qreal getMaxErrorAbsolute()               = 0;
//  virtual qreal getMaxErrorRelativInPercent()       = 0;
//  virtual bool  getMaxIsFixed()                     = 0;
//  virtual qreal getMaxAbsoluteLimitMin()            = 0;
//  virtual qreal getMaxAbsoluteLimitMax()            = 0;
//  virtual void  setPositionOfMax(qreal)             = 0;
//  virtual void  setPositionOfMaxIsFixed(bool)       = 0;
//  virtual void  setPositionOfMaxAbsoluteLimit(qreal min, qreal max) = 0;
//  virtual qreal getPositionOfMax()                  = 0;
//  virtual qreal getPositionOfMaxErrorAbsolute()     = 0;
//  virtual qreal getPositionOfMaxErrorRelativInPercent() = 0;
//  virtual bool  getPositionOfMaxIsFixed()           = 0;
//  virtual qreal getPositionOfMaxAbsoluteLimitMin()  = 0;
//  virtual qreal getPositionOfMaxAbsoluteLimitMax()  = 0;
};

//------------------------------------------------------------------------------

class Gaussian: public PeakFunction {
  SUPER(Gaussian,PeakFunction)
public:
  enum { parAMPL, parMU, parSIGMA };

  Gaussian(qreal ampl=1, qreal mu=0, qreal sigma=1);

  qreal y(qreal x, qreal const* parameterValues = nullptr) const;
  qreal dy(qreal x, uint parameterIndex, qreal const* parameterValues = nullptr) const;

  void setPeak(qreal tth, qreal intens);
  void setFWHM(qreal);
};

//------------------------------------------------------------------------------
}}
#endif