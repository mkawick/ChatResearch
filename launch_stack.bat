REM cd C:/projects/MberSW/main
REM start C:/projects/MberSW/main/WorkArea/game_serverD.exe listen.port=23550 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=24604
REM cd C:/projects/Mber/ServerStack

REM Pushd C:/projects/MberSW/main
REM start C:/projects/MberSW/main/WorkArea/game_serverD.exe listen.port=23550 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=24604
REM Popd


start ./debug/LoginServer.exe listen.address=127.0.0.1 listen.port=3072 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=24604 contact.port=9802 asset.port=9702

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/ChatServer.exe listen.address=127.0.0.1 listen.port=9601 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=24604 

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/ContactsServer.exe listen.address=127.0.0.1 listen.port=9800 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=9802

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/AssetDeliveryServer.exe listen.address=127.0.0.1 listen.port=9700 s2s.port=9702 game.port=24604 




REM start ./debug/GameServer.exe db.port=16384 listen.address=127.0.0.1 listen.port=23995

start ./debug/GatewayServer.exe listen.address=127.0.0.1 listen.port=9600 chat.address=127.0.0.1 chat.port=9601 asset.port=9700

exit

