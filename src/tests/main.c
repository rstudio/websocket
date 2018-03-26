#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/engine.h>
#include <openssl/hmac.h>

int main(){
  OpenSSL_add_all_digests();
  OpenSSL_add_all_algorithms();
  OpenSSL_add_all_ciphers();
  ERR_load_crypto_strings();
  SSL_load_error_strings();
  SSL_library_init();
}
