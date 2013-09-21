
start ./debug/ChatServer.exe listen.address=127.0.0.1 listen.port=9601 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=24604 

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/GatewayServer.exe listen.address=127.0.0.1 listen.port=9600 chat.address=127.0.0.1 chat.port=9601 asset.port=9700

exit

