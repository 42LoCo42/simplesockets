#include "multiserver.h"
#include <unistd.h>
#include <stdexcept>

auto MultiServer::start(const Address& address) -> bool {
	return address.foreach(*this);
}

auto MultiServer::operator()(const struct addrinfo* ai) -> bool {
	// open socket
	m_sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if(m_sockfd == -1) {
		return false;
	}

	// enable reusing of addresses
	int yes = 1;
	if(setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) != 0) {
		close(m_sockfd);
		m_sockfd = -1;
		return false;
	}

	// bind to address
	if(bind(m_sockfd, ai->ai_addr, ai->ai_addrlen) != 0) {
		close(m_sockfd);
		m_sockfd = -1;
		return false;
	}

	// set socket to listen mode
	if(listen(m_sockfd, 5) != 0) {
		close(m_sockfd);
		m_sockfd = -1;
		return false;
	}

	// add socket to m_socket_set
	m_max_fd = m_sockfd;
	FD_SET(m_sockfd, &m_socket_set);

	// we have a valid socket!
	return true;
}

void MultiServer::disconnect(size_t client_id) {
	if(client_id < m_clientfds.size()) {
		close(m_clientfds[client_id]);
		m_clientfds.erase(m_clientfds.begin() + static_cast<long>(client_id));
		FD_CLR(m_clientfds[client_id], &m_socket_set);

		// this socket was max fd, find new one
		if(m_clientfds[client_id] >= m_max_fd) {
			m_max_fd = -1;
			for(auto fd : m_clientfds) {
				if(fd > m_max_fd) {
					m_max_fd = fd;
				}
			}
		}
	}
}

void MultiServer::disconnectAll() {
	for(auto fd : m_clientfds) {
		close(fd);
	}
	m_clientfds.clear();
	m_max_fd = -1;
	FD_ZERO(&m_socket_set);
}

void MultiServer::sendPacket(const Packet& packet, size_t client_id) const {
	if(client_id < m_clientfds.size()) {
		packet.send(m_clientfds[client_id]);
	}
}

void MultiServer::recvPacket(Packet& packet, size_t& client_id, ConInfo& info, bool accept_new_clients) {
	m_tmp_set = m_socket_set;
	if(select(m_max_fd+1, &m_tmp_set, nullptr, nullptr, nullptr) == -1) {
		throw std::runtime_error("MultiServer: select");
	}

	// check for new connection
	if(accept_new_clients && FD_ISSET(m_sockfd, &m_tmp_set)) {
		int new_fd = accept(m_sockfd, nullptr, nullptr);
		if(new_fd < 0) {
			throw std::runtime_error("Connection to client failed!");
		}

		// store new socket
		FD_SET(new_fd, &m_socket_set);
		m_clientfds.push_back(new_fd);
		if(new_fd > m_max_fd) {
			m_max_fd = new_fd;
		}

		client_id = m_clientfds.size() - 1;
		info = CONNECT;
		return;
	}

	// find connection with data
	for(size_t i = 0; i < m_clientfds.size(); ++i) {
		if(FD_ISSET(m_clientfds[i], &m_tmp_set)) {
			// assign client ID
			client_id = i;

			if(packet.recv(m_clientfds[i])) { // data
				info = NORMAL;
			} else { // disconnect or invalid packet
				disconnect(i);
				info = DISCONNECT;
			}
			return;
		}
	}
}
