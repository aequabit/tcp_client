#pragma once
#define WIN32_LEAN_AND_MEAN
#include <functional>
#include <thread>
#include "client_exception.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <cstring>
#include <cstdio>
#include <unistd.h>
#endif

#define CUSTOM_EXCEPTION(NAME) \
    class NAME : public virtual std::exception \
    {\
    private:\
        std::string m_message;\
    public:\
        NAME(std::string const &message) : m_message(message) {}\
        const char *what() const throw() {\
            return m_message.c_str();\
        }\
    };

namespace tcp {
  class client {
    CUSTOM_EXCEPTION(wsa_startup_failed);
    CUSTOM_EXCEPTION(hostname_resolve_failed);
    CUSTOM_EXCEPTION(socket_creation_failed);
    CUSTOM_EXCEPTION(connect_failed);
    CUSTOM_EXCEPTION(disconnect_failed);
    CUSTOM_EXCEPTION(send_failed);
    CUSTOM_EXCEPTION(receive_failed);

   private:
    const char *m_host;
    unsigned short m_port;
#ifdef _WIN32
    SOCKET m_socket;
#else
    int m_socket;
#endif
    addrinfo *m_address;
    std::thread m_read_thread;
    std::function<void(char *)> m_data_callback;

   public:
    client(const char *host, unsigned short port);
    void connect();
    void disconnect();
    void send(const char *payload);
    void set_data_callback(const std::function<void(const char *)> &callback);
  };
}
