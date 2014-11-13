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

#include <getopt.h>

#include "qrss.hh"


using namespace sdr;

static void __sigint_handler(int signo) {
  // On SIGINT -> stop queue properly
  Queue::get().stop();
}


int debug_flag;
int monitor_flag;

static struct option long_options[] = {
{"source", required_argument, 0, 's'},
{"frequency", required_argument, 0, 'f'},
{"debug", no_argument, &debug_flag, 1},
{"monitor", no_argument, &monitor_flag, 1},
{0,0,0,0}
};



int main(int argc, char *argv[])
{
  /*
   * Parse command line options.
   */
  bool has_source=false, has_freq=false;
  std::string source_str, freq_str;

  while (true) {
    int option_index = 0;
    int c = getopt_long(argc, argv, "s:f:", long_options, &option_index);
    if (-1 == c) { break; }

    switch (c) {
    case 0:
      break;

    case 's':
      has_source = true;
      source_str = optarg;
      break;

    case 'f':
      has_freq = true;
      freq_str = optarg;
      break;

    case '?':
      break;

    default:
      abort();
    }
  }

  // Register handler:
  signal(SIGINT, __sigint_handler);

  /*
   *  Init
   */
  QApplication app(argc, argv);
  Queue &queue = Queue::get();
  PortAudio::init();

  if (debug_flag) {
    sdr::Logger::get().addHandler(
          new sdr::StreamLogHandler(std::cerr, sdr::LOG_DEBUG));
  }

  /*
   * Assemble processing chain
   */
  Source                *src = 0;
  PortSource< int16_t>  *port_src = 0;
  AGC< int16_t>         *agc = 0;
  RTLSource             *rtl_src = 0;
  AutoCast< std::complex<int16_t> > *ccast = 0;
  IQBaseBand< int16_t > *baseband = 0;
  USBDemod<int16_t>     *demod = 0;
  PortSink              *audio_sink = 0;

  // Default source == "audio"
  if ((! has_source) || ("audio" == source_str) ) {
    port_src = new PortSource< int16_t >(12000, 512);
    src = agc = new AGC< int16_t >(); agc->enable(true);
    port_src->connect(agc);
    queue.addIdle(port_src, &PortSource< int16_t >::next);
  } else if (has_source && ("rtl" == source_str)) {
    if (! has_freq) {
      std::cerr << "RTL: RX Frequency not specified!" << std::endl;
      return -1;
    }
    rtl_src  = new RTLSource(atof(freq_str.c_str())-700, 800e3);
    ccast    = new AutoCast< std::complex<int16_t> >();
    baseband = new IQBaseBand< int16_t >(0, 700, 300, 16, 1, 12000);
    src = demod = new USBDemod< int16_t >();
    rtl_src->connect(ccast);
    ccast->connect(baseband, true);
    baseband->connect(demod);
    Queue::get().addStart(rtl_src, &RTLSource::start);
    Queue::get().addStop(rtl_src, &RTLSource::stop);
  } else {
    std::cerr << "Unkown source specification: '" << source_str
              << "'. Possible values 'audio', 'rtl'" << std::endl;
    return -1;
  }

  QRSS qrss(700, 3, 300);
  src->connect(&qrss);

  if (monitor_flag) {
    audio_sink = new PortSink();
    src->connect(audio_sink);
  }

  /*
   * Init GUI
   */
  QMainWindow        *win       = new QMainWindow();
  gui::WaterFallView *spec_view = new gui::WaterFallView(&qrss, 640, gui::WaterFallView::RIGHT_LEFT);

  win->setCentralWidget(spec_view);
  win->setMinimumSize(640, 480);
  win->show();

  // GO
  queue.start();
  app.exec();

  // Done
  queue.stop();
  queue.wait();

  // Free mem.
  if (port_src)   { delete port_src; }
  if (agc)        { delete agc; }
  if (rtl_src)    { delete rtl_src; }
  if (ccast)      { delete ccast; }
  if (baseband)   { delete baseband; }
  if (demod)      { delete demod; }
  if (audio_sink) { delete audio_sink; }

  PortAudio::terminate();

  return 0;
}
