#include "rcon.h"

#include <iostream>

rcon::rcon(const std::string& addr, const unsigned int _port, const std::string& pass) : address(addr), port(_port), password(pass) {
    
    std::cout << "Attempting connection to RCON server..." << "\n";
    
    if(!connect()) {
        std::cout << "RCON is aborting as it failed to initiate." << "\n";
        close(sock);
        return;
    }
    
    connected = true;
    
    std::cout << "Connected successfully! Sending login data..." << "\n";

    std::string data_back = send_data_sync(pass, data_type::SERVERDATA_AUTH);
    
    if(data_back != "Authorised") {
        std::cout << "Failed to authorise." << "\n";
        close(sock);
        return;
    }
    
    authorised = true;
};

void rcon::send_data(const std::string& data, data_type type, std::function<void(const std::string& retrieved_data)> callback) {
    
}

bool rcon::connect() {
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(sock == -1) {
        std::cout << "Failed to open socket." << "\n";
        return false;
    }
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(address.c_str());
    server.sin_port = htons(port);

    fcntl(sock, F_SETFL, O_NONBLOCK);

    struct timeval tv;
    tv.tv_sec = 4;
    tv.tv_usec = 0;
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    int status = -1;
    if((status = ::connect(sock, (struct sockaddr *)&server, sizeof(server))) == -1)
        if(errno != EINPROGRESS)
            return false;

    status = select(sock +1, NULL, &fds, NULL, &tv);
    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) & ~O_NONBLOCK);
    if(status == 0)
        return false;
    
    return true;
}

const std::string rcon::send_data_sync(const std::string data, data_type type) {
    
    if(!connected) {
        std::cout << "Cannot send data when not connected." << "\n";
        return "";
    }
    
    if(!authorised && type != data_type::SERVERDATA_AUTH) {\
        std::cout << "RCON is not currently authorised to send any data. Please wait." << "\n";
        return "";
    }
    
    unsigned long long packet_len = data.length() + HEADER_SIZE;
    unsigned char packet[packet_len];
    form_packet(packet, data, packet_len, 50, type);
    if(::send(sock, packet, packet_len, 0) < 0) {
        std::cout << "Sending failed!" << "\n";
        return "";
    }
    
    if(type != SERVERDATA_EXECCOMMAND) {
        
        if(type == SERVERDATA_AUTH) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return "Authorised";
        }
        
        return "";
    }
    
    send_data_sync("", data_type::SERVERDATA_RESPONSE_VALUE);
    return receive_information();
}

std::string rcon::receive_information() {
    unsigned int bytes = 0;
    unsigned char* buffer = nullptr;
    std::string response;
    bool can_sleep = false;
    while(1){
        delete [] buffer;
        buffer = read_packet(bytes, can_sleep);
        std::cout << byte32_to_int(buffer) << "\n";
        if(byte32_to_int(buffer) == 50)
            break;
        int offset = bytes -HEADER_SIZE +3;
        if(offset == -1)
            continue;
        std::string part(&buffer[8], &buffer[8] +offset);
        std::cout << part << "\n";
        response += part;
    }
    delete [] buffer;
    buffer = read_packet(bytes, can_sleep);
    delete [] buffer;
    return response;
}
