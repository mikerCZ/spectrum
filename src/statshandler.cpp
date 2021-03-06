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

#include "statshandler.h"
#include "main.h"
#include <time.h>
#include <gloox/clientbase.h>
#include <gloox/tag.h>
#include <glib.h>
#include "usermanager.h"
#include "log.h"

#include "sql.h"
#include <sstream>
#include <fstream>

#ifndef WIN32
static void process_mem_usage(double& vm_usage, double& resident_set) {
	using std::ios_base;
	using std::ifstream;
	using std::string;

	vm_usage     = 0.0;
	resident_set = 0.0;

	// 'file' stat seems to give the most reliable results
	//
	ifstream stat_stream("/proc/self/stat",ios_base::in);

	// dummy vars for leading entries in stat that we don't care about
	//
	string pid, comm, state, ppid, pgrp, session, tty_nr;
	string tpgid, flags, minflt, cminflt, majflt, cmajflt;
	string utime, stime, cutime, cstime, priority, nice;
	string O, itrealvalue, starttime;

	// the two fields we want
	//
	unsigned long vsize;
	long rss;

	stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
				>> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
				>> utime >> stime >> cutime >> cstime >> priority >> nice
				>> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

	long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
	vm_usage     = vsize / 1024.0;
	resident_set = rss * page_size_kb;
}
#endif

StatsExtension::StatsExtension() : StanzaExtension( ExtStats )
{
	m_tag = NULL;
}

StatsExtension::StatsExtension(const Tag *tag) : StanzaExtension( ExtStats )
{
	m_tag = tag->clone();
}

StatsExtension::~StatsExtension()
{
	Log("StatsExtension", "deleting StatsExtension()");
	delete m_tag;
}

const std::string& StatsExtension::filterString() const
{
	static const std::string filter = "iq/query[@xmlns='http://jabber.org/protocol/stats']";
	return filter;
}

Tag* StatsExtension::tag() const
{
	return m_tag->clone();
}

GlooxStatsHandler::GlooxStatsHandler(GlooxMessageHandler *parent) : IqHandler(){
	p=parent;
	m_messagesIn = m_messagesOut = 0;
	m_startTime = time(NULL);
	p->j->registerStanzaExtension( new StatsExtension() );
}

GlooxStatsHandler::~GlooxStatsHandler(){
}

bool GlooxStatsHandler::handleIq (const IQ &stanza){
// 	      recv:     <iq type='result' from='component'>
//                   <query xmlns='http://jabber.org/protocol/stats'>
// 	            <stat name='time/uptime'/>
// 	            <stat name='users/online'/>
// 	                  .
// 	                  .
// 	                  .
// 	            <stat name='bandwidth/packets-in'/>
// 	            <stat name='bandwidth/packets-out'/>
// 	          </query>
//                 </iq>
//

	Tag *stanzaTag = stanza.tag();
	Tag *query = stanzaTag->findChild("query");
	if (query==NULL) {
		delete stanzaTag;
		return true;
	}
	std::cout << "*** "<< stanza.from().full() << ": received stats request\n";
	std::list<Tag*> stats = query->children();
	if (stats.empty()){
		std::cout << "* sending stats keys\n";
		IQ _s(IQ::Result, stanza.from(), stanza.id());
		std::string from;
		from.append(p->jid());
		_s.setFrom(from);

		Tag *s = _s.tag();
		Tag *query = new Tag("query");
		query->setXmlns("http://jabber.org/protocol/stats");
		Tag *t;

		t = new Tag("stat");
		t->addAttribute("name","uptime");
		query->addChild(t);

		t = new Tag("stat");
		t->addAttribute("name","users/registered");
		query->addChild(t);

		t = new Tag("stat");
		t->addAttribute("name","users/online");
		query->addChild(t);

		t = new Tag("stat");
		t->addAttribute("name","legacy-network-users/online");
		query->addChild(t);

		t = new Tag("stat");
		t->addAttribute("name","legacy-network-users/registered");
		query->addChild(t);

		t = new Tag("stat");
		t->addAttribute("name","messages/in");
		query->addChild(t);

		t = new Tag("stat");
		t->addAttribute("name","messages/out");
		query->addChild(t);
		
#ifndef WIN32
		t = new Tag("stat");
		t->addAttribute("name","memory-usage");
		query->addChild(t);
#endif

		s->addChild(query);

		p->j->send( s );

	}
	else{
		std::cout << "* sending stats values\n";
		IQ _s(IQ::Result, stanza.from(), stanza.id());
		std::string from;
		from.append(p->jid());
		_s.setFrom(from);

		Tag *s = _s.tag();
		Tag *query = new Tag("query");
		query->setXmlns("http://jabber.org/protocol/stats");

		Tag *t;
		long registered = p->sql()->getRegisteredUsersCount();
		long registeredUsers = p->sql()->getRegisteredUsersRosterCount();

		std::stringstream out;
		out << registered;

		time_t seconds = time(NULL);

		long users = p->userManager()->onlineUserCount();

		t = new Tag("stat");
		t->addAttribute("name","uptime");
		t->addAttribute("units","seconds");
		t->addAttribute("value",seconds - m_startTime);
		query->addChild(t);

		t = new Tag("stat");
		t->addAttribute("name","users/registered");
		t->addAttribute("units","users");
		t->addAttribute("value",out.str());
		query->addChild(t);

		t = new Tag("stat");
		t->addAttribute("name","users/online");
		t->addAttribute("units","users");
		t->addAttribute("value",p->userManager()->userCount());
		query->addChild(t);

		t = new Tag("stat");
		t->addAttribute("name","legacy-network-users/registered");
		t->addAttribute("units","users");
		t->addAttribute("value",registeredUsers);
		query->addChild(t);

		t = new Tag("stat");
		t->addAttribute("name","legacy-network-users/online");
		t->addAttribute("units","users");
		t->addAttribute("value",users);
		query->addChild(t);


		t = new Tag("stat");
		t->addAttribute("name","messages/in");
		t->addAttribute("units","messages");
		t->addAttribute("value",m_messagesIn);
		query->addChild(t);

		t = new Tag("stat");
		t->addAttribute("name","messages/out");
		t->addAttribute("units","messages");
		t->addAttribute("value",m_messagesOut);
		query->addChild(t);

#ifndef WIN32
		double vm, rss;
		std::stringstream rss_stream;
		process_mem_usage(vm, rss);
		rss_stream << rss;

		t = new Tag("stat");
		t->addAttribute("name","memory-usage");
		t->addAttribute("units","KB");
		t->addAttribute("value", rss_stream.str());
		query->addChild(t);
#endif
		s->addChild(query);

		p->j->send(s);

	}

	delete stanzaTag;
	return true;
}

void GlooxStatsHandler::handleIqID (const IQ &iq, int context){
}
