NESPRO
======

## code checklist
- [x] create makefile for http and https compile
  - ensure proper flags: `-lcurl -lssl -lcrypto` (`-lssl` & `-lcrypto` for HTTPS only)
- [x] configure sending images
- [ ] configure JSON nginx like config structure for server
- [ ] write description README
  - describe project purpose (HTTP/HTTPS file server)
  - how to compile and run
  - any prerequisites (e.g., OpenSSL, Linux only)
- [x] inherit HTTPS from HTTP class and overload using openSSL  

## deployment checklist
- [ ] domain name -> ip
- [ ] port forwarding (443 → my machine)
- [ ] assign static LAN IP to server
- [x] generate SSL certs (lets encrypt)
- [ ] allow HTTPS through firewall
