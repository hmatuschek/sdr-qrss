#include "receiver.hh"

Receiver::Receiver(double Fbfo, double dotLen, double width, QObject *parent) :
  QObject(parent), _sourceType(AUDIO_SOURCE), _source(0), _qrss(Fbfo, dotLen, width)
{
  _source = new AudioSource();

}
