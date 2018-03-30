/* -*-mode:c++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

#ifndef MEOW_MESSAGE_HH
#define MEOW_MESSAGE_HH

#include <string>
#include <queue>

#include "util/chunk.hh"

namespace meow {

  class Message
  {
  public:
    enum class OpCode: uint8_t
    {
      Hey = 0x1,
      Put,
      Get,
      Ping,
      Pong,
      Execute,
    };

  private:
    OpCode opcode_ { OpCode::Hey };
    uint32_t payload_length_ { 0 };
    std::string payload_ {};

  public:
    Message( const Chunk & chunk );
    Message( const OpCode opcode, string && payload );

    OpCode opcode() const { return opcode_; }
    uint32_t payload_length() const { return payload_length_; }
    const std::string payload() const { return payload_; }
  };

  class MessageParser
  {
  private:
    std::string raw_buffer_ {};
    std::queue<Message> completed_messages_ {};

  public:
    void parse( std::string && buf );

    bool empty() const { return completed_messages_.empty(); }
    const Message & front() const { return completed_messages_.front(); }
    void pop() { completed_messages_.pop(); }
  };

}

#endif /* MEOW_REQUEST_HH */
