#include "../protocol/tronnet.h"
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
	putenv("SDL_VIDEODRIVER=dummy");
	if(SDL_Init(SDL_INIT_VIDEO)==-1)
	{
		cerr<<"SDL_Init: "<<SDL_GetError()<<endl;
		return 1;
	}
	if(!tron::net_init_server())
	{
		cerr<<argv[0]<<": failed to initialize server!"<<endl;
		return 2;
	}
	SDL_Event e;
	while(SDL_WaitEvent(&e))
	{
		switch(e.type)
		{
			case SDL_QUIT:
				exit(0);
				break;
			case SDL_USEREVENT:
				switch((tron::TronUserCode)e.user.code)
				{
					case tron::TRON_EVENT_USER_CODE:
						{
							tron::Pack *tp=(tron::Pack*)e.user.data1;
							NPP::IPaddress *ip=(NPP::IPaddress*)e.user.data2;
							//cerr<<"packet from "<<*ip<<": \n"<<*tp;
							// TODO
							delete tp;
							delete ip;
						}
						break;
					case tron::TRON_CONNECT_USER_CODE:
						{
							NPP::TCPsocket *tcp=(NPP::TCPsocket*)e.user.data1;
							tron::net_server_add_client(tcp);
						}
						break;
					case tron::TRON_DISCONNECT_USER_CODE:
						tron::net_server_cleanup_client(e.user.data1);
						break;
				}
				break;
			default:
				break;
		}
	}
	/*
	if(!NPP::init())
		NPP::error("NPP:init()",1);
	NPP::UDPsocket udp(TRON_SERVER_PORT);
	NPP::TCPsocket tcp(TRON_SERVER_PORT);
	NPP::SocketSet set;
	set.add(udp);
	set.add(tcp);
	int nready=0;
	vector<NPP::TCPsocket*> clients;
	while((nready=set.check(-1U))>=0)
	{
		if(!nready)
			continue;
		if(tcp.ready())
		{
			NPP::TCPsocket *client=tcp.accept();
			clients.push_back(client);
			set.add(*client);
			cerr<<"connected: "<<client->get_peer_address()<<endl;
		}
		if(udp.ready())
		{
			NPP::UDPpacket packet;
			udp.recv(packet);
			cout<<packet<<endl;
		}
		for(int i=0; i<clients.size(); ++i)
		{
			if(!clients[i]->ready())
				continue;
			string line;
			cerr<<"data from: "<<clients[i]->get_peer_address()<<endl;
			if(getline(*clients[i],line))
			{
				cout<<line<<endl;
			}
			else
			{
				cerr<<"disconnecting: "<<clients[i]->get_peer_address()<<endl;
				delete clients[i];
				clients.erase(clients.begin()+i);
			}
		}
	}*/
}
