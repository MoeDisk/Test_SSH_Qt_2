#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <windows.h>
#include <QtConcurrent/QtConcurrentRun>
#include <QMessageBox>
#include <QLabel>
#include <QFileDialog>
#include <QStringList>
#include <QVector>
#include <QRegularExpression>
#include <QDebug>
#include <QProgressDialog>
#include <QStatusBar>
#include <QTableWidgetItem>
#include <QTimer>
#include <QDateTime>
#include <QFile>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->username_ssh_1->setText(username);
    ui->password_ssh_1->setText(password);
    ui->filepath_ssh_1->setText(remoteDirectory);
    ui->excutepath_ssh_1->setText(remoteExcutePath);
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->showMessage("Ready");
    //ui->ipadd1->setInputMask("000.000.000.000");

    QIcon DriveNetIcon = QApplication::style()->standardIcon(QStyle::SP_DriveNetIcon);
    QIcon DriveHDIcon = QApplication::style()->standardIcon(QStyle::SP_DriveHDIcon);
    ui->tabWidget->setTabIcon(0, DriveNetIcon);
    ui->tabWidget->setTabIcon(1, DriveNetIcon);
    ui->tabWidget->setTabIcon(2, DriveHDIcon);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_selectFileButton_clicked()
{
    QString filePath = QFileDialog::getExistingDirectory(this, tr("Choose"), QDir::homePath());
        if (!filePath.isEmpty()) {
            localFilePath = filePath;
            qDebug() << "Selected file:" << localFilePath;
        }
    ui->selectFileLabel->setText(localFilePath);
}

void MainWindow::on_transferButton_clicked(){
    if (localFilePath.isEmpty()) {
            qDebug() << "Please select a file first.";
            QMessageBox::warning(nullptr,"","Please select a file first.");
            return;
        }

    QTableWidget *tableWidget = ui->tableWidget;
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        statusBar->showMessage("Busy");
        QCoreApplication::processEvents();

    if (tableWidget->columnCount() < 3) {
        tableWidget->setColumnCount(3);
    }

    for (const QString &ip : ipVector) {
        bool alreadyExists = false;
        int rowCount = tableWidget->rowCount();
        int newRow = -1;

        for (int row = 0; row < rowCount; ++row) {
            QTableWidgetItem *item = tableWidget->item(row, 1);
            if (item && item->text() == ip) {
                alreadyExists = true;
                newRow = row;
                break;
            }
        }
        if (!alreadyExists) {
            newRow = tableWidget->rowCount();
            tableWidget->insertRow(newRow);

            QTableWidgetItem *checkBoxItem = new QTableWidgetItem();
            checkBoxItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
            checkBoxItem->setCheckState(Qt::Unchecked);
            tableWidget->setItem(newRow, 0, checkBoxItem);

            QTableWidgetItem *ipItem = new QTableWidgetItem(ip);
            tableWidget->setItem(newRow, 1, ipItem);

            QCoreApplication::processEvents();

            QTableWidgetItem *statusItem = new QTableWidgetItem("Ready");
            tableWidget->setItem(newRow, 2, statusItem);

            QCoreApplication::processEvents();
        }

        qDebug() << ip;
        QStringList arguments;
        arguments << "-batch" << "-r" << "-pw" << password << localFilePath << QString("%1@%2:%3").arg(username).arg(ip).arg(remoteDirectory);
        pscpProcess.start("pscp.exe", arguments);

        if (!pscpProcess.waitForStarted()) {
            qDebug() << "Error.";
            if (newRow != -1) {
                QTableWidgetItem *statusItem = tableWidget->item(newRow, 2);
                statusItem->setText("NO");

                QTableWidgetItem *checkBoxItem = tableWidget->item(newRow, 0);
                if (checkBoxItem)
                    checkBoxItem->setCheckState(Qt::Unchecked);
            }
            continue;
        }

        if (!pscpProcess.waitForFinished()) {
            qDebug() << "Time out.";
            if (newRow != -1) {
                QTableWidgetItem *statusItem = tableWidget->item(newRow, 2);
                statusItem->setText("NO");

                QTableWidgetItem *checkBoxItem = tableWidget->item(newRow, 0);
                if (checkBoxItem)
                    checkBoxItem->setCheckState(Qt::Unchecked);
            }
            continue;
        }
        QString errorOutput = QString(pscpProcess.readAllStandardError());
        if (!errorOutput.isEmpty()) {
            qDebug() << "pscp Error:" << errorOutput;
            if (newRow != -1) {
                QTableWidgetItem *statusItem = tableWidget->item(newRow, 2);
                statusItem->setText("NO");
                QTableWidgetItem *checkBoxItem = tableWidget->item(newRow, 0);
                if (checkBoxItem)
                    checkBoxItem->setCheckState(Qt::Unchecked);
            }
        } else {
            if (pscpProcess.exitStatus() == QProcess::NormalExit && pscpProcess.exitCode() == 0) {
                qDebug() <<  "pscp Finish.";
                if (newRow != -1) {
                    QTableWidgetItem *statusItem = tableWidget->item(newRow, 2);
                    statusItem->setText("OK");

                    QTableWidgetItem *checkBoxItem = tableWidget->item(newRow, 0);
                    if (checkBoxItem)
                        checkBoxItem->setCheckState(Qt::Checked);
                }
            } else {
                qDebug() << "pscp Error: " << pscpProcess.errorString();
                if (newRow != -1) {
                    QTableWidgetItem *statusItem = tableWidget->item(newRow, 2);
                    statusItem->setText("NO");
                    QTableWidgetItem *checkBoxItem = tableWidget->item(newRow, 0);
                    if (checkBoxItem)
                        checkBoxItem->setCheckState(Qt::Unchecked);
                }
            }
        }

        qDebug() << remoteHost;
        qDebug() << username;
        qDebug() << password;
        qDebug() << localFilePath;
        qDebug() << remoteDirectory;
        qDebug() << "File transferred done.";

    }statusBar->showMessage("Ready");
    QCoreApplication::processEvents();
    QMessageBox::information(nullptr,"","File transferred done.<br>");
}

void MainWindow::on_username_ssh_1_textChanged(const QString &text)
{
    username = text;
    qDebug() << "New username:" << username;
}

void MainWindow::on_password_ssh_1_textChanged(const QString &text)
{
    password = text;
    qDebug() << "New password:" << password;
}

void MainWindow::on_filepath_ssh_1_textChanged(const QString &text)
{
    remoteDirectory = text;
    qDebug() << "New localFilePath:" << remoteDirectory;
}

void MainWindow::on_ipadd1_textChanged(const QString &text)
{
    remoteHost = text;
    QStringList ipList;

    QRegularExpression ipRangeRegex(R"((\d+\.\d+\.\d+\.\d+)(-(\d+))?)");
    QRegularExpression splitRegex(R"([/\s]+)");

    QStringList ranges = remoteHost.split(splitRegex, Qt::SkipEmptyParts);

    for (const QString &range : ranges) {
        QRegularExpressionMatch match = ipRangeRegex.match(range);

        if (match.hasMatch()) {
            QString baseIp = match.captured(1);
            QStringList baseIpParts = baseIp.split('.');
            int lastPart = baseIpParts.takeLast().toInt();
            int endIp = match.captured(3).isEmpty() ? lastPart : match.captured(3).toInt();
            for (int i = lastPart; i <= endIp; ++i) {
                QString ip = baseIpParts.join('.') + '.' + QString::number(i);
                ipList.append(ip);
            }
        }
}
    ipVector = ipList.toVector();
    qDebug() << "New remoteHost:" << ipVector;
}

void MainWindow::on_excutepath_ssh_1_textChanged(const QString &text)
{
    remoteExcutePath = text;
    qDebug() << "New remoteExcutePath:" << remoteExcutePath;
}

void MainWindow::on_excuteButton_clicked()
{
    std::vector<QString> IPSelect;

    QTableWidget *tableWidget = ui->tableWidget;
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statusBar->showMessage("Busy");
    QCoreApplication::processEvents();
            for (int i = 0; i < tableWidget->rowCount(); ++i) {
                QTableWidgetItem *checkBoxItem = tableWidget->item(i, 0);
                bool isChecked = (checkBoxItem->checkState() == Qt::Checked);
                QTableWidgetItem *ipItem = tableWidget->item(i, 1);
                QString ipAddress = ipItem->text();
                QTableWidgetItem *statusItem = tableWidget->item(i, 2);
                QString status = statusItem->text();
                if (isChecked && status == "OK") {
                    IPSelect.push_back(ipAddress);
                }
            }
            for (const QString &ip : IPSelect) {
                        qDebug() << "Selected IP: " << ip;
            }
            if (remoteExcutePath.isEmpty()) {
                    qDebug() << "Please select a file first.";
                    QMessageBox::warning(nullptr,"","Please select a file first.");
                    return;
                }
            if (IPSelect.empty()) {
                    qDebug() << "Please select an IP Address first.";
                    QMessageBox::warning(nullptr,"","Please select an IP Address first.");
                    return;
                }

            for (const QString &ipselect : IPSelect) {
                QString remoteCommand = QString("sudo chmod 777 %1 ; nohup %1 > /dev/null 2>&1 &")
                                            .arg(remoteExcutePath);
                QStringList arguments;
                arguments << "-pw" << password << QString("%1@%2").arg(username).arg(ipselect) << remoteCommand;
                int exitCode = QProcess::execute("plink.exe", arguments);
                qDebug() << exitCode;
                qDebug() << ipselect;
                qDebug() << username;
                qDebug() << password;
                qDebug() << remoteExcutePath;
                statusBar->showMessage("Ready");
                QCoreApplication::processEvents();
                qDebug() << "File excuted done.";

                for(const QString &ipselect : IPSelect){
                    QDateTime currentTime = QDateTime::currentDateTime();
                    QString timeString = currentTime.toString("yyyy-MM-dd");
                    QString fileName = QString("log_%1.log").arg(timeString);
                    QFile file(fileName);
                    if (file.open(QIODevice::Append | QIODevice::Text)) {
                        QTextStream out(&file);
                        QString timeLog = currentTime.toString("yyyy-MM-dd-hh-mm-ss-zzz");
                                out << timeLog << "\n" << ipselect << "\t" << "Executed Done.\n\n";
                        file.close();
                    }
            }
                QMessageBox::information(nullptr,"","File executed done.");
        }
}

void MainWindow::on_testButton_clicked(){
    QTableWidget *tableWidget = ui->tableWidget;
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QList<QTableWidgetItem *> selectedItems = tableWidget->selectedItems();
    statusBar->showMessage("Busy");
    QCoreApplication::processEvents();
        if (selectedItems.isEmpty()) {
            QMessageBox::warning(this, "", "Please select an IP Address!");
                return;
            }
        bool ipSelected = false;
        QString selectedText;
        for (QTableWidgetItem *item : selectedItems) {
            if (item->column() == 1) {
                ipSelected = true;
                selectedText = item->text();
                break;
            }
        }
            if (!ipSelected) {
                QMessageBox::warning(this, "", "Please select an IP Address in the first column!");
            } else {
                    qDebug() << selectedItems;
                    QString command = "ping";
                    QStringList arguments;
                    arguments << "-n" << "1" << selectedText;
                    QProcess testProcess;
                    testProcess.start("cmd.exe", QStringList() << "/c" << command << arguments);
                    if (!testProcess.waitForFinished()) {
                        QMessageBox::warning(this, "", "Ping command failed to execute.");
                        return;
                    }
                    QString output = QString::fromLocal8Bit(testProcess.readAllStandardOutput());
                    QMessageBox::information(nullptr, "", output);
                    statusBar->showMessage("Ready");
                    QCoreApplication::processEvents();
            }
}

void MainWindow::on_adbconnect_Button_clicked()
{
    QString Command = QString("adb.exe connect %1")
                    .arg(remoteHost2);
    adbProcess.setProgram("cmd.exe");
                    QStringList arguments;
                    arguments << "/c" << Command;
                    adbProcess.setArguments(arguments);
                    adbProcess.start();
    QString output = QString::fromLocal8Bit(adbProcess.readAllStandardOutput());
    QMessageBox::information(nullptr, "Command Output", output);
}

void MainWindow::on_ipadd2_textChanged(const QString &text)
{
    remoteHost2 = text;
}

void MainWindow::on_adbkillButton_clicked()
{
    QString Command = QString("adb.exe kill-server");
    adbProcess.setProgram("cmd.exe");
                    QStringList arguments;
                    arguments << "/c" << Command;
                    adbProcess.setArguments(arguments);
                    adbProcess.start();
    QString output = QString::fromLocal8Bit(adbProcess.readAllStandardOutput());
    QMessageBox::information(nullptr, "Command Output", output);
}

void MainWindow::on_adbpushButton_clicked()
{
    if (localFilePath.isEmpty()) {
            qDebug() << "Please select a file first.";
            QMessageBox::warning(nullptr,"","Please select a file first.");
            return;
        }

    QProgressDialog progressDialog("Wait", "Cancel", 0, 0, this);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.show();

                QString transferCommand = QString("adb.exe push %1 %2")
                .arg(localFilePath)
                .arg(remoteDirectory);
                qDebug() << "Executing command: " << transferCommand;
                adbProcess.setProgram("cmd.exe");
                QStringList arguments;
                arguments << "/c" << transferCommand;
                adbProcess.setArguments(arguments);
                adbProcess.start();
        if (!adbProcess.waitForStarted()) {
            qDebug() << "Failed to start adb process:" << adbProcess.errorString();
            QMessageBox::warning(nullptr,"","Failed to start adb process.");
            return;
        }
        if (!adbProcess.waitForFinished(-1)) {
            qDebug() << "Failed to finish adb process:" << adbProcess.errorString();
            QMessageBox::warning(nullptr,"","Failed to finish adb process.");
            return;
        }

        progressDialog.hide();
        QString output = QString::fromLocal8Bit(adbProcess.readAllStandardOutput());
        QMessageBox::information(nullptr, "Command Output", output);
        qDebug() << localFilePath;
        qDebug() << remoteDirectory;
        qDebug() << "File transferred successfully.";
}

void MainWindow::on_CheckAll_stateChanged(int state)
{
    QTableWidget *tableWidget = ui->tableWidget;
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    if (state == Qt::Checked) {
        for (int row = 0; row < tableWidget->rowCount(); ++row) {
                QTableWidgetItem *item = tableWidget->item(row, 0);
                if (item) {
                    item->setCheckState(Qt::Checked);
                }
            }
        } else {
        for (int row = 0; row < tableWidget->rowCount(); ++row) {
                QTableWidgetItem *item = tableWidget->item(row, 0);
                if (item) {
                    item->setCheckState(Qt::Unchecked);
                }
            }
        }
}

void MainWindow::on_test2button_clicked()
{
    QTableWidget *tableWidget = ui->tableWidget;
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        QList<QTableWidgetItem *> selectedItems = tableWidget->selectedItems();
            statusBar->showMessage("Busy");
            QCoreApplication::processEvents();
                if (selectedItems.isEmpty()) {
                    QMessageBox::warning(this, "", "Please select an IP Address!");
                    return;
                }
                bool ipSelected = false;
                QString selectedText;
                for (QTableWidgetItem *item : selectedItems) {
                    if (item->column() == 1) {
                        ipSelected = true;
                        selectedText = item->text();
                        break;
                    }
                }
                if (!ipSelected) {
                    QMessageBox::warning(this, "", "Please select an IP Address in the first column!");
                } else {
                        qDebug() << selectedItems;
                        QString puttyCommand = QString("start putty.exe -pw %1 %2@%3 ")
                                                    .arg(password)
                                                    .arg(username)
                                                    .arg(selectedText);

                        QProcess *testSSHProcess = new QProcess(this);
                                testSSHProcess->setProgram("cmd.exe");
                                testSSHProcess->setArguments({"/C", puttyCommand});
                                qDebug() << puttyCommand;
                                testSSHProcess->start();
                }
}

void MainWindow::on_adbshellButton_clicked()
{
    QString Command = QString("start adb.exe shell");
    QProcess *testSSHProcess = new QProcess(this);
            testSSHProcess->setProgram("cmd.exe");
            testSSHProcess->setArguments({"/C", Command});
            qDebug() << Command;
            testSSHProcess->start();
}

void MainWindow::on_logsaveButton_clicked()
{
            QDateTime currentTime = QDateTime::currentDateTime();
            QString timeString = currentTime.toString("yyyy-MM-dd-hh-mm-ss-zzz");

            QString fileName = QString("tlog_%1.log").arg(timeString);

            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);

                int columnCount = ui->tableWidget->columnCount();

                for (int column = 0; column < columnCount; ++column) {
                    QString headerText = ui->tableWidget->horizontalHeaderItem(column)->text();
                    out << headerText;
                    if (column < columnCount - 1) {
                        out << ", ";
                    }
                }
                out << "\n";

                int rowCount = ui->tableWidget->rowCount();
                for (int row = 0; row < rowCount; ++row) {
                    for (int column = 0; column < columnCount; ++column) {
                        if (column == 0) {
                            QTableWidgetItem *checkBoxItem = ui->tableWidget->item(row, column);
                            if (checkBoxItem) {
                                out << (checkBoxItem->checkState() == Qt::Checked ? "T" : "F");
                            }
                        } else {
                            QTableWidgetItem *item = ui->tableWidget->item(row, column);
                            if (item) {
                                out << item->text();
                            }
                        }
                        if (column < columnCount - 1) {
                            out << ", ";
                        }
                    }
                    out << "\n";
                }
                file.close();
                qDebug() << "Table widget content saved to" << fileName;
            }
}
