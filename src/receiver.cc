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
  }
  return _ctrlView;
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
  }
  return _ctrlView;
}


/* ********************************************************************************************* *
 * Implementation of Receiver
 * ********************************************************************************************* */
Receiver::Receiver(QObject *parent) :
  QObject(parent), _sourceType(AUDIO_SOURCE), _source(0), _qrss(800, 3, 300)
{
  _source = new AudioSource(_qrss.Fbfo(), _qrss.width());
  _source->source()->connect(&_qrss);
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
  _source->source()->connect(&_qrss);
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
}

double
Receiver::dotLength() const {
  return _qrss.dotLength();
}

void
Receiver::setDotLength(double len) {
  _qrss.setDotLength(len);
}

double
Receiver::spectrumWidth() const {
  return _qrss.width();
}

void
Receiver::setSpectrumWidth(double width) {
  _qrss.setWidth(width);
}
