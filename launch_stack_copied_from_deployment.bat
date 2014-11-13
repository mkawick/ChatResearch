

REM start C:\projects\Mber\ServerStack\Debug/GatewayServer.exe server.name="Gateway2" listen.port=9600 analytics.address=10.16.50.68 analytics.port=7802 balancer.address=10.16.50.68 balancer.port=9502 chat.address=10.16.50.68 chat.port=7400 contact.address=10.16.50.68 contact.port=7500 login.address=10.16.50.68 login.port=7600 notification.address=10.16.50.68 notification.port=12000 purchase.address=10.16.50.68 purchase.port=7700 userstats.address=10.16.50.68 userstats.port=12000 asset.block=true print.packets=false games=[10.16.50.68:21000:summon_war,192.168.1.1:21100:MFM] 
pushd C:\projects\SummonWar\main
REM start C:/mickey/MberSW/main/WorkArea/game_serverD.exe listen.port=21000 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek_game_summonwar s2s.port=21002  chat.port=7402
REM start C:/mickey/MberSW/main/WorkArea/game_server.exe listen.port=21000 s2s.port=21002 chat.port=7402 userstats.port=12000 dblist=[user:10.16.4.44:3306:incinerator:Cm8235:playdek,game:10.16.4.44:3306:incinerator:Cm8235:playdek_game_summonwar]
start C:\projects\SummonWar\main\WorkArea/game_server.exe listen.port=21000 s2s.port=21002 chat.port=7402 userstats.port=12002 purchase.port=7702 dblist=[user:10.16.4.44:3306:incinerator:Cm8235:playdek,game:10.16.4.44:3306:incinerator:Cm8235:playdek_game_summonwar]
popd

start C:\projects\Mber\ServerStack\Debug/LoadBalancer.exe  listen.port=9500 s2s.port=9502

start C:\projects\Mber\ServerStack\Debug/LoginServer.exe db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek listen.address=localhost listen.port=7600 contact.address=localhost chat.port=7402 contact.port=7502 asset.port=7302 userstats.port=12002 autoAddLoginProduct=false games=[localhost:21000:summon_war,192.168.1.1:21100:MFM] print.functions=false
REM db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek listen.address=localhost listen.port=7600 contact.address=localhost chat.port=7402 contact.port=7502 asset.port=7302 userstats.port=12000 autoAddLoginProduct=true games=[localhost:21000:summon_war,192.168.1.1:21100:MFM] print.functions=false

ping -n 1 -w 1000 127.0.0.1 > nul

REM start ChatServer.exe listen.port=7400 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=21002
start C:\projects\Mber\ServerStack\Debug/ChatServer.exe db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek listen.address=localhost listen.port=7400 s2s.port=7402

ping -n 1 -w 1000 127.0.0.1 > nul

start C:\projects\Mber\ServerStack\Debug/ContactsServer.exe listen.port=7500 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=7502

ping -n 1 -w 1000 127.0.0.1 > nul

start C:\projects\Mber\ServerStack\Debug/AssetDeliveryServer.exe listen.port=7300 s2s.port=7302 asset.path='c:\gwshare' asset.dictionary="assets_of_assets.ini"
REM   AssetDeliveryServer.exe listen.port=7300 s2s.port=7302 asset.path='c:/gwshare' asset.dictionary="assets_of_assets.ini"
ping -n 1 -w 1000 127.0.0.1 > nul

start C:\projects\Mber\ServerStack\Debug/PurchaseServer.exe listen.port=7700 s2s.port=7702 s2s.address=localhost db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek

ping -n 1 -w 1000 127.0.0.1 > nul
	
start C:\projects\Mber\ServerStack\Debug/NotificationServer.exe listen.port=7900 s2s.port=7902 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek ios.certpath='C:/mickey/Notification/certificates'

ping -n 1 -w 1000 127.0.0.1 > nul
	
start C:\projects\Mber\ServerStack\Debug/AnalyticsServer.exe listen.port=7800 s2s.port=7802 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek
start C:\projects\Mber\ServerStack\Debug/UserStatsServer.exe listen.port=12000 s2s.port=12002 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek

ping -n 1 -w 14000 127.0.0.1 > nul
REM 14 second delay

REM start GatewayServer.exe listen.port=9600 chat.address=localhost chat.port=7400 login.port=7600 server.name="MainGateway" asset.block=true print.functions=false print.packets=true balancer.port=9502 external.ip.address=70.186.140.93
REM start GatewayServer.exe listen.port=9600 chat.address=localhost chat.port=7400 login.port=7600 server.name="Main Gateway" asset.block=true balancer.port=9502 chat.port=7400 print.packets=false userstats.port=12000 games=[localhost:21000:summon_war,192.168.1.1:21100:MFM] external.ip.address=70.186.140.93
start C:\projects\Mber\ServerStack\Debug/GatewayServer.exe listen.port=9600 chat.address=localhost chat.port=7400 login.port=7600 server.name="Main Gateway" asset.block=true balancer.port=9502 userstats.port=12000 print.packets=false games=[localhost:21000:summon_war,192.168.1.1:21100:MFM] external.ip.address=mber.pub.playdekgames.com
REM print.packets=true 

REM start GatewayServer.exe listen.port=9601 asset.port=7300 server.name="Asset-Only-Gateway" asset.only=true balancer.port=9502 external.ip.address="70.186.140.93"
REM start AssetGatewayServer.exe listen.port=9601 asset.port=7300 server.name="Asset-Only-Gateway" asset.only=true balancer.port=9502 external.ip.address=70.186.140.93

start C:\projects\Mber\ServerStack\Debug/GatewayServer.exe listen.port=9601 asset.port=7300 server.name="Asset-Only-Gateway" asset.only=true balancer.port=9502 external.ip.address=mber.pub.playdekgames.com

exit

