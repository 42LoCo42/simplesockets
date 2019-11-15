#ifndef ADDRESSLIST_H
#define ADDRESSLIST_H

#include "simplesockets_global.h"
#include <netdb.h>

/**
 * @brief A wrapper around getaddrinfo() with support for functors.
 */
class SIMPLESOCKETS_EXPORT Address {
	struct addrinfo* m_addresses = {};

public:
	Address() = default;
	explicit Address(const char* address, const char* port = nullptr) : Address() {setaddrinfo(address, port);}
	~Address();

	/**
	 * @brief setaddrinfo Resolves the given address and stores the result in this object.
	 * @param address The address to resolve. If it's nullptr, localhost is used with the AI_PASSIVE flag set, therefore it is suitable for binding.
	 * @param port Optional. Use if you need the address in the context of that port (e.g. for binding); therefore required if address is nullptr.
	 */
	void setaddrinfo(const char* address, const char* port = nullptr);

	/**
	 * @brief isValid Check if the provided address could be resolved.
	 * @return Return validity.
	 */
	auto isValid() const -> bool;

	template<typename Functor>
	/**
	 * @brief foreach Calls the functor for all addresses.
	 * @param af The functor to call. If it returns true, terminate.
	 * @return If the address list is valid.
	 */
	auto foreach(Functor& f) const -> bool {
		if(!isValid()) return false;

		for(auto p = m_addresses; p != nullptr; p = p->ai_next) {
			if(f(p)) return true; // the functor has signaled us to halt
		}
		return false; // the functor never succeeded
	}
};

#endif // ADDRESSLIST_H
