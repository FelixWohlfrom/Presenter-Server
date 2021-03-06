/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Presenter. Server software to remote control a presentation.         *
 *  Copyright (C) 2017 Felix Wohlfrom                                    *
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
 * Logger.cpp
 *
 *  Created on: 19.11.2017
 *      Author: Felix Wohlfrom
 */

#include "Logger.h"
#include "ui_Logger.h"

Logger::Logger(QWidget* parent) :
    QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint | Qt::MSWindowsFixedSizeDialogHint),
    ui(new Ui::Logger)
{
    // Initialize window
    ui->setupUi(this);
}

Logger::~Logger()
{
    delete ui;
}

void Logger::append(const QString &message)
{
    ui->logger->append(message);
}

void Logger::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}
