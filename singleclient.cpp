#include "singleclient.h"
#include <unistd.h>

auto SingleClient::start(const Address& address) -> bool {
	return address.foreach(*this);
}

auto SingleClient::operator()(const struct addrinfo* ai) -> bool {
	// open socket
	m_sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
	if(m_sockfd == -1) {
		return false;
	}

	// connect to address
	if(connect(m_sockfd, ai->ai_addr, ai->ai_addrlen) != 0) {
		close(m_sockfd);
		m_sockfd = -1;
		return false;
	}

	// everything is good!
	return true;
}

void SingleClient::disconnect() {
	if(m_sockfd == -1) return;

	close(m_sockfd);
	m_sockfd = -1;
}

void SingleClient::sendPacket(const Packet& p) const {
	p.send(m_sockfd);
}

void SingleClient::recvPacket(Packet& p) const {
	p.recv(m_sockfd);
}
