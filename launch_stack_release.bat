REM cd C:/projects/MberSW/main
REM start C:/projects/MberSW/main/WorkArea/game_serverD.exe listen.port=21000 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=21002
REM cd C:/projects/Mber/ServerStack

REM Pushd C:/projects/MberSW/main
REM start C:/projects/MberSW/main/WorkArea/game_serverD.exe listen.port=21000 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=21002 chat.port=7402
REM Popd


start ./release/LoadBalancer.exe  listen.port=9500 s2s.port=9502

start ./release/LoginServer.exe db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek listen.address=localhost listen.port=7600 contact.address=localhost chat.address=localhost contact.port=7502 asset.port=7302 autoAddLoginProduct=false games=[localhost:21000:summon_war,192.168.1.1:21100:MFM]

ping -n 1 -w 1000 127.0.0.1 > nul

start ./release/ChatServer.exe listen.port=7400 s2s.port=7402 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=21002

ping -n 1 -w 1000 127.0.0.1 > nul

start ./release/ContactsServer.exe listen.port=7500 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=7502

ping -n 1 -w 1000 127.0.0.1 > nul

start ./release/AssetDeliveryServer.exe listen.port=7300 s2s.port=7302 game.port=21002 asset.path='C:/projects/Mber/ServerStack/X_testFiles_X' asset.dictionary="assets_of_assets.ini"

ping -n 1 -w 1000 127.0.0.1 > nul

start ./release/PurchaseServer.exe listen.port=7700 s2s.port=7702 s2s.address=localhost db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek

ping -n 1 -w 1000 127.0.0.1 > nul
	
start ./release/StatServer.exe listen.port=7800 s2s.port=7802 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 


REM start ./release/GameServer.exe db.port=16384 listen.port=21000

start ./release/GatewayServer.exe listen.port=9600 chat.address=localhost chat.port=7400 asset.port=7300 print.packets=true login.port=7600 games=[localhost:21000:summon_war,192.168.1.1:21100:MFM]

exit

