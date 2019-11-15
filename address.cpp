#include "address.h"

Address::~Address() {
	freeaddrinfo(m_addresses);
}

void Address::setaddrinfo(const char *address, const char *port) {
	// prepare hints for getaddrinfo()
	struct addrinfo hints {};
	if(address == nullptr) {
		if(port == nullptr) { // can't resolve passive localhost with no port
			m_addresses = nullptr;
			return;
		}
		hints.ai_flags = AI_PASSIVE;
	}
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if(getaddrinfo(address, port, &hints, &m_addresses) != 0) { // error!
		m_addresses = nullptr; // invalidate
	}
}

auto Address::isValid() const -> bool {
	return m_addresses != nullptr;
}
