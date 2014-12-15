#ifndef RECEIVER_HH
#define RECEIVER_HH

#include <QObject>
#include "qrss.hh"


/** Abstract base class of all sources. */
class QRSSSource: public QObject
{
  Q_OBJECT

public:
  /** Constructor. */
  explicit QRSSSource(QObject *parent=0);
  /** Destructor. */
  virtual ~QRSSSource();

  /** Returns the SDR source node. */
  virtual sdr::Source *source() = 0;
  /** Returns the control view of the source. */
  virtual QWidget *view() = 0;
};


/** The sound card source. */
class AudioSource: public QRSSSource
{
  Q_OBJECT

public:
  explicit AudioSource(QObject *parent=0);
  virtual ~AudioSource();

  virtual sdr::Source *source();
  virtual QWidget *view();

protected:
  QWidget *_ctrlView;
};



class Receiver : public QObject
{
  Q_OBJECT

public:
  typedef enum {
    AUDIO_SOURCE
  } SourceType;

public:
  explicit Receiver(double Fbfo=800, double dotLen=3, double width=300, QObject *parent = 0);
  virtual ~Receiver();

  SourceType sourceType() const;
  void setSourceType(SourceType source);
  QWidget *sourceView();

  sdr::gui::SpectrumProvider *spectrum();

protected:
  SourceType _sourceType;
  QRSSSource *_source;
  sdr::QRSS _qrss;
};

#endif // RECEIVER_HH
