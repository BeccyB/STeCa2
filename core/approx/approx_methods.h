#ifndef APPROX_METHODS_H
#define APPROX_METHODS_H

#include "approx_functions.h"

namespace core { namespace approx {

class FittingMethod {
public:
  FittingMethod();
  virtual ~FittingMethod();

  bool fitWithoutCheck(Function&,Curve&);

protected:
  bool fit(Function&,Curve&, bool sideConditionCheckIsActive);
  virtual bool approximate(qreal*,qreal const*,qreal const*,qreal*,uint,qreal const*,qreal const*,uint) = 0;

  Function *function; ///< valid during fit()

protected:
  void __functionY(qreal*,qreal*,int,int,void*);
};

class FittingLinearLeastSquare: public FittingMethod {
  SUPER(FittingLinearLeastSquare,FittingMethod)
public:
  FittingLinearLeastSquare();

protected:
  bool approximate(qreal*,qreal const*,qreal const*,qreal*,uint,qreal const*,qreal const*,uint);
};

class FittingLevenbergMarquardt: public FittingLinearLeastSquare {
  SUPER(FittingLevenbergMarquardt,FittingLinearLeastSquare)
public:
  FittingLevenbergMarquardt();

protected:
  bool approximate(qreal*,qreal const*,qreal const*,qreal*,uint,qreal const*,qreal const*,uint);

private:
  void __functionJacobianLM(qreal*,qreal*,int,int,void*);
};

}}

#endif
