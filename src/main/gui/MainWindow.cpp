/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Presenter. Server software to remote control a presentation.         *
 *  Copyright (C) 2017-2019 Felix Wohlfrom                               *
 *                                                                       *
 *  This program is free software: you can redistribute it and/or modify *
 *  it under the terms of the GNU General Public License as published by *
 *  the Free Software Foundation, either version 3 of the License, or    *
 *  (at your option) any later version.                                  *
 *                                                                       *
 *  This program is distributed in the hope that it will be useful,      *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *  GNU General Public License for more details.                         *
 *                                                                       *
 *  You should have received a copy of the GNU General Public License    *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * MainWindow.cpp
 *
 *  Created on: 14.07.2017
 *      Author: Felix Wohlfrom
 */

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "AboutWindow.h"

#include <QIcon>
#include <QMenu>
#include <QThread>
#include <QCloseEvent>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    // Initialize window
    ui->setupUi(this);

    // Initialize system tray icon
    icon = new QIcon(":/icon");

    // Initialize the logger window
    logger = new Logger(this);

    // Create tray icon and context menu
    openAction = new QAction(tr("&Open"), this);
    connect(openAction, SIGNAL(triggered()), this, SLOT(restore()));
    aboutAction = new QAction(tr("&About"), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAboutScreen()));
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(openAction);
    trayIconMenu->addAction(aboutAction);
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    trayIcon->setIcon(*icon);
    setWindowIcon(*icon);
    trayIcon->show();

    // Set minimum width for info labels
    ui->bluetoothServerStatus->setMinimumWidth(
            ui->bluetoothServerStatus->fontMetrics()
                .boundingRect(ui->bluetoothServerStatus->text())
                .width());
    ui->bluetoothServerStatus->setMinimumWidth(
            ui->bluetoothServerStatus->fontMetrics()
                .boundingRect(tr("Error, see log for Details"))
                .width());
    ui->networkServerStatus->setMinimumWidth(
            ui->networkServerStatus->fontMetrics()
                .boundingRect(ui->networkServerStatus->text())
                .width());
    ui->networkServerStatus->setMinimumWidth(
            ui->networkServerStatus->fontMetrics()
                .boundingRect(tr("Error, see log for Details"))
                .width());

    // Start the server in a background thread to keep the UI responsible
    QTimer *serverStartTimer = new QTimer();
    serverStartTimer->setSingleShot(true);
    connect(serverStartTimer, SIGNAL(timeout()), this, SLOT(startServer()));
    serverStartTimer->start(100);
}

MainWindow::~MainWindow()
{
    delete btConnector;
    delete networkConnector;

    delete ui;
    delete icon;
    delete logger;
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange && isMinimized()
        && QSystemTrayIcon::isSystemTrayAvailable() && trayIcon->isVisible())
    {
        if (trayIcon->supportsMessages())
        {
            trayIcon->showMessage(tr("Presenter"),
                                 tr("The program will keep running in the "
                                    "system tray. To terminate the program, "
                                    "choose \"Quit\" in the context menu "
                                    "of the system tray icon."));
        }
        else
        {
            QMessageBox::information(this, tr("Presenter"),
                                 tr("The program will keep running in the "
                                    "system tray. To terminate the program, "
                                    "choose <b>Quit</b> in the context menu "
                                    "of the system tray icon."));
        }

        hide();
        event->ignore();
    }
}

void MainWindow::startServer()
{
    btConnector = new BluetoothConnector();
    networkConnector = new NetworkConnector();

    // The signals of our bt connector
    connect(btConnector, SIGNAL(info(QString)),
                    this, SLOT(info(QString)));
    connect(btConnector, SIGNAL(error(QString)),
                    this, SLOT(bluetoothError(QString)));
    connect(btConnector, SIGNAL(clientConnected(QString)),
                    this, SLOT(bluetoothClientConnected(QString)));
    connect(btConnector, SIGNAL(clientDisconnected()),
                    this, SLOT(bluetoothClientDisconnected()));
    connect(btConnector, SIGNAL(keySent(QString, QString)),
                    this, SLOT(keySent(QString, QString)));
    connect(btConnector, SIGNAL(serverReady()),
                    this, SLOT(bluetoothServerReady()));

    // The signals of our network connector
    connect(networkConnector, SIGNAL(info(QString)),
                    this, SLOT(info(QString)));
    connect(networkConnector, SIGNAL(error(QString)),
                    this, SLOT(networkError(QString)));
    connect(networkConnector, SIGNAL(clientConnected(QString)),
                    this, SLOT(networkClientConnected(QString)));
    connect(networkConnector, SIGNAL(clientDisconnected()),
                    this, SLOT(networkClientDisconnected()));
    connect(networkConnector, SIGNAL(keySent(QString, QString)),
                    this, SLOT(keySent(QString, QString)));
    connect(networkConnector, SIGNAL(serverReady()),
                    this, SLOT(networkServerReady()));

    btConnector->startServer();
    networkConnector->startServer();
}

void MainWindow::info(const QString &message)
{
    logger->append(message);
}

void MainWindow::bluetoothServerReady()
{
    ui->bluetoothServerStatus->setText(
                QString("<font color=\"#0b0\">%1</font>").arg(tr("Ready")));
}

void MainWindow::bluetoothError(const QString &message)
{
    logger->append(QString("<font color=\"#a33\">%1</font>").arg(message));
    ui->bluetoothServerStatus->setText(
            QString("<font color=\"#a33\">%1</font>")
                .arg(tr("Error, see log for Details")));
}

void MainWindow::bluetoothClientConnected(const QString &name)
{
    logger->append(tr("Connected: %1").arg(name));
    ui->bluetoothServerStatus->setText(
                QString("<font color=\"#0b0\">%1</font>").arg(tr("Connected")));
}

void MainWindow::bluetoothClientDisconnected()
{
    logger->append(tr("Disconnected."));
    ui->bluetoothServerStatus->setText(
                QString("<font color=\"#0b0\">%1</font>").arg(tr("Ready")));
}

void MainWindow::networkServerReady()
{
    ui->networkServerStatus->setText(
                QString("<font color=\"#0b0\">%1</font>").arg(tr("Ready")));
}

void MainWindow::networkError(const QString &message)
{
    logger->append(QString("<font color=\"#a33\">%1</font>").arg(message));
    ui->networkServerStatus->setText(
            QString("<font color=\"#a33\">%1</font>")
                .arg(tr("Error, see log for Details")));
}

void MainWindow::networkClientConnected(const QString &name)
{
    logger->append(tr("Connected: %1").arg(name));
    ui->networkServerStatus->setText(
                QString("<font color=\"#0b0\">%1</font>").arg(tr("Connected")));
}

void MainWindow::networkClientDisconnected()
{
    logger->append(tr("Disconnected."));
    ui->networkServerStatus->setText(
                QString("<font color=\"#0b0\">%1</font>").arg(tr("Ready")));
}

void MainWindow::keySent(const QString &sender, const QString &key)
{
    logger->append(tr("Key press, sender %1: %2").arg(sender, key));
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        if (this->isVisible())
        {
            this->hide();
        }
        else
        {
            this->show();
        }
    }
}

void MainWindow::showLog()
{
    if (!logger->isVisible())
    {
        logger->setVisible(true);
    }
}

void MainWindow::showAboutScreen()
{
    // Don't close the main application if just the about window is shown from
    // tray icon.
    if (!this->isVisible())
    {
        QApplication::setQuitOnLastWindowClosed(false);
    }
    AboutWindow(this).exec();
    QApplication::setQuitOnLastWindowClosed(true);
}

void MainWindow::restore()
{
    this->show();
}
