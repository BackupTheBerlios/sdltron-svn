#ifndef client_h
#define client_h

#include "tronnet.h"
#include <vector>

namespace tron
{

	class Client
	{
		NPP::TCPsocket *s;
		NPP::IPaddress udp_ip;
		::SDL_Thread *tid;
		bool go, done;
		Uint32 ping, last_ping;
		std::vector<tron::Pack*> outgoing;
		SDL_mutex *outgoing_mutex;
		Uint32 delay;

		static int run(void *arg);
		Client(const Client &from) : udp_ip(NPP::IPaddress::None) {}
		Client &operator=(const Client &from) { return *this; }

	public:
		Client(NPP::TCPsocket *s);
		~Client();
		void tcpsend(tron::Pack *p);
		void send_ping();
	};

};

#endif
