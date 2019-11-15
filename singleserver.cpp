#include "singleserver.h"
#include <unistd.h>

auto SingleServer::operator()(const struct addrinfo* ai) -> bool {
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

	// we have a valid socket!
	return true;
}

auto SingleServer::start(const Address& address) -> bool {
	address.foreach(*this);

	// no address could create a valid socket
	if(m_sockfd == -1) return false;

	m_clientfd = accept(m_sockfd, nullptr, nullptr);
	if(m_clientfd > -1) {
		// we don't listen for more than one connection, close our socket
		close(m_sockfd);
		m_sockfd = -1;
		return true;
	}
	// the client connection failed
	return false;
}

void SingleServer::disconnect() {
	if(m_clientfd == -1) return;

	close(m_clientfd);
	m_clientfd = -1;
}

void SingleServer::sendPacket(const Packet& p) const {
	p.send(m_clientfd);
}

void SingleServer::recvPacket(Packet& p) {
	if(!p.recv(m_clientfd)) { // disconnect
		disconnect();
	}
}
