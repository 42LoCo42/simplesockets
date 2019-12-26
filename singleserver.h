#ifndef SINGLESERVER_H
#define SINGLESERVER_H

#include "simplesockets_global.h"
#include "address.h"
#include "packet.h"

constexpr uint BACKLOG_SIZE = 5;

/**
 * @brief A server that can only handle a single client.
 */
class SIMPLESOCKETS_EXPORT SingleServer {
	int m_sockfd = -1;
	int m_clientfd = -1;

public:
	/**
	 * @brief operator () Tries to bind a socket to that address.
	 * @return If the address is usable.
	 */
	auto operator()(const struct addrinfo*) -> bool;

	/**
	 * @brief start Open a listening socket and wait for a client.
	 * @param address The address to listen on. It should have nullptr as address string.
	 * @return If we could set up everything correctly.
	 */
	auto start(const Address& address) -> bool;
	/**
	 * @brief stop close()s the client socket.
	 */
	void disconnect();

	void sendPacket(const Packet&) const;
	void recvPacket(Packet&);

	auto getServerFD() -> int {return m_sockfd;}
	auto getClientFD() -> int {return m_clientfd;}
};

#endif // SINGLESERVER_H
