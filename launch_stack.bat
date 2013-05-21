start ./debug/LoginServer.exe listen.address=127.0.0.1 listen.port=3072 db.address=10.16.4.44 db.port=3306 db.username=admin db.password=Pz5328!@ db.schema=pleiades

start ./debug/ChatServer.exe listen.address=127.0.0.1 listen.port=9601 db.address=10.16.4.44 db.port=3306 db.username=admin db.password=Pz5328!@ db.schema=pleiades

REM start ./debug/GameServer.exe db.port=16384 listen.address=127.0.0.1 listen.port=23995

start ./debug/GatewayServer.exe listen.address=127.0.0.1 listen.port=9600 chat.address=127.0.0.1 chat.port=9601

exit