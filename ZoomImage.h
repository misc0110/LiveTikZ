#ifndef _zoomImage_H_
#define _zoomImage_H_

#include <QLabel>
#include <QMouseEvent>
#include <QWheelEvent>

class ZoomImage : public QLabel {
  Q_OBJECT

signals:
  void zoomChanged(double factor);

public:
  ZoomImage();
  void wheelEvent(QWheelEvent *event) override;
  double getScale();

private:
  double scale;
};

#endif
