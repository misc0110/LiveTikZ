#ifndef _ZoomScrollImage_H_
#define _ZoomScrollImage_H_

#include <QImage>
#include <QPoint>
#include <QScrollArea>
#include <ZoomImage.h>

class ZoomScrollImage : public QScrollArea {
  Q_OBJECT

public:
  ZoomScrollImage();
  double getScale();
  void setImage(QImage &image);

public slots:
  void zoomChangedForward(double scale);

private:
  ZoomImage *image;
  QPoint mousePos;
  QPoint scrollPos;

signals:
  void zoomChanged(double factor);

protected:
  void mousePressEvent(QMouseEvent *e) override;
  void mouseMoveEvent(QMouseEvent *e) override;
};

#endif
