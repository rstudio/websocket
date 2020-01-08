#ifndef WRAPPED_PRINT_HPP
#define WRAPPED_PRINT_HPP

// This file provides substitutes for Rprintf, REprintf, Rcpp::Rcout, and
// Rcpp::Rcerr. However, when code on a background thread calls one of these
// R-prefixed functions, it is very unsafe and can lead to memory corruption.
//
// It isn't possible to switch to plain old printf, std::cerr, and std::cout,
// because R CMD check will flag these and raise a WARNING.
//
// This file provides substitutes which will not cause memory corruption and
// will not result in warnings from R CMD check.

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>

#include <iostream>
#include <streambuf>

// It's not safe to call REprintf from the background thread but we need some
// way to output error messages. R CMD check does not it if the code uses the
// symbols stdout, stderr, and printf, so this function is a way to avoid
// those. It's to calling `fprintf(stderr, ...)`.
inline void err_printf(const char *fmt, ...) {
  const size_t max_size = 4096;
  char buf[max_size];

  va_list args;
  va_start(args, fmt);
  int n = vsnprintf(buf, max_size, fmt, args);
  va_end(args);

  if (n == -1)
    return;

  ssize_t res = write(STDERR_FILENO, buf, n);
  // This is here simply to avoid a warning about "ignoring return value" of
  // the write(), on some compilers. (Seen with gcc 4.4.7 on RHEL 6)
  res += 0;
}

// Same as above, but for printf (or Rprintf).
inline void out_printf(const char *fmt, ...) {
  const size_t max_size = 4096;
  char buf[max_size];

  va_list args;
  va_start(args, fmt);
  int n = vsnprintf(buf, max_size, fmt, args);
  va_end(args);

  if (n == -1)
    return;

  ssize_t res = write(STDOUT_FILENO, buf, n);
  // This is here simply to avoid a warning about "ignoring return value" of
  // the write(), on some compilers. (Seen with gcc 4.4.7 on RHEL 6)
  res += 0;
}


class WrappedStreambuf : public std::streambuf {
public:
  // out_type: true means stdout; false means stderr.
  WrappedStreambuf(bool out_type) : out_type(out_type) {}

protected:
  std::streamsize xsputn(const char *s, std::streamsize n) {
    if (out_type) {
      err_printf("%.*s", n, s);
    } else {
      out_printf("%.*s", n, s);
    }
    return n;
  };

private:
  bool out_type;
};


namespace WrappedOstream {
  static WrappedStreambuf out_buf(true);
  static WrappedStreambuf err_buf(false);
  static std::ostream cout(&out_buf);
  static std::ostream cerr(&err_buf);
};

#endif
