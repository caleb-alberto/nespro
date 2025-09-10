NESPRO
======

## code checklist
- [x] create makefile for http and https compile
- [x] configure sending images
- [x] configure JSON nginx like config structure for server
- [x] inherit HTTPS from HTTP class and overload using openSSL
- [ ] dedicated threads for each connection
- [ ] write description README
  - describe project purpose (HTTP/HTTPS file server)
  - how to compile and run
  - any prerequisites (e.g., OpenSSL, Linux only)

## deployment checklist
- [x] generate SSL certs (lets encrypt)
- [x] domain name -> ip
- [ ] port forwarding (443 → my machine)
- [ ] assign static LAN IP to server
- [ ] allow HTTPS through firewall
- [ ] add timeouts and limits (max header size, max body size, max concurrent conns).
- [ ] keep a watchdog (systemd, supervisord) to restart on crashes.
- [ ] run server as a non-privileged user inside a container
