# simplesockets - Tutorial

This tutorial doesn't cover everything, please refer to the header files for more operations.

1. Basics
2. Address
3. Packet
4. SingleClient
5. SingleServer
6. MultiServer

# 1. Basics
Choose which header to use:
- Are you a client? Use `singleclient.h`
- Do you serve only a single client at a time? `Use singleserver.h`
- Do you serve mutliple clients at once? Use `multiserver.h`
All headers include `address.h` and `packet.h`, which we will cover now.

# 2. Address
If you are a client:
```C++
Address my_addr("<IP or Hostname of server>", "<Port of server>");
```
If you are a server (type doesn't matter):
```C++
Address my_addr(nullptr, "<Port you want to bind to>");
```
since a server doesn't connect, only listen on localhost.

# 3. Packet
simplesockets transfers data using a simple packet system.
A packet doesn't actually store the data, only the header.
Therefore, your data object must be permanent.
```C++
// Won't work
Packet p("some data");

// Will work
std::string data = "some data";
Packet p(data);
```

# 4. SingleClient
```C++
Address addr("www.example.com", "42");
SingleClient client;
if(!client.connect(addr)) {
	// error handling
}

std::string data = "hello";
client.sendPacket(data);
client.recvPacket(data); // data is now changed
```

# 5. SingleServer
Identical to SingleClient except for the name.
```C++
Address addr(nullptr, "42");
SingleServer server;
if(!server.start(addr)) {
	// error handling
}

// Packet handling identical to SingleClient
```

# 6. MultiServer
This one is interesting: `start()` doesn't immediately listen to client connections.
Instead, `recvPacket()` is used both to receive data from clients as well as to handle connecting and disconnecting clients.
```C++
Address addr(nullptr, "42");
MultiServer server;
if(!server.start(addr)) {
	// error handling
}

std::string data;
Packet p(data);

while(true) {
	// Storage for MultiServer status information
	size_t client_id; // indicates the origin of the received data
	ConInfo info; // indicates the type of the received data

	server.recvPacket(p, client_id, info);
	switch(info) {
	case ConInfo::CONNECT:
		// do stuff on new connection
		std::cout << "New client in slot " << client_id << '\n';
		break;
	case ConInfo::DISCONNECT:
		// do stuff on disconnect
		std::cout << "Client disconnected from slot " << client_id << '\n';
		break;
	case ConInfo::NORMAL:
		// a client has given data to us
		std::cout << "From client " << client_id << " : " << p.data() << '\n';

		auto response = std::string("thanks, ") + std::to_string(client_id);
		p = Packet(response);
		server.sendPacket(p, client_id); // thank sender for their data
		break;
	}
}
```
You can also add the argument `false` to `recvPacket` to forbid new connections.
