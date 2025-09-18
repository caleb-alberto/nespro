TODO
======
- [x] create makefile for http and https compile
- [x] configure sending images
- [x] configure JSON nginx like config structure for server
- [x] inherit HTTPS from HTTP class and overload using openSSL
- [x] dedicated threads for each connection
- [ ] set socket to non blocking so connecting with telnet doesnt block on `SSL_accept`
- [ ] create sleep() timers to limit open connections from blocking real connections
- [ ] figure out how to et telnet/openSSL to send more than one line
      (ie: include a host header) without being interupted
- [ ] add timeouts and limits (max header size, max body size).
