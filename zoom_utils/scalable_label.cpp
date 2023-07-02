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

#include <QEvent>
#include <QMouseEvent>

#include "scalable_label.h"

scalable_label::scalable_label(QWidget *parent) : QLabel(parent) {
  this->setMouseTracking(true);
}

void scalable_label::wheelEvent(QWheelEvent *event) {
  if (!this->lock.try_lock()) {
    return;
  }
  const QPointF point = event->position();

  // bool is_inverted = event->isInverted();
  // inverted() and isInverted don't work on windows10 Qt6.1.0
  const QPointF angle_delta = event->angleDelta();

  bool is_scaling_up;

  if (angle_delta.y() != 0) {
    is_scaling_up = (angle_delta.y() > 0);
  } else {
    is_scaling_up = (angle_delta.x() > 0);
  }

  event->accept();

  emit zoomed({(int)point.y(), (int)point.x()}, is_scaling_up);
  /*
    cout << "position : " << point.x() << ", " << point.y();
    cout << ", angle_delta = " << angle_delta.x() << ", " << angle_delta.y()
         << endl;
         */
  //<< ", inverted = " << (is_inverted ? "true" : "false") << endl;

  this->lock.unlock();
}

void scalable_label::mouseMoveEvent(QMouseEvent *event) {
  const QPointF point = event->position();

  emit moved({(int)point.y(), (int)point.x()});

  event->accept();
}

void scalable_label::mouseDoubleClickEvent(QMouseEvent *event) {
  if (!this->lock.try_lock()) {
    return;
  }
  const QPointF point = event->position();

  bool is_scaling_up{true};

  event->accept();

  emit zoomed({(int)point.y(), (int)point.x()}, is_scaling_up);
  /*
    cout << "position : " << point.x() << ", " << point.y();
    cout << ", angle_delta = " << angle_delta.x() << ", " << angle_delta.y()
         << endl;
         */
  //<< ", inverted = " << (is_inverted ? "true" : "false") << endl;

  this->lock.unlock();
}