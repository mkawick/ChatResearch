
start ./debug/ChatServer.exe listen.address=localhost listen.port=7400 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=21002 

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/GatewayServer.exe server.name=Gateway1 listen.port=9600 chat.address=localhost chat.port=7400 login.port=7600 login.address=localhost asset.address=localhost asset.port=7300

exit

