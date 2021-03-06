/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2014 Calle Laakkonen

   Drawpile is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Drawpile is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Drawpile.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SESSIONDESC_H
#define SESSIONDESC_H

#include "../net/protover.h"

#include <QString>
#include <QMetaType>
#include <QList>
#include <QHostAddress>
#include <QDateTime>

namespace server {

class Session;
class Client;

/**
 * @brief Session identifier
 */
class SessionId {
public:
	SessionId() : _custom(false) { }
	SessionId(const QString &id, bool custom) : _id(id), _custom(custom) { }

	//! Get the ID string
	const QString &id() const { return _id; }

	//! Is this a user specified ID?
	bool isCustom() const { return _custom; }

	//! Generate a random session ID
	static SessionId randomId();

	//! Get a SessionId with a user specified ID
	static SessionId customId(const QString &id);

	operator QString() const { return _id; }
	bool isEmpty() const { return _id.isEmpty(); }

private:
	QString _id;
	bool _custom;
};

/**
 * @brief Information about an available session
 */
struct SessionDescription {
	SessionId id;
	protocol::ProtocolVersion protocolVersion;
	int userCount;
	int maxUsers;
	QString title;
	QByteArray passwordHash;
	QString founder;
	bool closed;
	bool persistent;
	bool nsfm;
	QDateTime startTime;

	SessionDescription();
	SessionDescription(const Session &session);
};

}

Q_DECLARE_METATYPE(server::SessionDescription)

#endif

