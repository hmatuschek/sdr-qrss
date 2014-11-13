#include "sdr.hh"
#include "rtlsource.hh"
#include "baseband.hh"
#include "utils.hh"
#include "gui/gui.hh"
#include <signal.h>
#include "portaudio.hh"

#include <QApplication>
#include <QMainWindow>
#include <QThread>

#include "qrss.hh"


using namespace sdr;

static void __sigint_handler(int signo) {
  // On SIGINT -> stop queue properly
  Queue::get().stop();
}



int main(int argc, char *argv[])
{
  Queue &queue = Queue::get();

  // Register handler:
  signal(SIGINT, __sigint_handler);

  PortAudio::init();

  sdr::Logger::get().addHandler(
        new sdr::StreamLogHandler(std::cerr, sdr::LOG_DEBUG));


  // Assemble processing chain
  PortSource< int16_t > src(12000, 2048);
  AGC< int16_t > agc; agc.enable(true);
  QRSS qrss(700, 3, 300);

  src.connect(&agc);
  agc.connect(&qrss);

  queue.addIdle(&src, &PortSource< int16_t >::next);

  QApplication app(argc, argv);
  QMainWindow        *win       = new QMainWindow();
  gui::WaterFallView *spec_view = new gui::WaterFallView(&qrss, 640, gui::WaterFallView::RIGHT_LEFT);

  win->setCentralWidget(spec_view);
  win->setMinimumSize(640, 480);

  win->show();

  queue.start();
  app.exec();

  queue.stop();
  queue.wait();

  PortAudio::terminate();

  return 0;
}
