#ifndef MULTISERVER_H
#define MULTISERVER_H

#include "simplesockets_global.h"
#include "address.h"
#include "packet.h"
#include <vector>
#include <sys/select.h>

constexpr uint BAGLOG_SIZE = 5;

enum ConInfo {
	NORMAL,
	CONNECT,
	DISCONNECT
};

/**
 * @brief A MultiServer can listen to many clients. This is what you probably want when setting up a large network.
 */
class SIMPLESOCKETS_EXPORT MultiServer {
	int m_sockfd = -1;
	std::vector<int> m_clientfds {};

	int m_max_fd = -1;
	fd_set m_socket_set {};
	fd_set m_tmp_set {};

public:
	/**
	 * @brief operator () Tries to bind a socket to that address.
	 * @return If the address is usable.
	 */
	auto operator()(const struct addrinfo*) -> bool;

	/**
	 * @brief start Open a listening socket.
	 * @param address The address to listen on. It should have nullptr as address string.
	 * @return If we could set up everything correctly.
	 */
	auto start(const Address& address) -> bool;

	/**
	 * @brief disconnect Stops the connection to a client.
	 * @param client_id The ID of the client.
	 */
	void disconnect(size_t client_id);

	/**
	 * @brief disconnectAll Stops all connections.
	 */
	void disconnectAll();

	/**
	 * @brief sendPacket Send a packet.
	 * @param client_id The ID of the recipient.
	 */
	void sendPacket(const Packet&, size_t client_id) const;

	/**
	 * @brief recvPacket Receive a packet from a client or open a new connection.
	 * @param client_id The client from which the packet (or disconnect) came. -1 on new connection.
	 * @param info Information about the received data.
	 * @param allow_new_clients If new connections should be accepted.
	 */
	void recvPacket(Packet&, size_t& client_id, ConInfo& info, bool accept_new_clients = true);

	auto getServerFD() -> int {return m_sockfd;}
	auto getClientFDS() -> const std::vector<int>& {return m_clientfds;}
};

#endif // MULTISERVER_H
