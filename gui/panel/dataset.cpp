#include "dataset.h"
#include "thehub.h"
#include "settings.h"
#include <QPainter>
#include <QAction>

namespace panel {
//------------------------------------------------------------------------------

ImageWidget::ImageWidget(TheHub& theHub_,Dataset& dataset_)
: theHub(theHub_), dataset(dataset_), showOverlay(false), scale(2) {
  setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
}

void ImageWidget::setPixmap(QPixmap const& pixmap) {
  original = pixmap;
  setScale(scale);
}

void ImageWidget::setShowOverlay(bool on) {
  showOverlay = on;
  update();
}

void ImageWidget::setScale(uint scale_) {
  ASSERT(scale_ > 0)
  scale = scale_;

  scaled = original.isNull() ? original : original.scaled(original.size()*scale);

  updateGeometry();
  update();
}

QSize ImageWidget::sizeHint() const {
  return scaled.size() + QSize(2,2);
}

void ImageWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);

  // frame
  painter.setPen(Qt::black);
  painter.drawRect(rect().adjusted(0,0,-1,-1));

  // image
  painter.drawPixmap(1,1,scaled);

  // overlay follows
  if (!showOverlay) return;

  // cut
  auto cut = theHub.getImageCut();
  QRect r = rect()
    .adjusted(1,1,-2,-2)
    .adjusted(scale*cut.left,scale*cut.top,-scale*cut.right,-scale*cut.bottom);

  painter.setPen(Qt::lightGray);
  painter.drawRect(r);
}

//------------------------------------------------------------------------------

DatasetOptions1::DatasetOptions1(TheHub& theHub_)
: super(EMPTY_STR,theHub_,Qt::Vertical) {

  box->addWidget(label("Beam offset"));
  auto ho = hbox();
  box->addLayout(ho);

  ho->addWidget(label("X"));
  ho->addWidget((spinOffsetX = spinCell(4,0)));
  spinOffsetX->setToolTip("Horizontal offset from image center");
  ho->addWidget(label("Y"));
  ho->addWidget((spinOffsetY = spinCell(4,0)));
  spinOffsetY->setToolTip("Vertical offset from image center");
  ho->addWidget(label("pix"));
  ho->addWidget(iconButton(theHub.actHasBeamOffset));
  ho->addStretch();

  box->addWidget(label("Detector"));
  auto gd = gridLayout();
  box->addLayout(gd);

  gd->addWidget((spinDistance = spinCell(6,MIN_DISTANCE)),    0,0);
  spinDistance->setToolTip("Sample to detector distance");
  gd->addWidget(label("distance mm"),                         0,1);
  gd->addWidget((spinPixelSize = spinCell(6,MIN_PIXEL_SIZE)), 1,0);
  spinPixelSize->setToolTip("Physical pixel size");
  gd->addWidget(label("pixel size mm"),                       1,1);
  gd->setColumnStretch(2,1);

  box->addWidget(label("Normalization"));
  auto vn = vbox();
  box->addLayout(vn);

  QVector<QString> options;
  options.append("Disable");
  options.append("Measurement time");
  options.append("Monitor counts");
  options.append("Background level");

  vn->addWidget(comboBox(options));
  box->addStretch();

  // TODO separate geometryChanged into more signals?
  connect(&theHub, &TheHub::geometryChanged, [this]() {
    setFrom(theHub);
  });

  auto setEnabled = [this]() {
    bool on = theHub.actHasBeamOffset->isChecked();
    spinOffsetX->setEnabled(on);
    spinOffsetY->setEnabled(on);
  };

  setEnabled();

  // TODO split setTo() ?
  connect(theHub.actHasBeamOffset, &QAction::toggled, [this,setEnabled]() {
    setEnabled();
    setTo(theHub);
  });

  connect(spinOffsetX, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this]() {
    setTo(theHub);
  });

  connect(spinOffsetY, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this]() {
    setTo(theHub);
  });

  connect(spinDistance, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this]() {
    setTo(theHub);
  });

  connect(spinPixelSize, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [this]() {
    setTo(theHub);
  });

  // TODO handle in TheHub() ?
  connect(&theHub, &TheHub::readSettings, [this]() {
    readSettings(theHub);
  });

  connect(&theHub, &TheHub::saveSettings, [this]() {
    saveSettings();
  });
}

void DatasetOptions1::setTo(TheHub& theHub) {
  theHub.setGeometry(
    spinDistance->value(), spinPixelSize->value(),
    theHub.actHasBeamOffset->isChecked(),
    QPoint(spinOffsetX->value(),spinOffsetY->value()));
}

void DatasetOptions1::setFrom(TheHub& theHub) {
  auto const& g = theHub.getGeometry();

  theHub.actHasBeamOffset->setChecked(g.hasBeamOffset);
  spinOffsetX->setValue(g.middlePixOffset.x());
  spinOffsetY->setValue(g.middlePixOffset.y());

  spinDistance->setValue(g.sampleDetectorSpan);
  spinPixelSize->setValue(g.pixSpan);
}

static str GROUP_OPTIONS("Options");
static str KEY_IMAGE_SCALE("image_scale");

static str GROUP_BEAM("Beam");
static str KEY_IS_OFFSET("is_offset");
static str KEY_OFFSET_X("offset_x");
static str KEY_OFFSET_Y("offset_y");

static str GROUP_DETECTOR("Detector");
static str KEY_DISTANCE("distance");
static str KEY_PIXEL_SIZE("pixel_size");

void DatasetOptions1::readSettings(TheHub& theHub) {
  {
    Settings s(GROUP_BEAM);
    auto const& g = theHub.getGeometry();
    s.read(KEY_IS_OFFSET,   theHub.actHasBeamOffset,g.hasBeamOffset);
    s.read(KEY_OFFSET_X,    spinOffsetX,      g.middlePixOffset.x());
    s.read(KEY_OFFSET_Y,    spinOffsetY,      g.middlePixOffset.y());
  }
  {
    Settings s(GROUP_DETECTOR);
    auto const& g = theHub.getGeometry();
    s.read(KEY_DISTANCE,    spinDistance,     g.sampleDetectorSpan);
    s.read(KEY_PIXEL_SIZE,  spinPixelSize,    g.pixSpan);
  }
}

void DatasetOptions1::saveSettings() {
  {
    Settings s(GROUP_BEAM);
    s.save(KEY_IS_OFFSET,   theHub.actHasBeamOffset);
    s.save(KEY_OFFSET_X,    spinOffsetX);
    s.save(KEY_OFFSET_Y,    spinOffsetY);
  }
  {
    Settings s(GROUP_DETECTOR);
    s.save(KEY_DISTANCE,    spinDistance);
    s.save(KEY_PIXEL_SIZE,  spinPixelSize);
  }
}

//------------------------------------------------------------------------------

DatasetOptions2::DatasetOptions2(TheHub& theHub_)
: super (EMPTY_STR,theHub_,Qt::Vertical) {

  // TODO clean up the layout

  box->addWidget(label("Image"));
  auto hb = hbox();
  box->addLayout(hb);

  hb->addWidget(iconButton(theHub.actImageRotate));
  hb->addSpacing(5);
  hb->addWidget(iconButton(theHub.actImageMirror));
  hb->addSpacing(5);
  hb->addWidget(iconButton(theHub.actImagesGlobalNorm));
  hb->addStretch();

  auto sc = hbox();
  box->addLayout(sc);
  sc->addWidget(label("Scaling"));
  sc->addSpacing(5);
  sc->addWidget((spinImageScale = spinCell(4,1,4)));
  spinImageScale->setToolTip("Image scale");
  sc->addStretch();

  box->addWidget(label("Cut"));
  auto gc = gridLayout();
  box->addLayout(gc);

  gc->addWidget(icon(":/icon/cutTopU"),             0,0);
  gc->addWidget((cutTop = spinCell(4,0)),           0,1);
  cutTop->setToolTip("Top cut");
  gc->addWidget(icon(":/icon/cutBottomU"),          0,2);
  gc->addWidget((cutBottom = spinCell(4,0)),        0,3);
  cutBottom->setToolTip("Bottom cut");

  gc->addWidget(iconButton(theHub.actImagesLink),  0,5);
  gc->addWidget(iconButton(theHub.actImageOverlay), 1,5);

  gc->addWidget(icon(":/icon/cutLeftU"),            1,0);
  gc->addWidget((cutLeft = spinCell(4,0)),          1,1);
  cutLeft->setToolTip("Left cut");
  gc->addWidget(icon(":/icon/cutRightU"),           1,2);
  gc->addWidget((cutRight = spinCell(4,0)),         1,3);
  cutRight->setToolTip("Right cut");
  gc->setColumnStretch(4,1);

  box->addStretch();

  auto setImageCut = [this](bool topLeft, int value) {
    if (theHub.actImagesLink->isChecked())
      theHub.setImageCut(topLeft, true, core::ImageCut(value,value,value,value));
    else
      theHub.setImageCut(topLeft, false, core::ImageCut(cutTop->value(), cutBottom->value(), cutLeft->value(), cutRight->value()));
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

  // TODO separate geometryChanged into more signals?
  connect(&theHub, &TheHub::geometryChanged, [this]() {
    setFrom(theHub);
  });

  connect(spinImageScale, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [this](int scale) {
    emit imageScale(scale);
  });

  // TODO handle in Session() ?
  connect(&theHub, &TheHub::readSettings, [this]() {
    readSettings();
  });

  connect(&theHub, &TheHub::saveSettings, [this]() {
    saveSettings();
  });
}

void DatasetOptions2::setFrom(TheHub& theHub) {
  auto cut = theHub.getImageCut();

  cutTop    ->setValue(cut.top);
  cutBottom ->setValue(cut.bottom);
  cutLeft   ->setValue(cut.left);
  cutRight  ->setValue(cut.right);
}

void DatasetOptions2::readSettings() {
  {
    Settings s(GROUP_OPTIONS);
    s.read(KEY_IMAGE_SCALE, spinImageScale, 4);
  }

  emit imageScale(spinImageScale->value()); // TODO review
}

void DatasetOptions2::saveSettings() {
  {
    Settings s(GROUP_OPTIONS);
    s.save(KEY_IMAGE_SCALE, spinImageScale);
  }
}

//------------------------------------------------------------------------------

Dataset::Dataset(TheHub& theHub_)
: super("",theHub_,Qt::Vertical), dataset(nullptr) {

  box->addWidget(imageWidget = new ImageWidget(theHub,*this),0,Qt::AlignCenter);

  connect(theHub.actImagesEnableCorr, &QAction::toggled, [this](bool) {
    renderDataset();
  });

  connect(theHub.actImageOverlay, &QAction::toggled, [this](bool on) {
    imageWidget->setShowOverlay(on);
  });

  theHub.actImageOverlay->setChecked(true);

  connect(&theHub, &TheHub::displayChange, [this]() {
    renderDataset();
  });

  connect(&theHub, &TheHub::datasetSelected, [this](core::shp_Dataset dataset) {
    setDataset(dataset);
  });

  connect(&theHub, &TheHub::geometryChanged, [this]() {
    renderDataset();
  });
}

void Dataset::setImageScale(uint scale) {
  imageWidget->setScale(scale);
}

QPixmap Dataset::makePixmap(core::Image const& image, core::Range rgeIntens) {
  QPixmap pixmap;
  auto size = theHub.getImageSize();

  if (!size.isEmpty()) {
    qreal mi = rgeIntens.max;
    if (mi <= 0) mi = 1;  // sanity

    QImage qimage(size,QImage::Format_RGB32);

    for_i (size.height()) {
      auto y = i;
      for_i (size.width()) {
        auto x = i;
        qreal intens = theHub.pixIntensity(image,x,y) / mi;

        QRgb rgb;
        if (qIsNaN(intens))
          rgb = qRgb(0x00,0xff,0xff);
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
  renderDataset();
}

void Dataset::renderDataset() {
  QPixmap pixMap;
  if (dataset) {
    auto image = dataset->getImage();
    pixMap = makePixmap(image, dataset->getRgeIntens(theHub.globalNorm));
  }
  imageWidget->setPixmap(pixMap);
}

//------------------------------------------------------------------------------
}
// eof
