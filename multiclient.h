#ifndef MULTICLIENT_H
#define MULTICLIENT_H

#include "simplesockets_global.h"
#include "address.h"
#include "packet.h"
#include <vector>
#include <sys/select.h>

/**
 * @brief A MultiClient can connect to many servers. Highly experimental and mostly untested!
 */
class SIMPLESOCKETS_EXPORT MultiClient {
	std::vector<int> m_serverfds {};

	int m_max_fd = -1;
	fd_set m_socket_set {};
	fd_set m_tmp_set {};

public:
	/**
	 * @brief operator () Tries to open a socket on that address.
	 * @return If the socket creation succeeded.
	 */
	auto operator()(const struct addrinfo*) -> bool;

	/**
	 * @brief connect Connects to that address.
	 * @return If a connection could be established.
	 */
	auto connect(const Address&) -> bool;

	/**
	 * @brief disconnect Closes an open connection.
	 * @param server_id The ID of the connection.
	 */
	void disconnect(size_t server_id);

	/**
	 * @brief stopAll Stops all connections.
	 */
	void disconnectAll();

	/**
	 * @brief sendPacket Send a packet.
	 * @param server_id The ID of the recipient.
	 */
	void sendPacket(const Packet&, size_t server_id) const;

	/**
	 * @brief recvPacket Receive a packet from a server.
	 * @param server_id The ID of the sender.
	 * @return If the sender has closed the connection.
	 */
	auto recvPacket(Packet&, size_t& server_id) -> bool;

	auto getClientFDS() -> const std::vector<int>& {return m_serverfds;}
};

#endif // MULTICLIENT_H
