REM cd C:/projects/MberSW/main
REM start C:/projects/MberSW/main/WorkArea/game_serverD.exe listen.port=21000 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=21002
REM cd C:/projects/Mber/ServerStack

REM Pushd C:/projects/MberSW/main
REM start C:/projects/MberSW/main/WorkArea/game_serverD.exe listen.port=21000 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=21002
REM Popd


#start ./debug/LoadBalancer.exe  listen.port=9500 

start ./debug/LoginServer.exe listen.port=7600 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=21002 contact.port=7502 asset.port=7302

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/ChatServer.exe listen.port=7400 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=21002

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/ContactsServer.exe listen.port=7500 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=7502

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/AssetDeliveryServer.exe listen.port=7300 s2s.port=7302 game.port=21002 

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/PurchaseServer.exe listen.port=7700 s2s.port=7702 s2s.address=localhost db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek


REM start ./debug/GameServer.exe db.port=16384 listen.port=21000

start ./debug/GatewayServer.exe listen.port=9600 chat.address=localhost chat.port=7400 asset.port=7300 print.packets=true login.port=7600 games=[localhost:21000:summon_war,192.168.1.1:21100:MFM]

exit

