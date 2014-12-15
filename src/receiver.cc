#include "receiver.hh"
#include <QLabel>


/* ********************************************************************************************* *
 * Implementation of QRSSSource
 * ********************************************************************************************* */
QRSSSource::QRSSSource(QObject *parent)
  : QObject(parent)
{
  // pass...
}

QRSSSource::~QRSSSource() {
  // pass...
}


/* ********************************************************************************************* *
 * Implementation of AudioSource
 * ********************************************************************************************* */
AudioSource::AudioSource(QObject *parent)
  : QRSSSource(parent), _src(16e3, 256)
{
  // Connect to idle signal of queue
  sdr::Queue::get().addIdle(&_src, &sdr::PortSource<int16_t>::next);
}

AudioSource::~AudioSource()
{
  // unregister idle callbacks
  sdr::Queue::get().remIdle(&_src);
  // delete ctrl view later
  _ctrlView->deleteLater();
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
 * Implementation of Receiver
 * ********************************************************************************************* */
Receiver::Receiver(QObject *parent) :
  QObject(parent), _sourceType(AUDIO_SOURCE), _source(0), _qrss(800, 3, 300)
{
  _source = new AudioSource();
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
  case AUDIO_SOURCE: _source = new AudioSource(); break;
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
