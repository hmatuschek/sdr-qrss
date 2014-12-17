#include <QApplication>
#include "receiver.hh"
#include "mainwindow.hh"


using namespace sdr;


int main(int argc, char *argv[])
{
  /*  Init  */
  PortAudio::init();
  QApplication app(argc, argv);
  Queue &queue = Queue::get();

  /* Register log handler. */
  sdr::Logger::get().addHandler(
        new sdr::StreamLogHandler(std::cerr, sdr::LOG_DEBUG));

  Receiver rx;
  MainWindow win(&rx);

  win.show();

  // GO
  app.exec();

  // Done
  queue.stop();
  queue.wait();

  PortAudio::terminate();

  return 0;
}
