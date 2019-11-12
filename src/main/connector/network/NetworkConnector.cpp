/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *  Presenter. Server software to remote control a presentation.         *
 *  Copyright (C) 2019 Felix Wohlfrom                               *
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
 * NetworkConnector.cpp
 *
 *  Created on: 06.01.2019
 *      Author: Felix Wohlfrom
 */

#include <QHostInfo>
#include <QNetworkInterface>
#include "NetworkConnector.h"

// Randomly selected port for broadcasting
const int NetworkConnector::broadcastPort = 43154;

NetworkConnector::NetworkConnector() :
    broadcastSocket(NULL), keyCommandServer(NULL), clientSockets()
{
    connect(&broadcastTimer, SIGNAL(timeout()),
                       this, SLOT(broadcastServerAvailablility()));

    // TODO This is the same as the bluetooth service uuid => Unify this
    broadcastMessage = QString("be71c255-8349-4d86-b09e-7983c035a191\n")
            .append(QHostInfo::localHostName()).toUtf8();
}

NetworkConnector::~NetworkConnector()
{
    stopServer();
}

void NetworkConnector::startServer()
{
    keyCommandServer = new QTcpServer(this);
    connect(keyCommandServer, SIGNAL(newConnection()),
                        this, SLOT(clientConnected()));
    if (!keyCommandServer->listen(QHostAddress::Any, broadcastPort + 1))
    {
        emit error(tr("Did not start server. %1.")
                   .arg(keyCommandServer->errorString()));
        return;
    }

    broadcastSocket = new QUdpSocket(this);
    broadcastTimer.start(5000); // Emit the message every 5 seconds

    emit serverReady();
}

void NetworkConnector::stopServer()
{
    broadcastTimer.stop();

    // Close sockets
    qDeleteAll(clientSockets);

    broadcastSocket->close();
    delete broadcastSocket;
    broadcastSocket = NULL;
}

void NetworkConnector::broadcastServerAvailablility()
{
    // We can't emit on internet broadcast address 255.255.255.255 (filtered by most routers),
    // so instead, we need to use the broadcast address(es) of our available interfaces
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for (QNetworkInterface interface: interfaces)
    {
        QList<QNetworkAddressEntry> addressEntries = interface.addressEntries();
        for(QNetworkAddressEntry address: addressEntries)
        {
            if(!address.ip().isLoopback()
                && address.ip().protocol() == QAbstractSocket::IPv4Protocol)
            {
                broadcastSocket->writeDatagram(broadcastMessage, address.broadcast(), broadcastPort);
            }
        }
    }
}

void NetworkConnector::write(const QString& message)
{
    emit info(QString("Write: %1").arg(message));

    for (QTcpSocket* socket: clientSockets)
    {
        QByteArray messageToSend(message.toUtf8());
        int bytesSent = 0;
        while (bytesSent < messageToSend.length())
        {
            int sent = socket->write(messageToSend);
            messageToSend = messageToSend.mid(sent);
            bytesSent += sent;
        }
    }
}

void NetworkConnector::clientConnected()
{
    QTcpSocket *socket = keyCommandServer->nextPendingConnection();
    if (!socket)
    {
        return;
    }

    connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
    clientSockets.append(socket);

    handleClientConnected(socket->peerName());
}

void NetworkConnector::clientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
    {
        return;
    }

    emit RemoteControl::clientDisconnected();

    clientSockets.removeOne(socket);
    socket->deleteLater();
}

void NetworkConnector::readSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
    {
        return;
    }

    while (socket->canReadLine())
    {
        QByteArray line = socket->readLine().trimmed();
        handleLine(socket->peerName(),
                  QString::fromUtf8(line.constData(), line.length()));
    }
}
