/**
 * XMPP - libpurple transport
 *
 * Copyright (C) 2009, Jan Kaluza <hanzz@soc.pidgin.im>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#ifndef _HI_ADHOC_ADMIN_H
#define _HI_ADHOC_ADMIN_H

#include <string>
#include "purple.h"
#include "account.h"
#include "glib.h"
#include "request.h"
#include "adhoccommandhandler.h"

class GlooxMessageHandler;
class User;

extern LogClass Log_;

typedef enum {	ADHOC_ADMIN_INIT = 0,
				ADHOC_ADMIN_LOGGING
				} AdhocAdminState;

/*
 * AdhocCommandHandler for Administration node
 */
class AdhocAdmin : public AdhocCommandHandler
{
	public:
		AdhocAdmin(GlooxMessageHandler *m, User *user, const std::string &from, const std::string &id);
		~AdhocAdmin();
		bool handleIq(const IQ &iq);
		const std::string & from() { return m_from; }

	private:
		GlooxMessageHandler *main;		// client
		std::string m_from;				// full jid
		User *m_user;					// User class
		int m_state;					// current executing state
};

#endif
