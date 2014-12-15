#include "mainwindow.hh"
#include "libsdr/gui/waterfallview.hh"

#include <QSplitter>
#include <QGroupBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QDoubleValidator>


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
  _sourceLayout->addWidget(_sourceSelect);
  _sourceLayout->addWidget(_receiver->sourceView());

  QGroupBox *cfgBox = new QGroupBox("Settings");
  spLayout->addWidget(cfgBox);
  QFormLayout *cfgLayout = new QFormLayout();
  cfgBox->setLayout(cfgLayout);

  _Fbfo = new QLineEdit("800");
  cfgLayout->addRow("BFO Freq. (Hz)", _Fbfo);

  setCentralWidget(splitter);

  QObject::connect(_queueStartStop, SIGNAL(toggled(bool)), this, SLOT(onQueueStartStop(bool)));
  QObject::connect(_sourceSelect, SIGNAL(currentIndexChanged(int)), this, SLOT(onSourceSelected(int)));
  QObject::connect(_Fbfo, SIGNAL(returnPressed()), this, SLOT(onBFOFreqChanged()));
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
  _receiver->setSourceType(src);
  _sourceLayout->addWidget(_receiver->sourceView());
}

void
MainWindow::onBFOFreqChanged() {
  /// @todo Implement.
}
