#include "main.h"
#include "utils/check-inl.h"
#include "platform/cairo.h"
#include "platform/frame.h"

using namespace neutrino;

int main(int argc, char *argv[]) {
  Abort::install_signal_handlers();
  FrameSettings settings;
  Frame frame(settings);
  assert frame.initialize();
  Cairo cairo;
  assert frame.open(cairo);
  return 0;
}
