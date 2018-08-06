#define _CRT_SECURE_NO_WARNINGS

#include "tcp/client.h"

struct test_t {
  double a;
  bool b;
  unsigned long long c;
};

void on_receive(const char* payload) {
  printf("received data: %s\n", payload);
}

int main(const int argc, const char **argv) {
  tcp::client *client = nullptr;

  try
  {
    client = new tcp::client("127.0.0.1", 13337);
    client->set_data_callback(on_receive);
    client->connect();
  } catch (std::exception& ex)
  {
    printf("exception: %s", ex.what());
  }

  while (true) {
    char buffer[4096];
    scanf("%s", buffer);

    if (!strcmp(buffer, "disconnect")) {
      client->disconnect();
    }
    else {
      client->send(buffer);
    }
  }
}