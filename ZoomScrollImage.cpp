#include <QScrollBar>
#include <ZoomImage.h>
#include <ZoomScrollImage.h>
#include <iostream>

ZoomScrollImage::ZoomScrollImage() {
  image = new ZoomImage;
  setWidget(image);
  image->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  image->setScaledContents(true);

  connect(image, SIGNAL(zoomChanged(double)), this, SLOT(zoomChangedForward(double)));
}

void ZoomScrollImage::zoomChangedForward(double scale) { emit zoomChanged(scale); }

double ZoomScrollImage::getScale() { return image->getScale(); }

void ZoomScrollImage::setImage(QImage &img) {
  image->setPixmap(QPixmap::fromImage(img));
  image->adjustSize();
}

void ZoomScrollImage::mousePressEvent(QMouseEvent *e) {
  mousePos = e->pos();
  scrollPos = QPoint(horizontalScrollBar()->value(), verticalScrollBar()->value());
}

void ZoomScrollImage::mouseMoveEvent(QMouseEvent *e) {
  QPoint diff = e->pos() - mousePos;
  mousePos = e->pos();
  verticalScrollBar()->setValue(verticalScrollBar()->value() - diff.y());
  horizontalScrollBar()->setValue(horizontalScrollBar()->value() - diff.x());
}
