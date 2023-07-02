/*
Copyright © 2022-2023  TokiNoBug
This file is part of FractalUtils.

FractalUtils is free software: you can redistribute it and/or modify
                                                                    it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

                                        FractalUtils is distributed in the hope
that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with FractalUtils.  If not, see <https://www.gnu.org/licenses/>.

   Contact with me:
   github:https://github.com/ToKiNoBug
*/

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