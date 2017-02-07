#ifndef TORCLIENTWINDOW_H
#define TORCLIENTWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QTcpSocket>
#include <QSettings>

#include "Constants.h"

namespace Ui {
class TorClientWindow;
}

class TorClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TorClientWindow(QWidget *parent = 0);
    ~TorClientWindow();

private:
    void loadSettings();
    void saveSettings();

    void toggleStartAction();
    void toggleNewIdentity();

signals:
    void settingsLoaded();
    void settingsSaved();

public slots:
    void onActionTriggered();
    void showStandardOutput();
    void socketReadyRead();
    void socketError(QAbstractSocket::SocketError error);
    void socketConnected();

private:
    Ui::TorClientWindow *ui;
    QMap<QObject*, int> *objects;
    QSettings *settings;

    QTcpSocket *socket;
    QProcess *process;
    QString execPath;
};

#endif // TORCLIENTWINDOW_H
