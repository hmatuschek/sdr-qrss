#include "qrss.hh"

using namespace sdr;

QRSS::QRSS(double Fbfo, double dotlen, double width):
  gui::SpectrumProvider(), sdr::Sink<int16_t>(), _Fbfo(Fbfo), _dotlen(dotlen), _width(width),
  _samplerate(0), _fshift(0,0), _subsample(0),
  _N_fft(0), _fft_count(0), _fft_in(0), _fft_out(0), _fft(0), _currPSD()
{
  // pass...
}


QRSS::~QRSS() {
  if (0 != _fft) { delete _fft; }
}

bool
QRSS::isInputReal() const {
  return false;
}

double
QRSS::sampleRate() const {
  return _samplerate;
}

size_t
QRSS::fftSize() const {
  return _N_fft;
}

const Buffer<double> &
QRSS::spectrum() const {
  return _currPSD;
}


void
QRSS::config(const Config &src_cfg) {
  // Requires type, sample-rate and buffer size
  if (!src_cfg.hasType() || ! src_cfg.hasSampleRate() || !src_cfg.hasBufferSize()) { return; }

  // check buffer type
  if (Config::typeId<int16_t>() != src_cfg.type()) {
    ConfigError err;
    err << "Can not configure QRSS node: Invalid buffer type " << src_cfg.type()
        << ", expected " << Config::typeId<int16_t>();
    throw err;
  }

  // Config frequency shift
  _samplerate = src_cfg.sampleRate();
  _fshift.setFrequencyShift(-_Fbfo);
  _fshift.setSampleRate(_samplerate);

  // Trigger reconfig of spectrum
  configSpectrum();
}

void
QRSS::configSpectrum()
{
  // Skip config on incomplete data
  if (0 == _samplerate) { return; }
  if (0 == _width) { return; }
  if (0 == _dotlen) { return; }

  // Compute sub-sampling
  _subsample = _samplerate/_width;
  _curr_avg = 0; _avg_count = 0; _N_fft = 0;

  // Compute samples per spectrum with FFT period dotlen/2
  _N_fft = _dotlen*_samplerate/(2*_subsample);
  _fft_count = 0;
  // Construct FFT
  _fft_in = Buffer< std::complex<float> >(_N_fft);
  _fft_out = Buffer< std::complex<float> >(_N_fft);
  _currPSD = Buffer< double >(_N_fft);
  if (0 != _fft) { delete _fft; }
  _fft = new FFTPlan<float>(_fft_in, _fft_out, FFT::FORWARD);

  LogMessage msg(LOG_DEBUG);
  msg << "Configure QRSS node:" << std::endl
      << " F_bfo: " << _Fbfo << std::endl
      << " Sample rate: " << _samplerate << std::endl
      << " Spectrum width: " << _width << " Hz" << std::endl
      << " Refresh period: " << _N_fft*_subsample/_samplerate << "s" << std::endl
      << " Sub-sample: " << _subsample << std::endl
      << " FFT length: " << _N_fft << std::endl
      << " Freq. res: " << _samplerate/(_subsample*_N_fft) << "Hz";
  Logger::get().log(msg);

  emit spectrumConfigured();
}


void
QRSS::process(const Buffer<int16_t> &buffer, bool allow_overwrite) {
  for (size_t i=0; i<buffer.size(); i++)
  {
    // Shift frequency and sub-sample by 16 (for spectrum)
    _curr_avg += _fshift.applyFrequencyShift(buffer[i]); _avg_count++;
    if (_subsample == _avg_count) {
      _fft_in[_fft_count] = _curr_avg/float( _subsample * (1<<15) );
      _curr_avg=0; _avg_count=0; _fft_count++;
    }

    // If _N_fft samples have been received -> update spectrum
    if (_N_fft == _fft_count) {
      _fft_count = 0;
      // Compute FFT
      (*_fft)();
      // Compute PSD
      for (size_t j=0; j<_N_fft; j++) {
        _currPSD[j] = _fft_out[j].real()*_fft_out[j].real()
            + _fft_out[j].imag()*_fft_out[j].imag();
      }

      // Notify spectrum views about the new spectrum.
      emit spectrumUpdated();
    }
  }
}


double
QRSS::Fbfo() const {
  return _Fbfo;
}

void
QRSS::setFbfo(double F) {
  _Fbfo = F;
  _fshift.setFrequencyShift(-F);
}

double
QRSS::dotLength() const {
  return _dotlen;
}

void
QRSS::setDotLength(double len) {
  _dotlen = len;
  configSpectrum();
}

double
QRSS::width() const {
  return _width;
}

void
QRSS::setWidth(double width) {
  _width = width;
  configSpectrum();
}

