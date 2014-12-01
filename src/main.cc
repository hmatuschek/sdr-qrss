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

#include "options.hh"
#include "qrss.hh"


using namespace sdr;

static void __sigint_handler(int signo) {
  // On SIGINT -> stop queue properly
  Queue::get().stop();
}


static Options::Definition option_definitions[] = {
  {"source", 's', Options::ANY,
   "Specifies the input source. Possible arguments are 'audio' and 'rtl', "
   "where 'audio' specifies the sound card as the input source while 'rtl' "
   "specifies an RTL2832 USB dongle as the input source. "
   "Default: 'audio'"},
  {"frequency", 'f', Options::FLOAT,
   "Specifies the center frequency (in Hz), if a RTL2832 source is used."},
  {"dot-length", 0, Options::FLOAT,
   "Specifies the QRSS dot length is seconds. Default: 3s"},
  {"width", 0, Options::FLOAT,
   "Specifies the width of the visible spectrum in Hz. Default: 300Hz"},
  {"history", 0, Options::INTEGER,
   "Specifies the number of frames for the history (x-axis) of the spectum view."},
  {"bfo-frequency", 0, Options::FLOAT,
   "Specifies the BFO frequency in Hz. Default: 800 Hz"},
  {"agc", 0, Options::FLAG,
   "Enables the AGC."},
  {"debug", 0, Options::FLAG,
   "Enables the debug output."},
  {"monitor", 0, Options::FLAG,
   "Enables the audio monitoring."},
  {"help", 'h', Options::FLAG,
   "Prints this text."},
  {0,0,Options::FLAG,0}
};


int main(int argc, char *argv[])
{
  /*
   * Parse command line options.
   */
  Options options;
  if (! Options::parse(option_definitions, argc, argv, options)) { return -1; }
  if (options.has("help")) {
    std::cout << "sdr-qrss -- A QRSS receiver using libsdr." << std::endl << std::endl
              << " Usage: sdr-qrss [OPTIONS]" << std::endl << std::endl;
    Options::print_help(std::cout, option_definitions);
    return 0;
  }

  // Register handler:
  signal(SIGINT, __sigint_handler);

  /*
   *  Init
   */
  QApplication app(argc, argv);
  Queue &queue = Queue::get();
  PortAudio::init();

  if (options.has("debug")) {
    sdr::Logger::get().addHandler(
          new sdr::StreamLogHandler(std::cerr, sdr::LOG_DEBUG));
  } else {
    sdr::Logger::get().addHandler(
          new sdr::StreamLogHandler(std::cerr, sdr::LOG_WARNING));
  }


  double Fbfo       = 800;
  double dot_length = 3;
  double spec_width = 300;
  int    hist       = 800;

  if (options.has("dot-length")) { dot_length = options.get("dot-length").toFloat(); }
  if (options.has("width")) { spec_width = options.get("width").toFloat(); }
  if (options.has("bfo-frequency")) { Fbfo = options.get("bfo-frequency").toFloat(); }

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
  try {
    if ((! options.has("source")) || ("audio" == options.get("source").toString()) ) {
      port_src = new PortSource< int16_t >(12000, 512);
      src = agc = new AGC< int16_t >();
      if (options.has("agc")) { agc->enable(true); }
      else { agc->enable(false); }
      port_src->connect(agc);
      queue.addIdle(port_src, &PortSource< int16_t >::next);
    } else if (options.has("source") && ("rtl" == options.get("source").toString())) {
      if (! options.has("frequency")) {
        std::cerr << "RTL: RX Frequency not specified!" << std::endl;
        return -1;
      }
      rtl_src  = new RTLSource(options.get("frequency").toFloat()-Fbfo, 800e3);
      ccast    = new AutoCast< std::complex<int16_t> >();
      baseband = new IQBaseBand< int16_t >(0, Fbfo, spec_width, 17, 1, 12000);
      src = demod = new USBDemod< int16_t >();
      if (options.has("agc")) { rtl_src->enableAGC(true); }
      else { rtl_src->enableAGC(false); }
      rtl_src->connect(ccast);
      ccast->connect(baseband, true);
      baseband->connect(demod);
      Queue::get().addStart(rtl_src, &RTLSource::start);
      Queue::get().addStop(rtl_src, &RTLSource::stop);
    } else {
      std::cerr << "Unkown source specification: '" << options.get("source").toString()
                << "'. Possible values 'audio', 'rtl'" << std::endl;
      return -1;
    }
  } catch (SDRError &err) {
    LogMessage msg(LOG_ERROR); msg << err.what();
    msg << std::endl << "Abort.";
    Logger::get().log(msg);
    return -1;
  }

  QRSS qrss(Fbfo, dot_length, spec_width);
  src->connect(&qrss);

  if (options.has("monitor")) {
    audio_sink = new PortSink();
    src->connect(audio_sink);
  }

  /*
   * Init GUI
   */
  QMainWindow        *win       = new QMainWindow();
  gui::WaterFallView *spec_view = new gui::WaterFallView(&qrss, hist, gui::WaterFallView::RIGHT_LEFT);

  win->setWindowTitle("sdr-qrss");
  win->setCentralWidget(spec_view);
  win->setMinimumSize(hist, 480);
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
