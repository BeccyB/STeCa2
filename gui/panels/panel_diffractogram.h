// ************************************************************************** //
//
//  STeCa2:    StressTextureCalculator ver. 2
//
//! @file      panel_diffractogram.h
//! @brief     File selection panel.
//!
//! @homepage  http://apps.jcns.fz-juelich.de/steca2
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @copyright Forschungszentrum Jülich GmbH 2016
//! @authors   Scientific Computing Group at MLZ Garching
//! @authors   Rebecca Brydon, Jan Burle,  Antti Soininen
//! @authors   Based on the original STeCa by Christian Randau
//
// ************************************************************************** //

#ifndef PANEL_DIFFRACTOGRAM_H
#define PANEL_DIFFRACTOGRAM_H

#include "QCP/qcustomplot.h"
#include "data/data_dataset.h"
#include "fit/fit_methods.h"
#include "calc/calc_reflection.h"
#include "panel.h"
#include "typ/typ_curve.h"

namespace gui { namespace panel {
//------------------------------------------------------------------------------

class DiffractogramPlot;

class DiffractogramPlotOverlay : public QWidget {
  CLS(DiffractogramPlotOverlay) SUPER(QWidget)
public:
  DiffractogramPlotOverlay(DiffractogramPlot&);

  void setMargins(int left, int right);

private:
  DiffractogramPlot &plot_;

  QColor addColor_, remColor_, color_, bgColor_, reflColor_;
  int    marginLeft_, marginRight_;

protected:
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);

  void paintEvent(QPaintEvent*);

  bool hasCursor_, mouseDown_;
  int  cursorPos_, mouseDownPos_;

  void updateCursorRegion();
};

class DiffractogramPlot : public QCustomPlot, protected RefHub {
  CLS(DiffractogramPlot) SUPER(QCustomPlot)
public:
  enum class eTool {
    NONE,
    BACKGROUND,
    PEAK_REGION,
  };

  DiffractogramPlot(TheHub&, class Diffractogram&);

public:
  void  setTool(eTool);
  eTool getTool() const { return tool_; }

  void  plot(typ::Curve::rc, typ::Curve::rc, typ::Curve::rc,
             typ::curve_vec::rc, uint);

  typ::Range fromPixels(int, int);

  void clearBg();
  void addBg(typ::Range::rc);
  void remBg(typ::Range::rc);
  void setNewReflRange(typ::Range::rc);
  void updateBg();

  void clearReflLayer();

  QColor bgRgeColor_, reflRgeColor_;
  eFittingTab selectedFittingTab();

protected:
  void addBgItem(typ::Range::rc);
  void resizeEvent(QResizeEvent*);

private:
  Diffractogram &diffractogram_;

  eTool tool_;
  bool showBgFit_;

  QCPGraph *bgGraph_, *dgramGraph_, *dgramBgFittedGraph_, *guesses_, *fits_;

  typ::vec<QCPGraph*>       reflGraph_;
  DiffractogramPlotOverlay *overlay_;
};

class Diffractogram : public BoxPanel {
  CLS(Diffractogram) SUPER(BoxPanel)
public:
  Diffractogram(TheHub&);

  void render();

  data::shp_Dataset dataset() const { return dataset_; }

private:
  void setDataset(data::shp_Dataset);

  data::shp_Dataset dataset_;

  DiffractogramPlot *plot_;

  typ::Curve     dgram_, dgramBgFitted_, bg_;
  typ::curve_vec refls_;

  uint                 currReflIndex_;
  calc::shp_Reflection currentReflection_;

public:
  void calcDgram();
  void calcBackground();
  void calcReflections();

  void        setCurrReflNewRange(typ::Range::rc);
  typ::Range currReflRange() const;
};

//------------------------------------------------------------------------------
}}
#endif // PANEL_DIFFRACTOGRAM_H
