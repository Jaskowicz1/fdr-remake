#include "../include/rcon.h"

rcon::rcon(const std::string& addr, const unsigned int _port, const std::string& pass) : address(addr), port(_port), password(pass) {
    
	std::cout << "Attempting connection to RCON server..." << "\n";
    
	if(!connect()) {
		std::cout << "RCON is aborting as it failed to initiate." << "\n";
		close(sock);
		return;
	}
    
	connected = true;
    
	std::cout << "Connected successfully! Sending login data..." << "\n";

	// The server will send SERVERDATA_AUTH_RESPONSE once it's happy.
	send_data_sync(pass, 1, data_type::SERVERDATA_AUTH);
    
	std::thread queue_runner([this]() {
		while(connected) {
			if(requests_queued.empty()) {
				continue;
			}
            
			for(rcon_queued_request request : requests_queued) {
				// Send data to callback if it's been set.
				if(request.callback)
					request.callback(send_data_sync(request.data, request.id, request.type));
				else
					send_data_sync(request.data, request.id, request.type, false);
			}
            
			requests_queued.clear();
		}
	});
    
	queue_runner.detach();
};

void rcon::send_data(const std::string& data, const int32_t id, data_type type, std::function<void(const std::string& retrieved_data)> callback) {
	requests_queued.emplace_back(data, id, type, callback);
}

const std::string rcon::send_data_sync(const std::string data, const int32_t id, data_type type, bool feedback) {
	if(!connected) {
		std::cout << "Cannot send data when not connected." << "\n";
		return "";
	}
    
	unsigned long long packet_len = data.length() + HEADER_SIZE;
	unsigned char packet[packet_len];
	form_packet(packet, data, id, type);
    
	if(::send(sock, packet, packet_len, 0) < 0) {
		std::cout << "Sending failed!" << "\n";
		return "";
	}
    
	if(type != SERVERDATA_EXECCOMMAND || !feedback) {
		return "";
	}
    
	// Server will send a SERVERDATA_RESPONSE_VALUE packet.
	return receive_information(id);
}

bool rcon::connect() {
	// Create new TCP socket.
	sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(sock == -1) {
		std::cout << "Failed to open socket." << "\n";
		return false;
	}
    
	// Setup port, address, and family.
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(address.c_str());
	server.sin_port = htons(port);

	// Make it non-blocking.
	fcntl(sock, F_SETFL, O_NONBLOCK);

	// Set a timeout of 4 milliseconds.
	struct timeval tv;
	tv.tv_sec = 4;
	tv.tv_usec = 0;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(sock, &fds);

	// Create temp status
	int status = -1;
    
	// Connect to the socket and set the status to our temp status.
	if((status = ::connect(sock, (struct sockaddr *)&server, sizeof(server))) == -1) {
		if(errno != EINPROGRESS) {
			return false;
		}
	}

	status = select(sock +1, NULL, &fds, NULL, &tv);
	fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) & ~O_NONBLOCK);

	// If status wasn't zero, we successfully connected.
	return status != 0;
}

void rcon::form_packet(unsigned char packet[], const std::string& data, int32_t id, int32_t type) {
	const char nullbytes[] = {'\x00', '\x00'};
	const int32_t min_size = sizeof(id) + sizeof(type) + sizeof(nullbytes);
	const int32_t data_size = static_cast<int32_t>(data.size()) + min_size;
    
	if(data_size > 4096) {
		std::cout << "This packet is too big to send. Please generate a smaller packet." << "\n";
		return;
	}

	(memset(packet, '\0', data_size), (void) 0);

	// Each part is 4 bytes, so we allocate each part 4 bytes away.
	packet[0] = data_size;
	packet[4] = id;
	packet[8] = type;

	int write_nullbytes_at{0};

	const char* data_chars = data.c_str();

	for(int i = 0; i < data_size; i++) {
		packet[12 + i] = data_chars[i];
		write_nullbytes_at = 13 + i;
	}
}

std::string rcon::receive_information(int32_t id) {
	// Whilst this loop is better than a while loop,
	// it should really just keep going for a certain amount of seconds.
	for(int i=0; i < 500; i++) {
		rcon_packet packet = read_packet();
		unsigned char* buffer = packet.data;

		int offset = packet.bytes - HEADER_SIZE + 3;

		if(offset == -1)
			continue;

		std::string part(&buffer[8], &buffer[8] + offset);

		if(byte32_to_int(packet.data) == id) {
			return part;
		}
	}
	return "";
}

rcon_packet rcon::read_packet() {
	size_t packet_length = read_packet_length();
	auto* buffer = new unsigned char[packet_length]{0};
	unsigned int bytes = 0;

	do {
		bytes += ::recv(sock, buffer, packet_length - bytes, 0);
	} while(bytes < packet_length);

	return {bytes, buffer};
}
