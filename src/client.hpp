#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>


typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
typedef websocketpp::lib::function<void(websocketpp::connection_hdl, message_ptr)> message_handler;
typedef websocketpp::lib::function<void(websocketpp::connection_hdl)> close_handler;
typedef websocketpp::client<websocketpp::config::asio_client> ws_client;
typedef websocketpp::client<websocketpp::config::asio_tls_client> wss_client;

// The Client interface is mostly a thin wrapper for the
// websocketpp::client<T> class that is typedefed above to ws_client and
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
  virtual void set_access_channels(websocketpp::log::level channels) = 0;
  virtual void clear_access_channels(websocketpp::log::level channels) = 0;
  virtual void init_asio() = 0;
  virtual void set_tls_init_handler(websocketpp::transport::asio::tls_socket::tls_init_handler h) = 0;
  virtual void set_open_handler(websocketpp::open_handler h) = 0;
  virtual void set_message_handler(message_handler h) = 0;
  virtual void set_close_handler(close_handler h) = 0;
  virtual void set_fail_handler(websocketpp::fail_handler h) = 0;

  virtual void setup_connection(std::string location, websocketpp::lib::error_code &ec) = 0;
  virtual void connect() = 0;
  virtual std::size_t run_one() = 0;
  virtual websocketpp::lib::asio::io_service& get_io_service() = 0;
  virtual std::size_t poll() = 0;
  virtual void send(std::string const& payload,
                    websocketpp::frame::opcode::value op = websocketpp::frame::opcode::text) = 0;
  virtual void send(void const * payload, size_t len,
                    websocketpp::frame::opcode::value op = websocketpp::frame::opcode::binary) = 0;
  virtual void reset() = 0;
  virtual void close(websocketpp::close::status::value const code, std::string const & reason) = 0;
  virtual bool stopped() = 0;
};


template <class T>
class ClientImpl : public Client {

public:
  void set_access_channels(websocketpp::log::level channels) {
    client.set_access_channels(channels);
  };
  void clear_access_channels(websocketpp::log::level channels) {
    client.clear_access_channels(channels);
  };
  void init_asio() {
    client.init_asio();
  };
  void set_tls_init_handler(websocketpp::transport::asio::tls_socket::tls_init_handler h);
  void set_open_handler(websocketpp::open_handler h) {
    client.set_open_handler(h);
  };
  void set_message_handler(message_handler h) {
    client.set_message_handler(h);
  };
  void set_close_handler(close_handler h) {
    client.set_close_handler(h);
  };
  void set_fail_handler(websocketpp::fail_handler h) {
    client.set_fail_handler(h);
  };

  void setup_connection(std::string location, websocketpp::lib::error_code &ec) {
    this->con = client.get_connection(location, ec);
  };
  void connect() {
    client.connect(this->con);
  };
  std::size_t run_one() {
    return client.run_one();
  };
  websocketpp::lib::asio::io_service& get_io_service() {
    return client.get_io_service();
  };
  std::size_t poll() {
    return client.poll();
  };
  void send(std::string const& payload, websocketpp::frame::opcode::value op) {
    client.send(this->con, payload, op);
  };
  void send(void const* payload, size_t len, websocketpp::frame::opcode::value op) {
    client.send(this->con, payload, len, op);
  };
  void reset() {
    client.reset();
  };
  void close(websocketpp::close::status::value const code, std::string const& reason) {
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
void ClientImpl<wss_client>::set_tls_init_handler(websocketpp::transport::asio::tls_socket::tls_init_handler h) {
  client.set_tls_init_handler(h);
}

template <>
void ClientImpl<ws_client>::set_tls_init_handler(websocketpp::transport::asio::tls_socket::tls_init_handler h) {
  throw std::runtime_error("Can't set TLS init handler for ws:// connection.");
}

#endif
