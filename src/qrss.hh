#ifndef __SDR_QRSS__
#define __SDR_QRSS__

#include <freqshift.hh>
#include <gui/spectrum.hh>


namespace sdr {

/** Spectrum provider, extracts a spectrum +/- width (Hz) around the specified BFO frequency. */
class QRSS: public gui::SpectrumProvider, public sdr::Sink<int16_t>
{
  Q_OBJECT

public:
  /** Constructor.
   * @param Fbfo Specifies the BFO frequency in Hz.
   * @param dotlen Specifies "dot length" in s. Each FFT frame will be obtained from @c Fs*dotlen/2
   *        samples. Hence by increasing the dot length, the frequency resolution will be
   *        increased.
   * @param width Specifies the width of the visible spectrum round the BFO frequency in Hz. */
  QRSS(double Fbfo=800, double dotlen=3, double width=200);
  /** Destructor. */
  virtual ~QRSS();

  /** Implements the SpectrumProvider interface. */
  bool isInputReal() const;
  /** Implements the SpectrumProvider interface. */
  double sampleRate() const;
  /** Implements the SpectrumProvider interface. */
  size_t fftSize() const;
  /** Implements the SpectrumProvider interface. */
  const Buffer<double> & spectrum() const;

  /** Configures the node. */
  virtual void config(const Config &src_cfg);
  /** Processes the given buffer. */
  virtual void process(const Buffer<int16_t> &buffer, bool allow_overwrite);

  /** Returns the BFO frequency. */
  double Fbfo() const;
  /** Sets the BFO frequency. */
  void setFbfo(double F);

  /** Returns the dot length in s. */
  double dotLength() const;
  /** Sets the dot length in s. */
  void setDotLength(double len);
  /** Returns the spectrum width in Hz. */
  double width() const;
  /** Sets the spectrum width in Hz. */
  void setWidth(double width);

protected:
  /** (Re-) Configures the spectrum. */
  void configSpectrum();

protected:
  /** BFO frequency. */
  double _Fbfo;
  /** Length of the QRSS dot. */
  double _dotlen;
  /** Width of the spectrum. */
  double _width;
  /** The current input sample-rate. */
  double _samplerate;
  /** Removes the _Fbfo from the input signal. */
  FreqShiftBase<int16_t> _fshift;
  /** Sub-sample factor. */
  size_t _subsample;
  /** Current average value of the down-sampler. */
  std::complex<float> _curr_avg;
  /** Current average count of the down-sampler. */
  size_t _avg_count;

  /** Size of the FFT buffers. */
  size_t _N_fft;
  /** The number of samples in the @c _fft_in buffer. */
  size_t _fft_count;
  /** The fft input buffer. */
  Buffer< std::complex<float> > _fft_in;
  /** The output buffer of the FFT. */
  Buffer< std::complex<float> > _fft_out;
  /** The FFT plan. */
  FFTPlan<float> *_fft;
  /** The current PSD. */
  Buffer<double> _currPSD;
};


}
#endif // __SDR_QRSS__
