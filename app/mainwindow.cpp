#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPort>
#include <QDateTime>
#include <QMutex>
#include <QDebug>
#include "loggers.h"
#include "tools.h"
#include "serialport.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    CREATE_LOG_FILE()

    parm_extracter_init();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));
    fillPortsParameters();
    ui->deconnect_button->hide();

    _connected = false;
    _test_running_timer = new QTimer(this);
    QObject::connect(_test_running_timer, SIGNAL(timeout()), this, SLOT(connect_to_nfc_tag()));
    _test_running_timer->start(5000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::fillPortsParameters(){
    ui->baudrate_cbx->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudrate_cbx->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudrate_cbx->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudrate_cbx->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudrate_cbx->addItem(tr("Custom"));

    ui->parity_cbx->addItem(tr("None"), QSerialPort::NoParity);
    ui->parity_cbx->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parity_cbx->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parity_cbx->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parity_cbx->addItem(tr("Space"), QSerialPort::SpaceParity);
}

void
MainWindow::updateSettings(){
    Tools::gserialportsettings.portname = ui->portname_led->text();

    if (ui->baudrate_cbx->currentIndex() == 4) {
        Tools::gserialportsettings.baudrate = ui->baudrate_cbx->currentText().toInt();
    }
    else {
        Tools::gserialportsettings.baudrate = static_cast<QSerialPort::BaudRate>(
                    ui->baudrate_cbx->itemData(ui->baudrate_cbx->currentIndex()).toInt());
    }
    Tools::gserialportsettings.stringBaudRate = QString::number(Tools::gserialportsettings.baudrate);

    Tools::gserialportsettings.parity = static_cast<QSerialPort::Parity>(
                ui->parity_cbx->itemData(ui->parity_cbx->currentIndex()).toInt());
    Tools::gserialportsettings.stringParity = ui->parity_cbx->currentText();
}

void
MainWindow::connect_to_nfc_tag(){
//    qDebug () << "|" + QDateTime::currentDateTime().toString(Qt::ISODate)+"|"
//              << "\t" << __LINE__ << ".\t" << __func__<< "\t"
//              << __FILE__;
    if (_connected)
        sendquery("CONNECT=NFC");
}

void
MainWindow::on_connect_button_clicked(){
    ui->connect_button->hide();
    ui->deconnect_button->setVisible(true);

    updateSettings();

    //! Open the connection
    _serialport = new SerialPort (&Tools::gserialportsettings, this);
    if (_serialport->openSerialPort()){
        QObject::connect(_serialport, SIGNAL(data_avalaible (QString)),
                         this, SLOT(handleResponse(QString)));

        LOG_INFO (QString("%1 serial port is avalaible and successful opened.")
                  .arg(Tools::gserialportsettings.portname));
        _connected = true;
    }
    else{
        LOG_FATAL (QString("Faild to open the serial port for %1.")
                   .arg(Tools::gserialportsettings.portname));
    }
}

void
MainWindow::on_deconnect_button_clicked(){
    _connected = false;
    ui->deconnect_button->hide();
    ui->connect_button->setVisible(true);
    _test_running_timer->stop();
    _test_running_timer->deleteLater();
    delete(_serialport);
}

void
MainWindow::on_clear_button_clicked(){
    ui->console_pte->clear();
}

void
MainWindow::on_quit_button_clicked(){
    emit qApp->lastWindowClosed();
}

void
MainWindow::display(QString text){
    QString timestamp = "|" + QDateTime::currentDateTime().toString(Qt::ISODate)+"|";
    QString line = QString ("$ %1 %2").arg(timestamp).arg(text.replace(QRegExp("[\n\r]"), ""));

    LOG_DEBUG (line)
    ui->console_pte->appendPlainText(line);
}

void
MainWindow::handleResponse(QString datatext){

    if(datatext.contains("BAD CMD", Qt::CaseInsensitive)
            || datatext.contains("DISCONNECTED", Qt::CaseInsensitive)
            || datatext.contains("ERROR", Qt::CaseInsensitive)){
        emit fail_replay();
    }
    else{
        _replay_text = datatext;
        extract_dut_parm();
    }
    display(datatext);
}

void
MainWindow::sendquery(QString command){

    QTimer timer;
    timer.setInterval(3000);
    timer.setSingleShot(true);

    if (ui->carriage_return_ckbx->isChecked() ){
        _current_command = QString("%1\r").arg(command);
    }
    else {
        _current_command = QString("%1").arg(command);
    }

    QEventLoop loop;
//    loop.connect(_serialport, SIGNAL(request_timeout()),
//                 &loop, SLOT(quit()));

    loop.connect(this, SIGNAL(fail_replay()),
                 &loop, SLOT(quit()));


//    QObject::connect(&timer, SIGNAL(timeout()), &loop,
//                     SLOT(quit()), Qt::QueuedConnection);
    timer.start();
    QMutexLocker lock(&_mutex);
    _connected = false;
    _serialport->writeSerial(_current_command);
    lock.unlock();
    loop.exec();

    timer.stop();
    _connected = true;

}

void
MainWindow::parm_extracter_init(){
    _parm_parser_list.push_back(GatewayPairPointer{"Extracts the PID",
                                                   &MainWindow::pid_extracter});
    _parm_parser_list.push_back(GatewayPairPointer{"Extracts the SID",
                                                   &MainWindow::sid_extracter});
    _parm_parser_list.push_back(GatewayPairPointer{"Extracts the PAC",
                                                   &MainWindow::pac_extracter});
    _parm_parser_list.push_back(GatewayPairPointer{"Extracts the MAC",
                                                   &MainWindow::mac_extracter});
    _parm_parser_list.push_back(GatewayPairPointer{"Extracts the SWID",
                                                   &MainWindow::swid_extracter});
    _parm_parser_list.push_back(GatewayPairPointer{"Extracts the TX RSSI",
                                                   &MainWindow::tx_rssi_extracter});
}

void
MainWindow::swid_extracter(){

    const QString searching_pattern = "(SWID?:)\\s*(0[xX][a-fA-F0-9]{12})";
    const QString swid_pattern = "([a-fA-F0-9]{12})";
    QString match_substr = Tools::match_pattern_substr(_replay_text, searching_pattern);
    if (!match_substr.isEmpty()){
        _swid =Tools::substr_extracter(match_substr, swid_pattern);
        ui->small_console_pte->appendPlainText(QString("SWID: %1").arg(_swid));
    }
}

void
MainWindow::mac_extracter(){
    const QString searching_pattern = "(TO?)\\s*([a-fA-F0-9]{2}[:]){5}[0-9A-Fa-f]{2}";
    const QString mac_pattern = "(([a-fA-F0-9]{2}[:]){5}[0-9A-Fa-f]{2})$";
    QString match_substr = Tools::match_pattern_substr(_replay_text, searching_pattern);
    if (!match_substr.isEmpty()){
        _mac = Tools::substr_extracter(match_substr, mac_pattern);
        ui->small_console_pte->appendPlainText(QString("MAC: %1").arg(_mac));
    }
}

void
MainWindow::pac_extracter(){
    const QString searching_pattern = "(PAC?:)\\s*(0[xX][a-fA-F0-9]{16,32})";
    const QString pac_pattern = "([a-fA-F0-9]{16,32})";
    QString match_substr = Tools::match_pattern_substr(_replay_text, searching_pattern);

    if (!match_substr.isEmpty()){
        _pac =Tools::substr_extracter(match_substr, pac_pattern);
        ui->small_console_pte->appendPlainText(QString("PAC: %1").arg(_pac));

    }
}

void
MainWindow::sid_extracter(){
    const QString searching_pattern = "(SID?:)\\s*(0[xX][a-fA-F0-9]{8})";
    const QString sid_pattern = "([a-fA-F0-9]{8})";
    QString match_substr = Tools::match_pattern_substr(_replay_text, searching_pattern);

    if (!match_substr.isEmpty()){
        _sid =Tools::substr_extracter(match_substr, sid_pattern);
        ui->small_console_pte->appendPlainText(QString("SID: %1").arg(_sid));
    }
}

void
MainWindow::pid_extracter(){
    const QString searching_pattern = "(PID?:)\\s*(0[xX][a-fA-F0-9]{8})";
    const QString pid_pattern = "([a-fA-F0-9]{8})";
    QString match_substr = Tools::match_pattern_substr(_replay_text, searching_pattern);

    if (!match_substr.isEmpty()){
        ui->small_console_pte->appendPlainText("**************************************");
        _pid =Tools::substr_extracter(match_substr, pid_pattern);
        ui->small_console_pte->appendPlainText(QString("PID: %1").arg(_pid));
    }
}

void
MainWindow::tx_rssi_extracter(){
    const QString searching_pattern = "(RSSI?)\\s*[=][+-]{0,1}[0-9]{2,3}";
    const QString rssi_pattern = "([+-]{0,1}[0-9]{2,3})";
    QString match_substr = Tools::match_pattern_substr(_replay_text, searching_pattern);

    if (!match_substr.isEmpty()){
        _tx_rssi = Tools::substr_extracter(match_substr, rssi_pattern).toDouble();
        ui->small_console_pte->appendPlainText(QString("RSSI: %1").arg(_tx_rssi));
    }
}
void
MainWindow::extract_dut_parm(){
    foreach (const GatewayPairPointer func, _parm_parser_list) {
        (this->*func.fp)();
    }
}
