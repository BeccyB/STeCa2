/** \file
 */

#ifndef DIFFRACTOGRAM_H
#define DIFFRACTOGRAM_H

#include "panel.h"
#include "core_dataset.h"
#include "core_types.h"
#include "core_fit_methods.h"
#include "QCP/qcustomplot.h"

namespace panel {
//------------------------------------------------------------------------------

class DiffractogramPlot;

class DiffractogramPlotOverlay: public QWidget {
  SUPER(DiffractogramPlotOverlay,QWidget)
public:
  DiffractogramPlotOverlay(DiffractogramPlot&);

  void setMargins(int left,int right);

private:
  DiffractogramPlot& plot;
  QColor addColor, remColor, color;
  int marginLeft, marginRight;

protected:
  void enterEvent(QEvent*);
  void leaveEvent(QEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent*);

  void paintEvent(QPaintEvent*);

  bool hasCursor, mouseDown;
  int  cursorPos, mouseDownPos;

  void updateCursorRegion();
};

class Diffractogram;

class DiffractogramPlot: public QCustomPlot {
  SUPER(DiffractogramPlot,QCustomPlot)
public:
  enum Tool {
    TOOL_NONE,
    TOOL_BACKGROUND,
    TOOL_PEAK_REGION,
  };

  DiffractogramPlot(TheHub&,Diffractogram&);

public:
  void setTool(Tool);
  Tool getTool() const { return tool; }

  void plot(core::TI_Curve const&,core::TI_Curve const&, core::TI_Curve const&, core::TI_Curve const&);

  core::Range fromPixels(int,int);

  void clearBg();
  void addBg(core::Range const&);
  void remBg(core::Range const&);
  void setPeakRange(core::Range const&);
  void updateBg();

protected:
  void addBgItem(core::Range const&);
  void resizeEvent(QResizeEvent*);

private:
  TheHub        &theHub;
  Diffractogram &diffractogram;
  Tool tool;
  QCPGraph *bgGraph, *dgramGraph, *dgramBgFittedGraph, *dgramPeak;
  DiffractogramPlotOverlay *overlay;

};

class Diffractogram: public BoxPanel {
  SUPER(Diffractogram,BoxPanel)
  friend class DiffractogramPlot;
public:
  Diffractogram(TheHub&);

private:
  void setDataset(core::shp_Dataset);
  void renderDataset();

  core::shp_Dataset dataset;

  DiffractogramPlot *plot;

  core::TI_Curve dgram, dgramBgFitted, bg, peak;
  core::Ranges bgRanges;
  core::fit::Polynomial bgPolynomial;
  bool showBgFit;

  core::Range peakRange;

public:
  void calcDgram();
  void calcBackground();
  void calcPeak();  // TODO temporary for development, fitting one Gaussian peak
};

//------------------------------------------------------------------------------
}
#endif
