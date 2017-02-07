#include "TorClientWindow.h"
#include "ui_TorClientWindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QTcpSocket>

TorClientWindow::TorClientWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TorClientWindow),
    objects(new QMap<QObject *, int>())
{
    ui->setupUi(this);

    settings = new QSettings(QApplication::applicationDirPath() + "/settings.ini",
                             QSettings::NativeFormat);

    process = new QProcess(this);

    if (!QDir(QApplication::applicationDirPath() + "/data/").exists())
    {
        QDir().mkdir(QApplication::applicationDirPath() + "/data/");
    }

    if (!settings->value(TOGGLED).isValid())
    {
        settings->setValue(TOGGLED, false);
    }

    if (!settings->value(CONTROL_PORT).isValid())
    {
        settings->setValue(CONTROL_PORT, 5050);
    }

    if (!settings->value(SOCKS_PORT, 9050).isValid())
    {
        settings->setValue(SOCKS_PORT, 9050);
    }

    connect(ui->actionStart, SIGNAL(triggered()), this, SLOT(onActionTriggered()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(onActionTriggered()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(onActionTriggered()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(onActionTriggered()));
    connect(ui->actionNewIdentity, SIGNAL(triggered()), this, SLOT(onActionTriggered()));

    objects->insert(ui->actionStart, 1);
    objects->insert(ui->actionAbout, 2);
    objects->insert(ui->actionExit, 3);
    objects->insert(ui->actionSave, 4);
    objects->insert(ui->actionNewIdentity, 5);

    ui->logBrowser->append("------------------------- Tips -------------------------");
    ui->logBrowser->append("Use toggle button to start/stop the service.");
    ui->logBrowser->append("After setting ports, use save button to save the configs.");
    ui->logBrowser->append("Changing ports will apply at the next service start time.");
    ui->logBrowser->append("--------------------------------------------------------");

    loadSettings();
}

TorClientWindow::~TorClientWindow()
{
    delete ui;
}

void TorClientWindow::loadSettings()
{
    settings->setValue(TOGGLED,
                       QFile(QApplication::applicationDirPath() + "/data/tor.pid").exists());

    if (settings->value(TOGGLED).toBool())
    {
        ui->logBrowser->append("Service has already started.");
    }

    ui->actionStart->setIcon(settings->value(TOGGLED).toBool()
                             ? QIcon(":/images/switch_on.png")
                             : QIcon(":/images/switch_off.png"));

    this->execPath =
#ifdef __APPLE__
                     QApplication::applicationDirPath() + "/Tor/tor.real";
#elif __linux__
                     "Tor/tor"
#else
                     "Tor/tor.exe";
#endif

    ui->controlPortSpinBox->setValue(
                settings->value(CONTROL_PORT).isValid()
                ? settings->value(CONTROL_PORT).toInt()
                : 5050);

    ui->socksPortSpinBox->setValue(
                settings->value(SOCKS_PORT).isValid()
                ? settings->value(SOCKS_PORT).toInt()
                : 9050);
}

void TorClientWindow::saveSettings()
{
    settings->setValue(CONTROL_PORT, ui->controlPortSpinBox->value());
    settings->setValue(SOCKS_PORT, ui->socksPortSpinBox->value());
}

void TorClientWindow::toggleStartAction()
{
    settings->setValue(TOGGLED, !settings->value(TOGGLED).toBool());

    ui->actionStart->setIcon(settings->value(TOGGLED).toBool()
                             ? QIcon(":/images/switch_on.png")
                             : QIcon(":/images/switch_off.png"));

    if (settings->value(TOGGLED).toBool())
    {
        QStringList arguments;

        arguments << "--RunAsDaemon" << "1";
        arguments << "--CookieAuthentication" << "0";
        arguments << "--ControlPort" << QString::number(ui->controlPortSpinBox->value());
        arguments << "--PidFile" << QApplication::applicationDirPath() + "/data/tor.pid";
        arguments << "--SocksPort" << QString::number(ui->socksPortSpinBox->value());
        arguments << "--DataDirectory" << QApplication::applicationDirPath() + "/data/";

        process->start(this->execPath, arguments);
        connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(showStandardOutput()));
    }
    else
    {
        QFile f(QApplication::applicationDirPath() + "/data/tor.pid");

        if (f.exists())
        {
            if (!f.open(QFile::ReadOnly | QFile::Text))
            {
                return;
            }

            QTextStream in(&f);
            int pid = in.readAll().toInt();


#if __APPLE__
            process->start(QString("kill -2 %1").arg(pid));
#elif __linux__
            process->start(QString("kill -2 %1").arg(pid));
#else
            process->start(QString("Taskkill /PID %2 /F").arg(pid));
#endif
        }
        else
        {
#if __APPLE__
            process->start("killall -9 tor.real");
#elif __linux__
            process->start("killall -9 tor");
#else
            process->start("Taskkill /IM tor.exe /F");
#endif
        }
    }
}

void TorClientWindow::toggleNewIdentity()
{
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    socket->connectToHost("127.0.0.1", settings->value(CONTROL_PORT).toInt());
}

void TorClientWindow::onActionTriggered()
{
    switch(objects->value(sender()))
    {
        case 1: // start
        {
            toggleStartAction();
            break;
        }
        case 2: // about
        {
            break;
        }
        case 3: // exit
        {
            exit(EXIT_SUCCESS);
            break;
        }
        case 4: // save
        {
            saveSettings();
            break;
        }
        case 5:
        {
            toggleNewIdentity();
            break;
        }
        default:
        {
            break;
        }
    }
}

void TorClientWindow::showStandardOutput()
{
    QString output = process->readAllStandardOutput();
    if (!output.trimmed().isEmpty())
    {
        ui->logBrowser->append(output.trimmed());
    }
}

void TorClientWindow::socketReadyRead()
{
    ui->logBrowser->append("Socket: " + QString(socket->readAll()));
}

void TorClientWindow::socketError(QAbstractSocket::SocketError error)
{
    int iError = (int) error;
    ui->logBrowser->append(QString("SocketError: %1").arg(iError));
}

void TorClientWindow::socketConnected()
{
    ui->logBrowser->append("Connecting to control port...");
    socket->write(QString("AUTHENTICATE \"\"\r\n").toLocal8Bit());
    ui->logBrowser->append("AUTHENTICATE...");
    socket->write(QString("SIGNAL NEWNYM\r\n").toLocal8Bit());
    ui->logBrowser->append("Sending SIGNAL NEWNYM...");
    socket->write(QString("QUIT\r\n").toLocal8Bit());
    ui->logBrowser->append("QUIT");
    socket->close();
}
