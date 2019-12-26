#include "address.h"
#include <string>
#include <stdexcept>
using namespace std;

Address::Address() : m_addresses(nullptr, nullptr) {}

Address::Address(const char* address, const char* port) : m_addresses(nullptr, ADDRESS_DELETER) {
	setaddrinfo(address, port);
}

auto Address::operator=(Address&& other) noexcept -> Address& {
	m_addresses.reset(other.m_addresses.release());
	return *this;
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

	struct addrinfo* tmp {};
	int retval;
	if((retval = getaddrinfo(address, port, &hints, &tmp)) == 0) { // success, swap tmp into storage
		m_addresses.reset(tmp);
	} else {
		throw runtime_error(string("getaddrinfo: ") + gai_strerror(retval));
	}
}

auto Address::isValid() const -> bool {
	return m_addresses != nullptr;
}
