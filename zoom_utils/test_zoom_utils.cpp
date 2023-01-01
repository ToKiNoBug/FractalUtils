#include "zoom_utils.h"

#include <QApplication>

int main(int argC, char **argV) {
  QApplication qga(argC, argV);
  using namespace fractal_utils;

  mainwindow w(float(1), nullptr, {320, 320});

  w.show();

  return qga.exec();
}