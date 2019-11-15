#include "packet.h"
#include <cmath>
#include <stdexcept>
#include <netdb.h>
#include <cerrno>
using namespace std;

static constexpr size_t BYTESIZE = 8;
static constexpr size_t BYTEBASE = 1 << BYTESIZE;

Packet::Packet(const string& s): m_data(move(s)) {
	itostr256(s.size(), m_length_data); // write length data
	if(m_length_data.size() > static_cast<unsigned char>(-1)) {
		throw overflow_error("Size of length data larger than one byte!");
	}
	m_length_header = static_cast<unsigned char>(m_length_data.size()); // write length header
}

void Packet::itostr256(size_t value, string& target) {
	// special case
	if(value == 0) {
		target = {'\0'};
		return;
	}

	// init target
	auto val_size = static_cast<size_t>(floor(log(value) / log(BYTEBASE)) + 1);
	target = string(val_size, '\0');

	// convert value to base BYTEBASE
	for(size_t i = val_size-1; i < val_size; --i) {
		target[val_size-i-1] = static_cast<char>(floor(value / pow(BYTEBASE, i))); // get current factor
		value %= static_cast<size_t>(pow(BYTEBASE, i)); // shift value
	}
}

void Packet::str256toi(const std::string& data, size_t& value) {
	value = 0;
	for(auto byte : data) {
		value <<= BYTESIZE; // shift all bits over by one byte, does nothing on first iteration since value is 0
		value += static_cast<size_t>(static_cast<unsigned char>(byte)); // add current byte. yes the ugly cast is neccessary.
	}
}

void Packet::send(int socket) const {
	ssize_t tmp = 0; // return value for ::send(), signed to test for errors

	// send length header
	while(tmp < 1) {
		// create buffer
		string lenstr = string(1, static_cast<char>(m_length_header));
		const char* buf = lenstr.c_str();

		tmp = ::send(socket, buf, 1, 0); // 1 is size of length header, alwyas one byte
		if(tmp < 0) { // local error
			throw runtime_error(string("send: ").append(string(gai_strerror(errno))));
		}
	}

	// send length data
	size_t total = m_length_data.size(); // length of data
	size_t sent = 0; // how many bytes are already sent
	const char* length_buf = m_length_data.c_str();
	while(sent < total) {
		tmp = ::send(socket, length_buf + sent, total - sent, 0);
		if(tmp < 0) { // local error
			throw runtime_error(string("send: ").append(string(gai_strerror(errno))));
		}
		sent += static_cast<size_t>(tmp);
	}

	// send packet data
	total = m_data.size();
	sent = 0;
	const char* data_buf = m_data.c_str();
	while(sent < total) {
		tmp = ::send(socket, data_buf + sent, total - sent, 0);
		if(tmp < 0) { // local error
			throw runtime_error(string("send: ").append(string(gai_strerror(errno))));
		}
		sent += static_cast<size_t>(tmp);
	}
}

auto Packet::recv(int socket) -> bool {
	ssize_t tmp = 0; // return value for ::recv

	// receive length header
	char length_header_buf[1];
	while(tmp < 1) {
		tmp = ::recv(socket, length_header_buf, 1, 0); // 1 is size of length header, alwyas one byte
		if(tmp == 0) {
			m_length_header = '\0';
			m_length_data = {};
			m_data = {};
			return false;
		}
		if(tmp < 0) { // local error
			throw runtime_error(string("recv: ").append(string(gai_strerror(errno))));
		}
	}
	m_length_header = static_cast<unsigned char>(length_header_buf[0]);

	// receive length data
	size_t received = 0;
	char* length_data_buf = new char[m_length_header+1];
	length_data_buf[m_length_header] = '\0';

	while(received < m_length_header) {
		tmp = ::recv(socket, length_data_buf + received, m_length_header - received, MSG_WAITALL);
		if(tmp == 0) {
			m_length_header = '\0';
			m_length_data = {};
			m_data = {};
			delete [] length_data_buf;
			return false;
		}
		if(tmp < 0) { // local error
			delete [] length_data_buf;
			throw runtime_error(string("recv: ").append(string(gai_strerror(errno))));
		}
		received += static_cast<size_t>(tmp);
	}

	m_length_data = string(length_data_buf);
	size_t length_data;
	str256toi(m_length_data, length_data);
	delete[] length_data_buf;

	//receive packet data
	received = 0;
	char* data_buf = new char[length_data+1];
	data_buf[length_data] = '\0';

	while(received < length_data) {
		tmp = ::recv(socket, data_buf + received, length_data - received, 0);
		if(tmp == 0) {
			m_length_header = '\0';
			m_length_data = {};
			m_data = {};
			delete [] data_buf;
			return false;
		}
		if(tmp < 0) { // local error
			delete[] data_buf;
			throw runtime_error(string("recv: ").append(string(gai_strerror(errno))));
		}
		received += static_cast<size_t>(tmp);
	}

	m_data = string(data_buf);
	delete [] data_buf;

	return true;
}