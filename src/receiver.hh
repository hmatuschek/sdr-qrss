#ifndef RECEIVER_HH
#define RECEIVER_HH

#include <QObject>
#include "qrss.hh"
#include <libsdr/baseband.hh>


/** Abstract base class of all sources. */
class QRSSSource: public QObject
{
  Q_OBJECT

public:
  /** Constructor. */
  explicit QRSSSource(double Fbfo, double width, QObject *parent=0);
  /** Destructor. */
  virtual ~QRSSSource();

  /** Returns the SDR source node. */
  virtual sdr::Source *source() = 0;
  /** Returns the control view of the source. */
  virtual QWidget *view() = 0;

  virtual void setBFOFrequency(double F);
  virtual void setSpectrumWidth(double width);

protected:
  double _Fbfo;
  double _width;
};


/** The sound card source. */
class AudioSource: public QRSSSource
{
  Q_OBJECT

public:
  explicit AudioSource(double Fbfo, double width, QObject *parent=0);
  virtual ~AudioSource();

  virtual sdr::Source *source();
  virtual QWidget *view();

protected:
  sdr::PortSource<int16_t> _src;
  QWidget *_ctrlView;
};


/** Sound card IQ input. */
class IQAudioSource: public QRSSSource
{
  Q_OBJECT

public:
  explicit IQAudioSource(double Fbfo, double width, QObject *parent=0);
  virtual ~IQAudioSource();

  virtual void setBFOFrequency(double F);
  virtual void setSpectrumWidth(double width);

  virtual sdr::Source *source();
  virtual QWidget *view();

protected:
  sdr::PortSource< std::complex<int16_t> > _src;
  sdr::IQBaseBand<int16_t> _filter;
  sdr::USBDemod<int16_t> _demod;
  QWidget *_ctrlView;
};


class Receiver : public QObject
{
  Q_OBJECT

public:
  typedef enum {
    AUDIO_SOURCE,
    IQ_AUDIO_SOURCE
  } SourceType;

public:
  explicit Receiver(QObject *parent = 0);
  virtual ~Receiver();

  SourceType sourceType() const;
  void setSourceType(SourceType source);
  QWidget *sourceView();

  sdr::gui::SpectrumProvider *spectrum();

  double bfoFrequency() const;
  void setBFOFrequency(double F);
  double dotLength() const;
  void setDotLength(double len);
  double spectrumWidth() const;
  void setSpectrumWidth(double width);

protected:
  SourceType _sourceType;
  QRSSSource *_source;
  sdr::QRSS _qrss;
};

#endif // RECEIVER_HH
