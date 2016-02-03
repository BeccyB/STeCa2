#include "dataset.h"
#include "mainwin.h"
#include "session.h"
#include <QPainter>
#include <QAction>

namespace panel {

ImageWidget::ImageWidget(Dataset& dataset_)
: dataset(dataset_), showOverlay(false) {
  setMinimumSize(16,16);  // so it does not completely disappear
}

void ImageWidget::setPixmap(QPixmap const& pixmap) {
  original = pixmap;
  update();
}

void ImageWidget::setShowOverlay(bool on) {
  showOverlay = on;
  update();
}

QSize ImageWidget::sizeHint() const {
  lastHeight = height();
  return QSize(lastHeight,lastHeight);
}

void ImageWidget::resizeEvent(QResizeEvent* e) {
  super::resizeEvent(e);
  auto h = height();
  if (h != lastHeight) {
    auto r = geometry();
    r.setWidth(h);
    setGeometry(r);
    updateGeometry(); // TODO on Mac stays a while, then changes
  }
}

void ImageWidget::paintEvent(QPaintEvent*) {
  if (lastPaintSize!=size()) {
    lastPaintSize = size();

    if (original.isNull()) {
      scaled = original;
      scale.setX(0); scale.setY(0);
    } else {
      // retransform
      scaled = original.scaled(width()-2,height()-2,Qt::IgnoreAspectRatio,Qt::FastTransformation);

      auto h = scaled.height(), w = scaled.width();

      scale.setX((qreal)w  / original.width());
      scale.setY((qreal)h  / original.height());
    }
  }

  QPainter painter(this);

  QRect r = rect();
  r.adjust(0,0,-1,-1);

  // frame
  painter.setPen(Qt::black);
  painter.drawRect(r);

  // image
  painter.drawPixmap(1,1,scaled);

  // overlay
  if (!showOverlay) return;

  // cut
  auto cut = dataset.getSession().getImageCut();
  r.adjust(qRound(scale.x()*cut.left),  qRound(scale.y()*cut.top),
          -qRound(scale.x()*cut.right),-qRound(scale.y()*cut.bottom));

  painter.setPen(Qt::lightGray);
  painter.drawRect(r);
}

void ImageWidget::update() {
  lastPaintSize = QSize();
  super::update();
}

//------------------------------------------------------------------------------

Dataset::Dataset(MainWin& mainWin_, Session& session_)
: super("",mainWin_,session_,Qt::Vertical), dataset(nullptr), globalNorm(false) {
  auto bb = hbox(); // (b)ottom (b)uttons

  bb->addWidget(check("gl. normalize",mainWin.actImagesGlobalNorm));
  bb->addWidget(icon(":/icon/top"));
  bb->addWidget((cutTop = spinCell(4,0)));
  bb->addWidget(icon(":/icon/bottom"));
  bb->addWidget((cutBottom = spinCell(4,0)));
  bb->addWidget(icon(":/icon/left"));
  bb->addWidget((cutLeft = spinCell(4,0)));
  bb->addWidget(icon(":/icon/right"));
  bb->addWidget((cutRight = spinCell(4,0)));
  bb->addWidget(iconButton(mainWin.actImagesLink));
  bb->addStretch();
  bb->addWidget(iconButton(mainWin.actImagesEye));

  auto sb = vbox(); // (s)ide   (b)uttons

  sb->addWidget(iconButton(mainWin.actImagesShowRaw));
  sb->addStretch();
  sb->addWidget(iconButton(session.actImageMirror));
  sb->addWidget(iconButton(session.actImageRotate));

  auto hb = hbox();
  hb->addWidget(imageWidget = new ImageWidget(*this),0,0);
  hb->addLayout(sb);

  box->addLayout(hb);
  box->addLayout(bb);

  connect(&session, &Session::filesChanged, [this](){
    bool on  = session.hasCorrFile();
    auto act = mainWin.actImagesShowRaw;
    if (!on) act->setChecked(false);
    act->setEnabled(on);
  });

  connect(mainWin.actImagesShowRaw, &QAction::toggled, [this](bool) {
    refresh();
  });

  auto setImageCut = [this](bool topLeft, int value) {
    if (mainWin.actImagesLink->isChecked())
      session.setImageCut(topLeft, true, core::ImageCut(value,value,value,value));
    else
      session.setImageCut(topLeft, false, core::ImageCut(cutTop->value(), cutBottom->value(), cutLeft->value(), cutRight->value()));
  };

  connect(cutTop, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [setImageCut](int value) {
    setImageCut(true,value);
  });

  connect(cutBottom, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [setImageCut](int value) {
    setImageCut(false,value);
  });

  connect(cutLeft, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [setImageCut](int value) {
    setImageCut(true,value);
  });

  connect(cutRight, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [setImageCut](int value) {
    setImageCut(false,value);
  });

  connect(&session_, &Session::imageCutChanged, [this]() {
    // set GUI from cut values
    auto cut = session.getImageCut();
    cutTop    ->setValue(cut.top);
    cutBottom ->setValue(cut.bottom);
    cutLeft   ->setValue(cut.left);
    cutRight  ->setValue(cut.right);

    refresh();
  });

  connect(mainWin.actImagesEye, &QAction::toggled, [this](bool on) {
    imageWidget->setShowOverlay(on);
  });

  mainWin.actImagesEye->setChecked(true);

  connect(mainWin.actImagesGlobalNorm, &QAction::toggled, [this](bool on) {
    globalNorm = on;
    refresh();
  });

  connect(&session, &Session::datasetSelected, [this](core::shp_Dataset dataset) {
    setDataset(dataset);
  });

  connect(session.actImageMirror, &QAction::toggled, [this](bool on) {
    session.setImageMirror(on);
    refresh();
  });

  connect(session.actImageRotate, &QAction::triggered, [this]() {
    session.nextImageRotate();
    refresh();
  });
}

QPixmap Dataset::makePixmap(core::Image const& image, core::Range rgeIntens,
                            core::Image* corr) {
  QPixmap pixmap;
  auto size = image.getSize();

  if (!size.isEmpty()) {
    qreal mi = rgeIntens.max;
    if (mi <= 0) mi = 1;  // sanity

    QImage qimage(size,QImage::Format_RGB32);

    for_i(size.height()) {
      auto y = i;
      for_i(size.width()) {
        auto x = i;
        qreal intens = image.intensity(session.imageTransform,x,y) / mi;
        bool isNan = false; // TODO temporary fix
        if (corr) {
          auto factor = corr->intensity(session.imageTransform,x,y);
          if (qIsFinite(factor)) // TODO still actual?
            intens *= factor;
          else
            isNan = true;
        }

        QRgb rgb;
        if (isNan)
          rgb = qRgb(0xff,0xff,0xff);
        else if (intens < 0.25)
          rgb = qRgb(0xff * intens * 4, 0, 0);
        else if (intens < 0.5)
          rgb = qRgb(0xff, 0xff * (intens - 0.25) * 4, 0);
        else if (intens < 0.75)
          rgb = qRgb(0xff - (0xff * (intens - 0.5) * 4), 0xff, (0xff * (intens - 0.5) * 4));
        else
          rgb = qRgb(0xff * (intens - 0.75) * 4, 0xff, 0xff);

        qimage.setPixel(x, y, rgb);
      }
    }

    pixmap = QPixmap::fromImage(qimage);
  }

  return pixmap;
}

void Dataset::setDataset(core::shp_Dataset dataset_) {
  dataset = dataset_;
  QPixmap pixMap;
  if (dataset) {
    auto image = dataset->getImage();
    core::Image *corr = nullptr;
    if (session.hasCorrFile() && !mainWin.actImagesShowRaw->isChecked())
      corr = &session.intensCorrArray;
    pixMap = makePixmap(image, globalNorm ? dataset->getFile().getRgeIntens() : image.getRgeIntens(), corr);
  }
  imageWidget->setPixmap(pixMap);
}

void Dataset::refresh() {
  setDataset(dataset);
}

}

// eof
