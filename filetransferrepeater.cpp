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
 
#include "filetransferrepeater.h"
#include "main.h"

FiletransferRepeater::FiletransferRepeater(GlooxMessageHandler *main, const JID& to, const std::string& sid, SIProfileFT::StreamType type, const JID& from) {
	m_main = main;
	m_to = to;
	m_sid = sid;
	m_type = type;
	m_from = from;
}

void FiletransferRepeater::registerXfer(PurpleXfer *xfer) {
	m_xfer = xfer;
	
}

