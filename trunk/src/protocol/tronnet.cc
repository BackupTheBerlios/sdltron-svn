#include "client.h"
#include "SDL_thread.h"
#include "hexstream.h"
#include <sstream>

using namespace std;

namespace tron
{

	NPP::TCPsocket *tcp=0;
	::SDL_Thread *tcp_tid=0;
	NPP::UDPsocket *udp=0;
	::SDL_Thread *udp_tid=0;
	bool done=false;
	bool udp_done=true;
	bool tcp_done=true;
	bool cleanup_installed=false;
	vector<Client*> clients;
	NetParams net_params={10};

	bool net_udp_send(tron::Pack &p, const NPP::IPaddress &ip)
	{
		ostringstream oss;
		p.pack(oss);
		return udp->send(NPP::UDPpacket((Sint16)oss.str().size(),
					(void*)oss.str().data(), ip));
	}

	namespace
	{

		int net_udpthread(void *arg)
		{
			NPP::UDPsocket *s=reinterpret_cast<NPP::UDPsocket*>(arg);
			NPP::SocketSet sockset;
			sockset.add(*s);
			while(!done && sockset.check(100)>=0)
			{
				//clog<<"net_udpthread running..."<<endl;
				if(!s->ready())
				{
					// ping clients
					for(vector<Client*>::iterator it=clients.begin();
							it!=clients.end(); ++it)
						(*it)->send_ping();
					continue;
				}
				NPP::UDPpacket p;
				if(s->recv(p)>0)
				{
					istringstream iss(string((char*)p.get_data(),(int)p.get_len()));
					cerr<<"udp packet received from "<<p.get_address()<<endl;
					hexstream hex(false,cerr);
					hex<<iss.str()<<endl;
					tron::Pack *tp=tron::unpack(iss);
					if(tp)
					{
						cerr<<*tp;
						switch(tp->pack_type())
						{
							case tron::Pack::PING:
								{
									tron::Ping *ping=dynamic_cast<tron::Ping*>(tp);
									tron::Pong pong(ping->time);
									net_udp_send(pong,p.get_address());
									delete tp;
								}
								break;
							default:
								{
									SDL_Event e;
									e.type=SDL_USEREVENT;
									e.user.code=TRON_EVENT_USER_CODE;
									NPP::IPaddress *ip=new NPP::IPaddress(p.get_address());
									e.user.data1=tp;
									e.user.data2=ip;
									while(SDL_PushEvent(&e)==-1)
										SDL_Delay(1);
									ip=0;
								}
								break;
						}
					}
				}
			}
			clog<<"net_udpthread finished"<<endl;
			udp_done=true;
			return 0;
		}

		int net_tcpserverthread(void *arg)
		{
			NPP::TCPsocket *s=reinterpret_cast<NPP::TCPsocket*>(arg);
			NPP::SocketSet sockset;
			sockset.add(*s);
			while(!done && sockset.check(1000)>=0)
			{
				if(!s->ready())
				{
					//do not ping clients via tcp
					continue;
				}
				clog<<"net_tcpserverthread accepting new connection..."<<endl;
				NPP::TCPsocket *client=s->accept();
				if(client)
				{
					clog<<"net_tcpserverthread accept new connection from "<<client->get_peer_address()<<endl;
					SDL_Event e;
					e.type=SDL_USEREVENT;
					e.user.code=TRON_CONNECT_USER_CODE;
					e.user.data1=client;
					e.user.data2=0;
					while(SDL_PushEvent(&e)==-1)
						SDL_Delay(1);
				}
				else
				clog<<"net_tcpserverthread accept failed...oddly enough."<<endl;
			}
			clog<<"net_tcpserverthread finished"<<endl;
			tcp_done=true;
			return 0;
		}

		void cleanup()
		{
			clog<<"cleanup..."<<endl;
			done=true;
			int t=SDL_GetTicks();
			while(udp_tid && !udp_done && SDL_GetTicks()-t<200) SDL_Delay(10);
			SDL_Delay(200);
			if(udp_tid)
			{
				if(udp_done)
					SDL_WaitThread(udp_tid,0);
				else
					SDL_KillThread(udp_tid);
				udp_tid=0;
			}
			delete udp;
			udp=0;
			while(tcp_tid && !tcp_done && SDL_GetTicks()-t<2000) SDL_Delay(10);
			if(tcp_tid)
			{
				if(tcp_done)
					SDL_WaitThread(tcp_tid,0);
				else
					SDL_KillThread(tcp_tid);
				tcp_tid=0;
			}
			delete tcp;
			tcp=0;
			for(vector<Client*>::iterator it=clients.begin();
					it!=clients.end(); ++it)
				delete *it;
			clients.clear();
			clog<<"cleanup finished"<<endl;
		}

	}; // anonymous namespace

	bool net_init_server(Uint16 port)
	{
		if(!NPP::init())
			NPP::error("NPP:init()",1);
		if(!cleanup_installed)
			atexit(cleanup);
		cleanup_installed=true;
		clog<<"Creating UDP server socket at port "<<port<<endl;
		udp=new NPP::UDPsocket(port);
		if(!udp->valid())
		{
			cerr<<"Failed to create UDP socket for port "<<port<<'!'<<endl;
			cerr<<SDL_GetError()<<endl;
			cleanup();
			return false;
		}
		clog<<"Creating TCP server socket at port "<<port<<endl;
		tcp=new NPP::TCPsocket(port);
		if(!tcp->valid())
		{
			cerr<<"Failed to create TCP socket for port "<<port<<'!'<<endl;
			cerr<<SDL_GetError()<<endl;
			cleanup();
			return false;
		}
		udp_done=false;
		clog<<"Creating UDP server thread ... ";
		udp_tid=SDL_CreateThread(net_udpthread,reinterpret_cast<void*>(udp));
		clog<<udp_tid<<endl;
		if(!udp_tid)
		{
			cerr<<"Failed to create UDP server thread!"<<endl;
			cerr<<SDL_GetError()<<endl;
			cleanup();
			return false;
		}
		tcp_done=false;
		clog<<"Creating TCP server thread ... ";
		tcp_tid=SDL_CreateThread(net_tcpserverthread,reinterpret_cast<void*>(tcp));
		clog<<tcp_tid<<endl;
		if(!tcp_tid)
		{
			cerr<<"Failed to create TCP server thread!"<<endl;
			cerr<<SDL_GetError()<<endl;
			cleanup();
			return false;
		}
		return true;
	}

	void *net_server_add_client(NPP::TCPsocket *tcp)
	{
		Client *c=new Client(tcp);
		clients.push_back(c);
		return c;
	}

	void net_server_cleanup_client(void *client)
	{
		vector<Client*>::iterator it=find(clients.begin(),clients.end(),(Client*)client);
		if(it!=clients.end())
		{
			delete *it;
			clients.erase(it);
		}
	}

	bool net_init_client(Uint16 port, const NPP::IPaddress &server)
	{
		if(!NPP::init())
			NPP::error("NPP:init()",1);
		if(!cleanup_installed)
			atexit(cleanup);
		cleanup_installed=true;
		udp=new NPP::UDPsocket(port);
		if(!udp->valid())
		{
			cleanup();
			return false;
		}
		tcp=new NPP::TCPsocket(server);
		if(!tcp->valid())
		{
			cleanup();
			return false;
		}
		return true;
	}

}; // namespace tron
