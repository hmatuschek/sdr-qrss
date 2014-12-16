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

  /** Set the BFO frequency. This method can be overridden by sub-classes to
   * update filters. */
  virtual void setBFOFrequency(double F);
  /** Set the spectrum width. This method can be overridden by sub-classes to
   * update filters. */
  virtual void setSpectrumWidth(double width);

protected:
  /** The current BFO frequency. */
  double _Fbfo;
  /** The current spectrum width. */
  double _width;
};


/** The sound card source. */
class AudioSource: public QRSSSource
{
  Q_OBJECT

public:
  /** Constructor. */
  explicit AudioSource(double Fbfo, double width, QObject *parent=0);
  /** Destructor. */
  virtual ~AudioSource();

  virtual sdr::Source *source();
  virtual QWidget *view();

protected slots:
  void onViewDeleted();

protected:
  /** The actual SDR audio source. */
  sdr::PortSource<int16_t> _src;
  /** Holds a reference to the ctrl view. */
  QWidget *_ctrlView;
};


/** Sound card IQ input. */
class IQAudioSource: public QRSSSource
{
  Q_OBJECT

public:
  /** Constructor. */
  explicit IQAudioSource(double Fbfo, double width, QObject *parent=0);
  /** Destructor. */
  virtual ~IQAudioSource();

  virtual void setBFOFrequency(double F);
  virtual void setSpectrumWidth(double width);

  virtual sdr::Source *source();
  virtual QWidget *view();

protected slots:
  void onViewDeleted();

protected:
  /** The audio imput source. */
  sdr::PortSource< std::complex<int16_t> > _src;
  /** A filter around the BFO frequency. */
  sdr::IQBaseBand<int16_t> _filter;
  /** A SSB demodulator. */
  sdr::USBDemod<int16_t> _demod;
  /** A reference to the ctrl view. */
  QWidget *_ctrlView;
};


/** Central controller class. */
class Receiver : public QObject
{
  Q_OBJECT

public:
  /** Possible input sources. */
  typedef enum {
    AUDIO_SOURCE,    ///< Real audio input source.
    IQ_AUDIO_SOURCE  ///< IQ audio input source.
  } SourceType;

public:
  /** Constructor. */
  explicit Receiver(QObject *parent = 0);
  /** Destructor. */
  virtual ~Receiver();

  /** Returns the currenly selected input source. */
  SourceType sourceType() const;
  /** Sets the current input source. */
  void setSourceType(SourceType source);
  /** Creates a control view for the current input source. */
  QWidget *sourceView();

  /** Returns the spectrum provider. */
  sdr::gui::SpectrumProvider *spectrum();

  /** Returns the current BFO frequency (Hz). */
  double bfoFrequency() const;
  /** Sets the BFO frequency. */
  void setBFOFrequency(double F);
  /** Returns the current dot length (s). */
  double dotLength() const;
  /** Sets the current dot length (s). */
  void setDotLength(double len);
  /** Returns the current spectrum width (Hz). */
  double spectrumWidth() const;
  /** Sets the spectrum width (Hz). */
  void setSpectrumWidth(double width);
  /** Returns @c true if audio monitoring is enabled. */
  bool monitor() const;
  /** Enables/Disables audio monitoring. */
  void setMonitor(bool enabled);

protected:
  /** The currently selected source type. */
  SourceType _sourceType;
  /** The currently selected source instance. */
  QRSSSource *_source;
  /** QRSS "demodulator" instance. */
  sdr::QRSS _qrss;
  /** If true, audio monitoring is enabled. */
  bool _monitor;
  /** Audio monitor sink. */
  sdr::PortSink _audioSink;
};

#endif // RECEIVER_HH
