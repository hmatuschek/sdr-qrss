#include "receiver.hh"
#include <QLabel>


/* ********************************************************************************************* *
 * Implementation of QRSSSource
 * ********************************************************************************************* */
QRSSSource::QRSSSource(double Fbfo, double width, QObject *parent)
  : QObject(parent), _Fbfo(Fbfo), _width(width)
{
  // pass...
}

QRSSSource::~QRSSSource() {
  // pass...
}

void
QRSSSource::setBFOFrequency(double F) {
  _Fbfo = F;
}

void
QRSSSource::setSpectrumWidth(double width) {
  _width = width;
}


/* ********************************************************************************************* *
 * Implementation of AudioSource
 * ********************************************************************************************* */
AudioSource::AudioSource(double Fbfo, double width, QObject *parent)
  : QRSSSource(Fbfo, width, parent), _src(16e3, 256), _ctrlView(0)
{
  // Connect to idle signal of queue
  sdr::Queue::get().addIdle(&_src, &sdr::PortSource<int16_t>::next);
}

AudioSource::~AudioSource()
{
  // unregister idle callbacks
  sdr::Queue::get().remIdle(&_src);
  if (0 != _ctrlView) {
    // delete ctrl view later
    _ctrlView->deleteLater();
  }
}

sdr::Source *
AudioSource::source() {
  return &_src;
}

QWidget *
AudioSource::view() {
  if (0 == _ctrlView) {
    _ctrlView = new QLabel("No settings for audio source.");
    QObject::connect(_ctrlView, SIGNAL(destroyed()), this, SLOT(onViewDeleted()));
  }
  return _ctrlView;
}

void
AudioSource::onViewDeleted() {
  _ctrlView = 0;
}


/* ********************************************************************************************* *
 * Implementation of IQAudioSource
 * ********************************************************************************************* */
IQAudioSource::IQAudioSource(double Fbfo, double width, QObject *parent)
  : QRSSSource(Fbfo, width, parent), _src(16e3, 256), _filter(0, Fbfo, width, 31, 1), _demod(),
    _ctrlView(0)
{
  sdr::Queue::get().addIdle(&_src, &sdr::PortSource< std::complex<int16_t> >::next);
  _src.connect(&_filter, true);
  _filter.connect(&_demod, true);
}

IQAudioSource::~IQAudioSource() {
  // unregister idle callbacks
  sdr::Queue::get().remIdle(&_src);
  if (0 != _ctrlView) {
    // delete ctrl view later
    _ctrlView->deleteLater();
  }
}

void
IQAudioSource::setBFOFrequency(double F) {
  QRSSSource::setBFOFrequency(F);
  _filter.setFilterFrequency(F);
}

void
IQAudioSource::setSpectrumWidth(double width) {
  QRSSSource::setSpectrumWidth(width);
  _filter.setFilterWidth(width);
}

sdr::Source *
IQAudioSource::source() {
  return &_demod;
}

QWidget *
IQAudioSource::view() {
  if (0 == _ctrlView) {
    _ctrlView = new QLabel("No settings for IQ audio source.");
    QObject::connect(_ctrlView, SIGNAL(destroyed()), this, SLOT(onViewDeleted()));
  }
  return _ctrlView;
}

void
IQAudioSource::onViewDeleted() {
  _ctrlView = 0;
}


/* ********************************************************************************************* *
 * Implementation of Receiver
 * ********************************************************************************************* */
Receiver::Receiver(QObject *parent) :
  QObject(parent), _sourceType(AUDIO_SOURCE), _source(0), _agc(0.1, 10e3), _qrss(800, 3, 300), _monitor(true),
  _audioSink(), _settings("com.github.hmatuschek", "sdr-qrss")
{
  // Config AGC
  _agc.enable(_settings.value("agc", false).toBool());
  _agc.setGain(_settings.value("gain", 1.0).toDouble());

  // Config QRSS node
  _qrss.setFbfo(_settings.value("Fbfo", 800.0).toDouble());
  _qrss.setDotLength(_settings.value("dotLength", 3.0).toDouble());
  _qrss.setWidth(_settings.value("width", 300.0).toDouble());

  // Config monitor
  _monitor = _settings.value("monitor", true).toBool();

  _source = new AudioSource(_qrss.Fbfo(), _qrss.width());
  _source->source()->connect(&_agc);
  _agc.connect(&_qrss, true);
  if (_monitor) {
    _agc.connect(&_audioSink, true);
  }
}

Receiver::~Receiver() {
  if (0 != _source) { delete _source; }
}

Receiver::SourceType
Receiver::sourceType() const {
  return _sourceType;
}

void
Receiver::setSourceType(SourceType source) {
  _sourceType = source;
  if (0 != _source) { delete _source; }
  switch (_sourceType) {
  case AUDIO_SOURCE:
    _source = new AudioSource(_qrss.Fbfo(), _qrss.width()); break;
  case IQ_AUDIO_SOURCE:
    _source = new IQAudioSource(_qrss.Fbfo(), _qrss.width()); break;
  }
  // Connect to QRSS node
  _source->source()->connect(&_agc);
}

QWidget *
Receiver::sourceView() {
  return _source->view();
}

sdr::gui::SpectrumProvider *
Receiver::spectrum() {
  return &_qrss;
}

double
Receiver::bfoFrequency() const {
  return _qrss.Fbfo();
}

void
Receiver::setBFOFrequency(double F) {
  _qrss.setFbfo(F);
  _settings.setValue("Fbfo", F);
}

double
Receiver::dotLength() const {
  return _qrss.dotLength();
}

void
Receiver::setDotLength(double len) {
  _qrss.setDotLength(len);
  _settings.setValue("dotLength", len);
}

double
Receiver::spectrumWidth() const {
  return _qrss.width();
}

void
Receiver::setSpectrumWidth(double width) {
  _qrss.setWidth(width);
  _settings.setValue("width", width);
}

bool
Receiver::agcEnabled() const {
  return _agc.enabled();
}

void
Receiver::enableAGC(bool enabled) {
  _agc.enable(enabled);
  _settings.setValue("agc", enabled);
}

double
Receiver::gain() const {
  return _agc.gain();
}

void
Receiver::setGain(double gain) {
  _agc.setGain(gain);
  _settings.setValue("gain", gain);
}

bool
Receiver::monitor() const {
  return _monitor;
}

void
Receiver::setMonitor(bool enabled) {
  if (enabled && !_monitor) {
    // enable monitoring
    _agc.connect(&_audioSink, true);
  } else if (!enabled && _monitor) {
    _agc.disconnect(&_audioSink);
  }
  _monitor = enabled;
  _settings.setValue("monitor", enabled);
}
