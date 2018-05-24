#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>


typedef ws_websocketpp::config::asio_client::message_type::ptr message_ptr;
typedef ws_websocketpp::lib::function<void(ws_websocketpp::connection_hdl, message_ptr)> message_handler;
typedef ws_websocketpp::lib::function<void(ws_websocketpp::connection_hdl)> close_handler;
typedef ws_websocketpp::client<ws_websocketpp::config::asio_client> ws_client;
typedef ws_websocketpp::client<ws_websocketpp::config::asio_tls_client> wss_client;

// The Client interface is mostly a thin wrapper for the
// ws_websocketpp::client<T> class that is typedefed above to ws_client and
// wss_client. Client in turn has derived template class ClientImpl<T>, which
// can take ws_client/wss_client classes.
//
// One important difference between the ClientImpl<T> class and the ws_client/
// wss_client classes is that ClientImpl<T> stores the T::connection_ptr
// object, so that it doesn't have to be passed to some of the methods like
// connect(). This is so that all instances of the template class can use the
// same Client interface.

class Client {
public:
  virtual void set_access_channels(ws_websocketpp::log::level channels) = 0;
  virtual void clear_access_channels(ws_websocketpp::log::level channels) = 0;
  virtual void init_asio() = 0;
  virtual void set_tls_init_handler(ws_websocketpp::transport::asio::tls_socket::tls_init_handler h) = 0;
  virtual void set_open_handler(ws_websocketpp::open_handler h) = 0;
  virtual void set_message_handler(message_handler h) = 0;
  virtual void set_close_handler(close_handler h) = 0;
  virtual void set_fail_handler(ws_websocketpp::fail_handler h) = 0;

  virtual void setup_connection(std::string location, ws_websocketpp::lib::error_code &ec) = 0;
  virtual void append_header(std::string key, std::string value) = 0;
  virtual void connect() = 0;

  virtual std::size_t run_one() = 0;
  virtual ws_websocketpp::lib::asio::io_service& get_io_service() = 0;
  virtual std::size_t poll() = 0;
  virtual void send(std::string const& payload,
                    ws_websocketpp::frame::opcode::value op = ws_websocketpp::frame::opcode::text) = 0;
  virtual void send(void const * payload, size_t len,
                    ws_websocketpp::frame::opcode::value op = ws_websocketpp::frame::opcode::binary) = 0;
  virtual void reset() = 0;
  virtual void close(ws_websocketpp::close::status::value const code, std::string const & reason) = 0;
  virtual bool stopped() = 0;
};


template <class T>
class ClientImpl : public Client {

public:
  void set_access_channels(ws_websocketpp::log::level channels) {
    client.set_access_channels(channels);
  };
  void clear_access_channels(ws_websocketpp::log::level channels) {
    client.clear_access_channels(channels);
  };
  void init_asio() {
    client.init_asio();
  };
  void set_tls_init_handler(ws_websocketpp::transport::asio::tls_socket::tls_init_handler h);
  void set_open_handler(ws_websocketpp::open_handler h) {
    client.set_open_handler(h);
  };
  void set_message_handler(message_handler h) {
    client.set_message_handler(h);
  };
  void set_close_handler(close_handler h) {
    client.set_close_handler(h);
  };
  void set_fail_handler(ws_websocketpp::fail_handler h) {
    client.set_fail_handler(h);
  };

  void setup_connection(std::string location, ws_websocketpp::lib::error_code &ec) {
    this->con = client.get_connection(location, ec);
  };
  void append_header(std::string key, std::string value) {
    this->con->append_header(key, value);
  }
  void connect() {
    client.connect(this->con);
  };

  std::size_t run_one() {
    return client.run_one();
  };
  ws_websocketpp::lib::asio::io_service& get_io_service() {
    return client.get_io_service();
  };
  std::size_t poll() {
    return client.poll();
  };
  void send(std::string const& payload, ws_websocketpp::frame::opcode::value op) {
    client.send(this->con, payload, op);
  };
  void send(void const* payload, size_t len, ws_websocketpp::frame::opcode::value op) {
    client.send(this->con, payload, len, op);
  };
  void reset() {
    client.reset();
  };
  void close(ws_websocketpp::close::status::value const code, std::string const& reason) {
    client.close(this->con, code, reason);
  };
  bool stopped() {
    return client.stopped();
  };

private:
  T client;
  typename T::connection_ptr con;
};



// Specializations for set_tls_init_handler()
template <>
void ClientImpl<wss_client>::set_tls_init_handler(ws_websocketpp::transport::asio::tls_socket::tls_init_handler h) {
  client.set_tls_init_handler(h);
}

template <>
void ClientImpl<ws_client>::set_tls_init_handler(ws_websocketpp::transport::asio::tls_socket::tls_init_handler h) {
  throw std::runtime_error("Can't set TLS init handler for ws:// connection.");
}

#endif
