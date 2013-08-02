REM cd C:/projects/MberSW/main
REM start C:/projects/MberSW/main/WorkArea/game_serverD.exe listen.port=23550 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=24604
REM cd C:/projects/Mber/ServerStack

REM Pushd C:/projects/MberSW/main
REM start C:/projects/MberSW/main/WorkArea/game_serverD.exe listen.port=23550 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=24604
REM Popd


start ./debug/LoginServer.exe listen.address=127.0.0.1 listen.port=3072 db.address=10.16.4.44 db.port=3306 db.username=admin db.password=Pz5328!@ db.schema=playdek game.port=24604 contact.port=9802 asset.port=10002

start ./debug/ChatServer.exe listen.address=127.0.0.1 listen.port=9601 db.address=10.16.4.44 db.port=3306 db.username=admin db.password=Pz5328!@ db.schema=playdek

start ./debug/ContactsServer.exe listen.address=127.0.0.1 listen.port=9800 db.address=10.16.4.44 db.port=3306 db.username=admin db.password=Pz5328!@ db.schema=playdek s2s.port=9802

start ./debug/AssetDeliveryServer.exe listen.address=127.0.0.1 listen.port=9700 db.address=10.16.4.44 db.port=3306 db.username=admin db.password=Pz5328!@ db.schema=playdek




REM start ./debug/GameServer.exe db.port=16384 listen.address=127.0.0.1 listen.port=23995

start ./debug/GatewayServer.exe listen.address=127.0.0.1 listen.port=9600 chat.address=127.0.0.1 chat.port=9601

exit

