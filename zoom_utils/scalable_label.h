#ifndef FRACTALUTILS_ZOOM_UTILS_SCALABLE_LABEL_H
#define FRACTALUTILS_ZOOM_UTILS_SCALABLE_LABEL_H

#include <QLabel>
#include <mutex>

class QWheelEvent;
class QMouseEvent;

class scalable_label : public QLabel {
  Q_OBJECT
 public:
  explicit scalable_label(QWidget *parent = nullptr);
 signals:

  void zoomed(std::array<int, 2> pos, bool is_scaling_up);
  void moved(std::array<int, 2> pos);

 private:
  std::mutex lock;

 protected:
  void wheelEvent(QWheelEvent *event) override;

  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
};

#endif  // FRACTALUTILS_ZOOM_UTILS_SCALABLE_LABEL_H