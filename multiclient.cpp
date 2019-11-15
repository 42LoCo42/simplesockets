#include "multiclient.h"
#include <unistd.h>
#include <stdexcept>

auto MultiClient::connect(const Address& address) -> bool {
	return address.foreach(*this);
}

auto MultiClient::operator()(const struct addrinfo* ai) -> bool {
	// open socket
	int sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if(sockfd == -1) {
		return false;
	}

	// connect to address
	if(::connect(sockfd, ai->ai_addr, ai->ai_addrlen) != 0) {
		close(sockfd);
		return false;
	}

	// store new socket
	m_serverfds.push_back(sockfd);
	if(sockfd > m_max_fd) {
		m_max_fd = sockfd;
	}
	FD_SET(sockfd, &m_socket_set);
	return true;
}

void MultiClient::disconnect(size_t server_id) {
	if(server_id < m_serverfds.size()) {
		close(m_serverfds[server_id]);
		m_serverfds.erase(m_serverfds.begin() + static_cast<long>(server_id));
		FD_CLR(m_serverfds[server_id], &m_socket_set);

		// this socket was max fd, find new one
		if(m_serverfds[server_id] >= m_max_fd) {
			m_max_fd = -1;
			for(auto fd : m_serverfds) {
				if(fd > m_max_fd) {
					m_max_fd = fd;
				}
			}
		}
	}
}

void MultiClient::disconnectAll() {
	for(auto fd : m_serverfds) {
		close(fd);
	}
	m_serverfds.clear();
	m_max_fd = -1;
	FD_ZERO(&m_socket_set);
}

void MultiClient::sendPacket(const Packet& packet, size_t server_id) const {
	if(server_id < m_serverfds.size()) {
		packet.send(m_serverfds[server_id]);
	}
}

auto MultiClient::recvPacket(Packet& packet, size_t& server_id) -> bool {
	m_tmp_set = m_socket_set;
	if(select(m_max_fd+1, &m_tmp_set, nullptr, nullptr, nullptr) == -1) {
		throw std::runtime_error("MultiClient: select");
	}

	// find connection with data
	for(size_t i = 0; i < m_serverfds.size(); ++i) {
		if(FD_ISSET(m_serverfds[i], &m_tmp_set)) {
			// assign client ID
			server_id = i;

			if(packet.recv(m_serverfds[i])) { // data
				return true;
			} else { // disconnect or invalid packet
				disconnect(i);
				return false;
			}
		}
	}

	return false;
}
