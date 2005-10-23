#include "client.h"
#include "hexstream.h"

using namespace std;

namespace tron
{

int Client::run(void *arg)
{
	//Uint32 last_ping=0;
	Client *This=reinterpret_cast<Client*>(arg);
	NPP::TCPsocket *s=This->s;
	NPP::SocketSet sockset;
	sockset.add(*s);
	while(This->go && sockset.check(net_params.tcp_delay)>=0)
	{
		if(s->ready())
		{
			tron::Pack *tp=tron::unpack(*s);
			if(tp)
			{
				cerr<<"tcp packet from "<<s->get_peer_address()<<": \n"<<*tp;
				hexstream hex;
				tp->pack(hex);
				switch(tp->pack_type())
				{
					case tron::Pack::PING:
						{
							tron::Ping *ping=dynamic_cast<tron::Ping*>(tp);
							tron::Pong pong(ping->time);
							pong.pack(*s);
							delete tp;
						}
						break;
					default:
						{
							NPP::IPaddress *ip=new NPP::IPaddress(s->get_peer_address());
							SDL_Event e;
							e.type=SDL_USEREVENT;
							e.user.code=TRON_EVENT_USER_CODE;
							e.user.data1=tp;
							e.user.data2=ip;
							while(SDL_PushEvent(&e)==-1)
								SDL_Delay(1);
						}
				}
			}
			else
			{
				cerr<<"Client::run: "<<This->s->get_peer_address()<<": disconnected!"<<endl;
				goto end_run;
			}
		}
		/*if(SDL_GetTicks()-last_ping>1000)
		{
			tron::Ping ping(SDL_GetTicks());
			ping.pack(*s);
			last_ping=ping.time;
		}*/
		while(SDL_mutexP(This->outgoing_mutex)==0)
		{
			tron::Pack *p=0;
			if(This->outgoing.size())
			{
				p=This->outgoing[0];
				This->outgoing.erase(This->outgoing.begin());
				SDL_mutexV(This->outgoing_mutex);
			}
			SDL_mutexV(This->outgoing_mutex);
			if(!p)
				break;
			if(!(p->pack(*s)))
			{
				delete p;
				cerr<<"Client::run: "<<This->s->get_peer_address()<<": failed to send!"<<endl;
				goto end_run;
			}
			delete p;
		}
	}
end_run:
	This->done=true;
	SDL_Event e;
	e.type=SDL_USEREVENT;
	e.user.code=TRON_DISCONNECT_USER_CODE;
	e.user.data1=This;
	while(SDL_PushEvent(&e)==-1)
		SDL_Delay(1);
	clog<<"Client("<<(void*)This<<") thread finished"<<endl;
	return 0;
}

Client::Client(NPP::TCPsocket *s)
	: s(s)
	, udp_ip(s->get_peer_address().get_ip())
	, tid(0)
	, go(true)
	, done(false)
	, ping(0), last_ping(0)
	, outgoing_mutex(SDL_CreateMutex())
{
	clog<<"Client("<<(void*)this<<") creating client thread ... ";
	if(!(tid=SDL_CreateThread(run,this)))
	{
		clog<<SDL_GetError()<<endl;
		done=true;
	}
	clog<<tid<<endl;
}
	
Client::~Client()
{
	go=false;
	clog<<"Client("<<(void*)this<<") destroying client thread "<<tid<<endl;
	if(tid)
	{
		if(!done)
			SDL_Delay(net_params.tcp_delay+1);
		if(done)
			SDL_WaitThread(tid,0);
		else
			SDL_KillThread(tid);
		tid=0;
	}
	delete s;
	s=0;
	SDL_mutexP(outgoing_mutex);
	while(outgoing.size())
	{
		delete outgoing[0];
		outgoing.erase(outgoing.begin());
	}
	SDL_mutexV(outgoing_mutex);
	SDL_DestroyMutex(outgoing_mutex);
}

void Client::tcpsend(tron::Pack *p)
{
	if(!go || done || !tid) return;
	SDL_mutexP(outgoing_mutex);
	outgoing.push_back(p);
	SDL_mutexV(outgoing_mutex);
}

void Client::send_ping()
{
	if(!udp_ip.get_port())
		return;
	Uint32 now=SDL_GetTicks();
	if(now-last_ping<1000)
		return;
	tron::Ping ping(now);
	net_udp_send(ping,udp_ip);
	last_ping=now;
	clog<<"Client("<<(void*)this<<") ping sent"<<endl;
}

}; // namespace tron
