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
  virtual void set_error_channels(ws_websocketpp::log::level channels) = 0;
  virtual void clear_error_channels(ws_websocketpp::log::level channels) = 0;
  virtual void update_log_channels(std::string accessOrError, std::string setOrClear, Rcpp::CharacterVector logChannels) = 0;
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
  void set_error_channels(ws_websocketpp::log::level channels) {
    client.set_error_channels(channels);
  };
  void clear_error_channels(ws_websocketpp::log::level channels) {
    client.clear_error_channels(channels);
  };
  void update_log_channels(std::string accessOrError, std::string setOrClear, Rcpp::CharacterVector logChannels) {
    if (logChannels.size() == 0) return;
    ws_websocketpp::log::level channel;
    std::string fnName = accessOrError + "_" + setOrClear;

    for (int i = 0; i < logChannels.size(); i++) {
      if (accessOrError == "access") {
        channel = getAccessLogLevel(std::string(logChannels[i]));
        if (setOrClear == "set")
          client.set_access_channels(channel);
        else if (setOrClear == "clear")
          client.clear_access_channels(channel);

      } else if (accessOrError == "error") {
        channel = getErrorLogLevel(std::string(logChannels[i]));
        if (setOrClear == "set")
          client.set_error_channels(channel);
        else if (setOrClear == "clear")
          client.clear_error_channels(channel);
      }
    }
  }
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

  ws_websocketpp::log::level getAccessLogLevel(std::string logLevel) {
    if      (logLevel == "none")            return ws_websocketpp::log::alevel::none;
    else if (logLevel == "connect")         return ws_websocketpp::log::alevel::connect;
    else if (logLevel == "disconnect")      return ws_websocketpp::log::alevel::disconnect;
    else if (logLevel == "control")         return ws_websocketpp::log::alevel::control;
    else if (logLevel == "frame_header")    return ws_websocketpp::log::alevel::frame_header;
    else if (logLevel == "frame_payload")   return ws_websocketpp::log::alevel::frame_payload;
    else if (logLevel == "message_header")  return ws_websocketpp::log::alevel::message_header;
    else if (logLevel == "message_payload") return ws_websocketpp::log::alevel::message_payload;
    else if (logLevel == "endpoint")        return ws_websocketpp::log::alevel::endpoint;
    else if (logLevel == "debug_handshake") return ws_websocketpp::log::alevel::debug_handshake;
    else if (logLevel == "debug_close")     return ws_websocketpp::log::alevel::debug_close;
    else if (logLevel == "devel")           return ws_websocketpp::log::alevel::devel;
    else if (logLevel == "app")             return ws_websocketpp::log::alevel::app;
    else if (logLevel == "http")            return ws_websocketpp::log::alevel::http;
    else if (logLevel == "fail")            return ws_websocketpp::log::alevel::fail;
    else if (logLevel == "access_core")     return ws_websocketpp::log::alevel::access_core;
    else if (logLevel == "all")             return ws_websocketpp::log::alevel::all;
    else
      Rcpp::stop("logLevel must be one of the access logging levels (alevel).  See https://www.zaphoyd.com/websocketpp/manual/reference/logging");
  }

  ws_websocketpp::log::level getErrorLogLevel(std::string logLevel) {
    if      (logLevel == "none")    return ws_websocketpp::log::elevel::none;
    else if (logLevel == "devel")   return ws_websocketpp::log::elevel::devel;
    else if (logLevel == "library") return ws_websocketpp::log::elevel::library;
    else if (logLevel == "info")    return ws_websocketpp::log::elevel::info;
    else if (logLevel == "warn")    return ws_websocketpp::log::elevel::warn;
    else if (logLevel == "rerror")  return ws_websocketpp::log::elevel::rerror;
    else if (logLevel == "fatal")   return ws_websocketpp::log::elevel::fatal;
    else if (logLevel == "all")     return ws_websocketpp::log::elevel::all;
    else
      Rcpp::stop("logLevel must be one of the error logging levels (elevel).  See https://www.zaphoyd.com/websocketpp/manual/reference/logging");
  }

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
