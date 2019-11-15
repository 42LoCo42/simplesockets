#ifndef SINGLECLIENT_H
#define SINGLECLIENT_H

#include "simplesockets_global.h"
#include "address.h"
#include "packet.h"

/**
 * @brief A client that connects to an address.
 */
class SIMPLESOCKETS_EXPORT SingleClient {
	int m_sockfd = -1;

public:
	/**
	 * @brief operator () Tries to connect our socket to that address.
	 * @return If the address is usable.
	 */
	auto operator()(const struct addrinfo*) -> bool;

	/**
	 * @brief start Connects a socket.
	 * @param address The address to connect to.
	 * @return If the connection could be established.
	 */
	auto start(const Address& address) -> bool;
	/**
	 * @brief stop close()s our socket.
	 */
	void disconnect();

	void sendPacket(const Packet&) const;
	void recvPacket(Packet&) const;
};

#endif // SINGLECLIENT_H
