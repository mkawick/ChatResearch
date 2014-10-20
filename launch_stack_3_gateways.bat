pushd C:\projects\SummonWar\main\
start C:\projects\SummonWar\main\WorkArea/game_serverD.exe listen.port=21000 s2s.port=21002 chat.port=7402 purchase.port=7702 userstats.port=12000 dblist=[user:10.16.4.44:3306:incinerator:Cm8235:playdek,game:10.16.4.44:3306:incinerator:Cm8235:playdek_game_summonwar_test]
popd

start C:\projects\Mber\ServerStack\Debug/LoadBalancer.exe  listen.port=9500 s2s.port=9502

start C:\projects\Mber\ServerStack\Debug/LoginServer.exe db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek listen.address=localhost listen.port=7600 contact.address=localhost chat.port=7402 contact.port=7502 asset.port=7302 userstats.port=12000 autoAddLoginProduct=false games=[localhost:21000:summon_war,192.168.1.1:21100:MFM] print.functions=false

ping -n 1 -w 1000 127.0.0.1 > nul

start C:\projects\Mber\ServerStack\Debug/ChatServer.exe db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek listen.address=localhost listen.port=7400 s2s.port=7402

ping -n 1 -w 1000 127.0.0.1 > nul

start C:\projects\Mber\ServerStack\Debug/ContactsServer.exe listen.port=7500 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=7502

ping -n 1 -w 1000 127.0.0.1 > nul

start C:\projects\Mber\ServerStack\Debug/PurchaseServer.exe listen.port=7700 s2s.port=7702 s2s.address=localhost db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek

ping -n 1 -w 1000 127.0.0.1 > nul
	
start C:\projects\Mber\ServerStack\Debug/NotificationServer.exe listen.port=7900 s2s.port=7902 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek ios.certpath="../../../../SummonWar/main/data/ios/certificates"
start C:\projects\Mber\ServerStack\Debug/AssetDeliveryServer.exe listen.port=7300 s2s.port=7302 game.port=21002 asset.path2='C:/projects/Mber/ServerStack/X_testFiles_X' asset.dictionary2="assets_of_assets.ini" asset.path='c:/gwshare' asset.dictionary="assets_of_assets.ini"

ping -n 1 -w 1000 127.0.0.1 > nul

start C:\projects\Mber\ServerStack\Debug/UserStatsServer.exe listen.port=12000 s2s.port=12002   db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek

ping -n 1 -w 14000 127.0.0.1 > nul

start C:\projects\Mber\ServerStack\Debug/GatewayServer.exe listen.port=9600 chat.address=localhost chat.port=7400 login.port=7600 server.name="Gateway1" asset.block=true balancer.port=9502 chat.port=7400 print.packets=false userstats.port=12000 games=[localhost:21000:summon_war,192.168.1.1:21100:MFM] 
start C:\projects\Mber\ServerStack\Debug/GatewayServer.exe listen.port=9610 chat.address=localhost chat.port=7400 login.port=7600 server.name="Gateway2" asset.block=true balancer.port=9502 chat.port=7400 print.packets=false userstats.port=12000 games=[localhost:21000:summon_war,192.168.1.1:21100:MFM] 
REM start C:\projects\Mber\ServerStack\Debug/GatewayServer.exe listen.port=9620 chat.address=localhost chat.port=7400 login.port=7600 server.name="Gateway3" asset.block=true balancer.port=9502 chat.port=7400 print.packets=false games=[localhost:21000:summon_war,192.168.1.1:21100:MFM] 

ping -n 1 -w 1000 127.0.0.1 > nul

start C:\projects\Mber\ServerStack\Debug/GatewayServer.exe listen.port=9620 asset.port=7300 server.name="Asset-Only-Gateway" asset.only=true balancer.port=9502 external.ip.address=70.186.140.93

exit