#ifndef PACKET_H
#define PACKET_H

#include "simplesockets_global.h"
#include <string>

/**
 * @brief Packs and safely sends data over sockets.
 */
class SIMPLESOCKETS_EXPORT Packet {
	unsigned char m_length_header {};
	std::string m_length_data {};
	std::string m_data;

public:
	/**
	 * @brief Packet Default constructor. Leaves members empty.
	 */
	explicit Packet() = default;

	/**
	 * @brief Packet Constructor. Loads the string into the content and calculates length data.
	 * @param s The content.
	 */
	explicit Packet(const std::string& s);

	/**
	 * @brief itostr256 Converts the value to a string of bytes (thus base 256, the size of a byte).
	 * @param value The value to convert.
	 * @param data The string to create.
	 */
	static void itostr256(size_t value, std::string& data);

	/**
	 * @brief str256toi Converts the string to a value, reading bytes as base 256 digits.
	 * @param data The byte string to read.
	 * @param value The value to create.
	 */
	static void str256toi(const std::string& data, size_t& value);

	/**
	 * @brief send Send this packet.
	 * @param socket The socket to send to.
	 */
	void send(int socket) const;
	/**
	 * @brief recv Read a packet.
	 * @param socket The socket to read from.
	 * @return True if it was a real packet, false if it the socket disconnected.
	 */
	auto recv(int socket) -> bool;

	// getters
	auto length_header() -> unsigned char {return m_length_header;}
	auto length_data() -> const std::string& {return m_length_data;}
	auto data() -> const std::string& {return m_data;}
};

#endif // PACKET_H
