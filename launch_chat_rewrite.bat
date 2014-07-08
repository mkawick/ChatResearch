

REM Pushd C:/projects/MberSW/main
REM start C:/projects/MberSW/main/WorkArea/game_serverD.exe listen.port=21000 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=21002 chat.port=8402
REM Popd

start ./debug/LoginServer.exe listen.port=7600 chat.port=8402 autoAddLoginProduct=false db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek games=[localhost:21000:summon_war]

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/ContactsServer.exe listen.port=7500 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=7502

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/ChatServer.exe listen.port=8400 s2s.port=8402 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek

ping -n 1 -w 1000 127.0.0.1 > nul
	
start ./debug/AnalyticsServer.exe listen.port=7800 s2s.port=7802 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 

REM ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/GatewayServer.exe listen.port=9600 chat.port=8400 login.port=7600 games=[localhost:21000:summon_war]

exit