#ifndef PROTOCOL_H_32989137418932748923
#define PROTOCOL_H_32989137418932748923
#include <memory>
struct Output
{
  unsigned int length;
  char *content;
};

struct WriteStream
{
  virtual int write_to_stream(char *out, unsigned int len) = 0;
};

struct Protocol
{
  Protocol() = default;
  virtual ~Protocol() = default;

  virtual void register_write_stream(std::weak_ptr<WriteStream> stream) = 0;

  virtual int routing_and_answser(char *income, unsigned int len) = 0;

  virtual int query_status() = 0;
};

#endif