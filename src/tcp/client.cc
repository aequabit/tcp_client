#include "client.h"

#if defined(_DEBUG) || !defined(NDEBUG)
#define DBG_PRINT(format, ...) printf(format, ##__VA_ARGS__);
#else
#define DBG_PRINT(format, ...)
#endif

namespace tcp {
  client::client(const char *host, const unsigned short port) : m_host(host), m_port(port) {
#ifdef _WIN32
    WSADATA wsa_data;

    DBG_PRINT("initializing winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
      DBG_PRINT("err\n");
      throw client_exception::wsa_startup_failed("WSAStartup failed");
    }
    DBG_PRINT("ok\n");
#endif

    struct addrinfo hints{};
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    char port_str[8];
#ifdef _WIN32
    sprintf_s(port_str, sizeof(port_str), "%d", m_port);
#else
    sprintf(port_str, "%d", m_port);
#endif

    DBG_PRINT("resolving host...");
    if (getaddrinfo(m_host, port_str, &hints, &m_address) != 0) {
      DBG_PRINT("err\n");
#ifdef _WIN32
      WSACleanup();
#endif
      throw client_exception::hostname_resolve_failed("Failed to resolve hostname");
    }
    DBG_PRINT("ok\n");

    DBG_PRINT("creating socket...");
    m_socket = socket(m_address->ai_family, m_address->ai_socktype, m_address->ai_protocol);
#ifdef _WIN32
    if (m_socket == INVALID_SOCKET) {
#else
    if (m_socket == -1) {
#endif
      DBG_PRINT("err\n");
      freeaddrinfo(m_address);
#ifdef _WIN32
      WSACleanup();
#endif
      throw client_exception::socket_creation_failed("Failed to create socket");
    }
    DBG_PRINT("ok\n");
  }

  void client::connect() {
    DBG_PRINT("connecting to server...");
#ifdef _WIN32
    if (::connect(m_socket, m_address->ai_addr, (int) m_address->ai_addrlen) == SOCKET_ERROR) {
#else
    if (::connect(m_socket, m_address->ai_addr, (int) m_address->ai_addrlen) == -1) {
#endif
      DBG_PRINT("err\n");
      freeaddrinfo(m_address);

#ifdef _WIN32
      closesocket(m_socket);
      m_socket = INVALID_SOCKET;
      WSACleanup();
#else
      close(m_socket);
      m_socket = -1;
#endif
      throw client_exception::connect_failed("Failed to connect to server");
    }
    DBG_PRINT("ok\n");

    freeaddrinfo(m_address);

    m_read_thread = std::thread([this]() {
      char buffer[4096];
      while (true) {
        memset(buffer, 0, sizeof(buffer));

        int result = recv(m_socket, buffer, sizeof(buffer), 0);

        if (result > 0) {
          if (m_data_callback) {
            m_data_callback(buffer);
          }
        } else if (result == 0) {
          disconnect();
          break;
        } else {
          DBG_PRINT("receive failed\n");

#ifdef _WIN32
          closesocket(m_socket);
          WSACleanup();
#else
          close(m_socket);
#endif
          throw client_exception::receive_failed("Receiving data failed");
        }
      }
    });
  }

  void client::disconnect() {
    DBG_PRINT("shutting down socket...");

#ifdef _WIN32
    if (shutdown(m_socket, SD_SEND) == SOCKET_ERROR) {
#else
    if (shutdown(m_socket, SHUT_WR) == -1) {
#endif
      DBG_PRINT("err\n");

#ifdef _WIN32
      closesocket(m_socket);
      WSACleanup();
#else
      close(m_socket);
#endif
      throw client_exception::disconnect_failed("Failed to close the connection");
    }
    DBG_PRINT("ok\n");

#ifdef _WIN32
    closesocket(m_socket);
    WSACleanup();
#else
    close(m_socket);
#endif
  }

  void client::send(const char *payload) {
    DBG_PRINT("sending %d bytes...", strlen(payload));
#ifdef _WIN32
    if (::send(m_socket, payload, (int) strlen(payload), 0) == SOCKET_ERROR) {
#else
    if (::send(m_socket, payload, (int) strlen(payload), 0) == -1) {
#endif
      DBG_PRINT("err\n");

#ifdef _WIN32
      closesocket(m_socket);
      WSACleanup();
#else
      close(m_socket);
#endif
      throw client_exception::send_failed("Failed to send payload");
    }
    DBG_PRINT("ok\n");
  }

  void client::set_data_callback(const std::function<void(const char *)> &callback) {
    m_data_callback = callback;
  }
}