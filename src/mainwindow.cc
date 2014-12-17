#include "mainwindow.hh"
#include "libsdr/gui/waterfallview.hh"

#include <QSplitter>
#include <QGroupBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QCheckBox>


MainWindow::MainWindow(Receiver *rx, QWidget *parent) :
  QMainWindow(parent), _receiver(rx)
{
  setWindowTitle("SDR-QRSS");

  QSplitter *splitter = new QSplitter();
  splitter->addWidget(new sdr::gui::WaterFallView(_receiver->spectrum(), 800,
                                                  sdr::gui::WaterFallView::RIGHT_LEFT));

  QWidget *sidepanel = new QWidget();
  splitter->addWidget(sidepanel);

  QVBoxLayout *spLayout = new QVBoxLayout();
  sidepanel->setLayout(spLayout);
  _queueStartStop = new QPushButton();
  spLayout->addWidget(_queueStartStop);
  _queueStartStop->setCheckable(true);
  if (sdr::Queue::get().isRunning()) {
    _queueStartStop->setText("Stop");
    _queueStartStop->setChecked(true);
  } else {
    _queueStartStop->setText("Start");
    _queueStartStop->setChecked(false);
  }

  QGroupBox *sourceBox = new QGroupBox("Source");
  spLayout->addWidget(sourceBox);
  _sourceLayout = new QVBoxLayout();
  sourceBox->setLayout(_sourceLayout);

  _sourceSelect = new QComboBox();
  _sourceSelect->addItem("Audio", Receiver::AUDIO_SOURCE);
  _sourceSelect->addItem("IQ Audio", Receiver::IQ_AUDIO_SOURCE);
  _sourceLayout->addWidget(_sourceSelect);
  _sourceLayout->addWidget(_receiver->sourceView());

  QGroupBox *cfgBox = new QGroupBox("Settings");
  spLayout->addWidget(cfgBox);
  QFormLayout *cfgLayout = new QFormLayout();
  cfgBox->setLayout(cfgLayout);

  _Fbfo = new QLineEdit(QString::number(_receiver->bfoFrequency()));
  QDoubleValidator *FbfoVal = new QDoubleValidator();
  _Fbfo->setValidator(FbfoVal);
  cfgLayout->addRow("BFO freq. (Hz)", _Fbfo);

  _dotLen = new QLineEdit(QString::number(_receiver->dotLength()));
  QDoubleValidator *dotLenVal = new QDoubleValidator();
  dotLenVal->setBottom(0.01); _dotLen->setValidator(dotLenVal);
  cfgLayout->addRow("Dot length (s)", _dotLen);

  _width = new QLineEdit(QString::number(_receiver->spectrumWidth()));
  QDoubleValidator *widthVal = new QDoubleValidator();
  widthVal->setBottom(10); widthVal->setTop(1000);
  _width->setValidator(widthVal);
  cfgLayout->addRow("Spec. width (Hz)", _width);

  QCheckBox *agc = new QCheckBox("AGC");
  agc->setChecked(_receiver->agcEnabled());
  _gain = new QLineEdit(QString::number(10*std::log10(_receiver->gain())));
  QDoubleValidator *gainVal = new QDoubleValidator();
  gainVal->setBottom(0); _gain->setValidator(gainVal);
  _gain->setEnabled(!_receiver->agcEnabled());
  _gainTimer.setInterval(500);
  _gainTimer.setSingleShot(false);

  QVBoxLayout *gainLayout = new QVBoxLayout();
  gainLayout->addWidget(agc);
  gainLayout->addWidget(_gain);
  cfgLayout->addRow("Gain [dB]", gainLayout);

  QCheckBox *monitor = new QCheckBox();
  monitor->setChecked(_receiver->monitor());
  cfgLayout->addRow("Audio monitor", monitor);

  setCentralWidget(splitter);

  QObject::connect(_queueStartStop, SIGNAL(toggled(bool)), this, SLOT(onQueueStartStop(bool)));
  QObject::connect(_sourceSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(onSourceSelected(int)));
  QObject::connect(_Fbfo, SIGNAL(returnPressed()), this, SLOT(onBFOFreqChanged()));
  QObject::connect(_dotLen, SIGNAL(returnPressed()), this, SLOT(onDotLengthChanged()));
  QObject::connect(_width, SIGNAL(returnPressed()), this, SLOT(onWidthChanged()));
  QObject::connect(agc, SIGNAL(toggled(bool)), this, SLOT(onAGCToggled(bool)));
  QObject::connect(_gain, SIGNAL(returnPressed()), this, SLOT(onGainChanged()));
  QObject::connect(monitor, SIGNAL(toggled(bool)), this, SLOT(onMonitorToggled(bool)));
  QObject::connect(&_gainTimer, SIGNAL(timeout()), this, SLOT(onGainUpdate()));

  if (_receiver->agcEnabled()) { _gainTimer.start(); }
}

void
MainWindow::onQueueStartStop(bool start) {
  if (start && !sdr::Queue::get().isRunning()) {
    sdr::Queue::get().start();
    _queueStartStop->setText("Stop");
  } else if (!start && sdr::Queue::get().isRunning()) {
    sdr::Queue::get().stop();
    sdr::Queue::get().wait();
    _queueStartStop->setText("Start");
  }
}

void
MainWindow::onSourceSelected(int idx) {
  Receiver::SourceType src = Receiver::SourceType(_sourceSelect->itemData(idx).toUInt());
  bool isRunning = sdr::Queue::get().isRunning();
  if (isRunning) { sdr::Queue::get().stop(); sdr::Queue::get().wait(); }
  _receiver->setSourceType(src);
  _sourceLayout->addWidget(_receiver->sourceView());
  if (isRunning) { sdr::Queue::get().start(); }
}

void
MainWindow::onBFOFreqChanged() {
  _receiver->setBFOFrequency(_Fbfo->text().toDouble());
}

void
MainWindow::onDotLengthChanged() {
  _receiver->setDotLength(_dotLen->text().toDouble());
}

void
MainWindow::onWidthChanged() {
  _receiver->setSpectrumWidth(_width->text().toDouble());
}

void
MainWindow::onAGCToggled(bool enabled) {
  _receiver->enableAGC(enabled);
  _gain->setEnabled(!enabled);
  if (enabled) { _gainTimer.start(); }
  else { _gainTimer.stop(); }
}

void
MainWindow::onGainChanged() {
  _receiver->setGain(std::pow(10, _gain->text().toDouble()/10));
}

void
MainWindow::onGainUpdate() {
  _gain->setText(QString::number(10*std::log10(_receiver->gain())));
}

void
MainWindow::onMonitorToggled(bool enabled) {
  _receiver->setMonitor(enabled);
}
