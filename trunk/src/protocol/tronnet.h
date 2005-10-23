#ifndef tronnet_h
#define tronnet_h

#include "tron.h"
#include "SDL_net++.h"

namespace tron
{

	const Uint16 SERVER_PORT=3300; //tcp & udp
	const Uint16 TRACKER_PORT=3301; //tcp

	struct NetParams {
		Uint32 tcp_delay;
	};

	extern NetParams net_params;

	enum TronUserCode {
		TRON_EVENT_USER_CODE=1,		//data1=tron::Pack* data2=NPP::IPaddress*
		TRON_CONNECT_USER_CODE,		//data1=NPP::TCPsocket*
		TRON_DISCONNECT_USER_CODE	//data1=Client*
	};

	bool net_init_server(Uint16 port=SERVER_PORT);
	void *net_server_add_client(NPP::TCPsocket *tcp);
	void net_server_cleanup_client(void *client);

	bool net_init_client(Uint16 port, const NPP::IPaddress &server);

	bool net_udp_send(tron::Pack &p, const NPP::IPaddress &ip);

};

#endif
