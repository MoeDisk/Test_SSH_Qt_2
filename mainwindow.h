#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:

    void on_selectFileButton_clicked();
    void on_transferButton_clicked();

    void on_username_ssh_1_textChanged(const QString &arg1);
    void on_password_ssh_1_textChanged(const QString &arg1);

    void on_filepath_ssh_1_textChanged(const QString &arg1);

    void on_ipadd1_textChanged(const QString &arg1);

    void on_excutepath_ssh_1_textChanged(const QString &arg1);

    void on_excuteButton_clicked();

    void on_testButton_clicked();

    void on_adbconnect_Button_clicked();

    void on_ipadd2_textChanged(const QString &arg1);

    void on_adbkillButton_clicked();

    void on_adbpushButton_clicked();

    void on_CheckAll_stateChanged(int arg1);

    void on_test2button_clicked();

    void on_adbshellButton_clicked();

    void on_logsaveButton_clicked();

private:
    Ui::MainWindow *ui;

    QProcess pscpProcess;
    QProcess excuteProcess;
    QProcess testProcess;
    QProcess adbProcess;
    QProcess mysqlProcess;
    QProcess killallProcess;
    QVector<QString> ipVector;
    QString localFilePath;
    QString remoteHost = "";
    QString remoteHost2 = "";
    QString username = "";
    QString password = "";
    QString remoteDirectory="";
    QString remoteExcutePath="";

    QStatusBar *statusBar;
};
#endif // MAINWINDOW_H
