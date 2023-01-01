#include "zoom_utils.h"

#include <QApplication>

int main(int argC, char **argV) {
  QApplication qga(argC, argV);
  using namespace fractal_utils;

  mainwindow w(nullptr, {320, 320}, callback_create_center_wind<float>,
               callback_destroy_center_wind);

  w.show();

  return qga.exec();
}