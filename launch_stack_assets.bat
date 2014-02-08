
start ./debug/LoginServer.exe db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek listen.address=localhost listen.port=7600 contact.address=localhost chat.address=localhost contact.port=7502 asset.port=7302 autoAddLoginProduct=false games=[localhost:21000:summon_war,192.168.1.1:21100:MFM]

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/AssetDeliveryServer.exe listen.port=7300 s2s.port=7302 game.port=21002 asset.path='C:/projects/Mber/ServerStack/X_testFiles_X' asset.dictionary="assets_of_assets.ini"

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/GatewayServer.exe listen.port=9600 chat.address=localhost chat.port=7400 asset.port=7300 print.packets=true login.port=7600 

exit

