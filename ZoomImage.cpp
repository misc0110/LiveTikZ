#include <ZoomImage.h>
#include <iostream>

ZoomImage::ZoomImage() { scale = 1; }

void ZoomImage::wheelEvent(QWheelEvent *event) {
  if (event->modifiers() & Qt::ControlModifier) {
    if (event->delta() > 0) {
      scale *= 1.15;
    } else {
      scale /= 1.15;
    }
    event->accept();
    emit zoomChanged(scale);
  } else {
    event->ignore();
  }
}

double ZoomImage::getScale() { return scale; }
