/*
   Drawpile - a collaborative drawing program.

   Copyright (C) 2006-2015 Calle Laakkonen

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

#include "core/brush.h"
#include "net/client.h"
#include "net/commands.h"

#include "tools/toolcontroller.h"
#include "tools/brushes.h"

#include "../shared/net/undo.h"
#include "../shared/net/pen.h"

namespace tools {

void BrushBase::begin(const paintcore::Point& point, float zoom)
{
	Q_UNUSED(zoom);

	QList<protocol::MessagePtr> msgs;
	msgs << protocol::MessagePtr(new protocol::UndoPoint(0));

	// TODO remember last sent brush
	msgs << net::command::brushToToolChange(0, owner.activeLayer(), owner.activeBrush());
	protocol::PenPointVector v(1);
	v[0] = net::command::pointToProtocol(point);
	msgs << protocol::MessagePtr(new protocol::PenMove(0, v));
	owner.client()->sendMessages(msgs);
}

void BrushBase::motion(const paintcore::Point& point, bool constrain, bool center)
{
	Q_UNUSED(constrain);
	Q_UNUSED(center);

	protocol::PenPointVector v(1);
	v[0] = net::command::pointToProtocol(point);
	owner.client()->sendMessage(protocol::MessagePtr(new protocol::PenMove(0, v)));
}

void BrushBase::end()
{
	owner.client()->sendMessage(protocol::MessagePtr(new protocol::PenUp(0)));
}

}

