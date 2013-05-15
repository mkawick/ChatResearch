start ./debug/LoginServer.exe db.port=16384 listen.address=127.0.0.1 listen.port=3072

start ./debug/ChatServer.exe db.port=16384 listen.address=127.0.0.1 listen.port=9601

start ./debug/GameServer.exe db.port=16384 listen.address=127.0.0.1 listen.port=23995

start ./debug/GatewayServer.exe listen.address=127.0.0.1 listen.port=9600 chat.address=127.0.0.1 chat.port=9601

exit