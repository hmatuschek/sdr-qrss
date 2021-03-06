#ifndef MAINWINDOW_HH
#define MAINWINDOW_HH

#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTimer>

#include "receiver.hh"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(Receiver *rx, QWidget *parent = 0);

protected slots:
  void onQueueStartStop(bool start);
  void onSourceSelected(int idx);
  void onBFOFreqChanged();
  void onDotLengthChanged();
  void onWidthChanged();
  void onAGCToggled(bool enabled);
  void onGainChanged();
  void onGainUpdate();
  void onMonitorToggled(bool enabled);

protected:
  Receiver *_receiver;
  QPushButton *_queueStartStop;
  QVBoxLayout *_sourceLayout;
  QComboBox *_sourceSelect;
  QLineEdit *_Fbfo;
  QLineEdit *_dotLen;
  QLineEdit *_width;
  QLineEdit *_gain;
  QTimer    _gainTimer;
};

#endif // MAINWINDOW_HH
