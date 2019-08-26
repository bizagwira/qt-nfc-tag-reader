#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QMutex>

class SerialPort;

namespace Ui {
class MainWindow;
}

class MainWindow;

typedef void (MainWindow::*func_ptr)(void);

typedef struct{
    QString description;
    func_ptr fp;
}GatewayPairPointer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);

        ~MainWindow();

    signals:
        void fail_replay();

    public slots:
        void handleResponse(QString datatext);

        void sendquery(QString command);

    private:

        void parm_extracter_init();

        void fillPortsParameters();

        void updateSettings();

        void pid_extracter();

        void sid_extracter();

        void pac_extracter();

        void mac_extracter();

        void swid_extracter();

        void tx_rssi_extracter();

        void extract_dut_parm();

    private slots:
        void on_connect_button_clicked();

        void on_deconnect_button_clicked();

        void on_clear_button_clicked();

        void on_quit_button_clicked();

        void display (QString text);

        void connect_to_nfc_tag();

    private:
        Ui::MainWindow *ui;
        SerialPort *_serialport;
        QString _current_command;
        QTimer* _test_running_timer ;
        QMutex _mutex;
        bool _connected;
        QString _mac, _pid, _sid, _swid, _pac, _replay_text;
        QList<GatewayPairPointer> _parm_parser_list;
        double _tx_rssi;
};

#endif // MAINWINDOW_H
