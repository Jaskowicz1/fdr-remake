#pragma once

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <thread>

#define DEFAULT_TIMEOUT 4
#define HEADER_SIZE 14

enum data_type {
    SERVERDATA_AUTH = 3,
    SERVERDATA_AUTH_RESPONSE = 2,
    SERVERDATA_EXECCOMMAND = 2,
    SERVERDATA_RESPONSE_VALUE = 0
};

class rcon {
    const std::string address;
    const unsigned int port;
    const std::string password;
    unsigned int sock{0};
    bool connected{false};
    bool authorised{false};
    
public:
    
    rcon(const std::string& addr, const unsigned int _port, const std::string& pass);
    
    void send_data(const std::string& data, data_type type, std::function<void(const std::string& retrieved_data)> callback);
    
    const std::string send_data_sync(const std::string data, data_type type);
    
private:
    
    /**
     * @brief Connects to RCON using `address`, `port`, and `password`.
     * Those values are pre-filled when constructing this class.
     * 
     * @warning This should only ever be called by the constructor.
     * The constructor calls this function once it has filled in the required data and proceeds to login.
     */
    bool connect();
    
    void form_packet(unsigned char packet[], const std::string& data, unsigned long long packet_len, int id, int type) {
        unsigned long long data_len = packet_len + HEADER_SIZE;
        bzero(packet, packet_len);
        packet[0] = data_len - 2;
        packet[4] = id;
        packet[8] = type;
        for(int i = 0; i < data_len; i++)
            packet[12 +i] = data.c_str()[i];
    }
    
    std::string receive_information();
    
    unsigned char* read_packet(unsigned int& size, bool& can_sleep) const {
        size_t len = read_packet_len();
        if(can_sleep && len > 500){
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            can_sleep = false;
        }
        unsigned char* buffer = new unsigned char[len]{0};
        unsigned int bytes = 0;
        do {
            bytes += ::recv(sock, buffer, len -bytes, 0);
        }
        while(bytes < len);
        size = len;
        return buffer;
    }
    
    size_t read_packet_len() const{
        unsigned char* buffer = new unsigned char[4]{0};
        ::recv(sock, buffer, 4, 0);
        const size_t len = byte32_to_int(buffer);
        delete [] buffer;
        return len;
    }

    size_t byte32_to_int(unsigned char* buffer) const{
        return    static_cast<size_t>(
                    static_cast<unsigned char>(buffer[0])        |
                    static_cast<unsigned char>(buffer[1]) << 8    |
                    static_cast<unsigned char>(buffer[2]) << 16    |
                    static_cast<unsigned char>(buffer[3]) << 24 );
    }
};
