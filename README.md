NESPRO
======

## code checklist
- [x] create makefile for http and https compile
  - ensure proper flags: `-lcurl -lssl -lcrypto` (`-lssl` & `-lcrypto` for HTTPS only)
- [ ] configure JSON nginx like config structure for server
- [ ] write description README
  - describe project purpose (HTTP/HTTPS file server)
  - how to compile and run
  - any prerequisites (e.g., OpenSSL, Linux only)
- [ ] inherit HTTPS from HTTP class and overload using openSSL  
  openSSL functionality to be used:
  - `SSL_new()`
  - `SSL_set_fd()`
  - `SSL_accept()` (performs the handshake)
  - `SSL_read()` / `SSL_write()` instead of `read()` / `write()`

  we create bind and accept the socket with POSIX but then hand off to openSSL for encrypted TLS

## deployment checklist
- [ ] domain name -> ip
- [ ] port forwarding (443 → my machine)
- [ ] assign static LAN IP to server
- [ ] generate SSL certs (lets encrypt)
- [ ] allow HTTPS through firewall
