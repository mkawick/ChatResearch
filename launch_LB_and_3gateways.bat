start ./debug/LoadBalancer.exe 

start ./debug/GatewayServer.exe server.name=Gateway1 balancer.port=9502 listen.port=9600 chat.address=localhost chat.port=7400 asset.port=7300 print.packets=true login.port=7600 games=[localhost:21000:summon_war,192.168.1.1:21100:MFM]

start ./debug/GatewayServer.exe server.name=Gateway2 balancer.port=9502 listen.port=9601 chat.address=localhost chat.port=7400 asset.port=7300 print.packets=true login.port=7600 games=[localhost:21000:summon_war,192.168.1.1:21100:MFM]

start ./debug/GatewayServer.exe server.name=Gateway3 balancer.port=9502 listen.port=9602 chat.address=localhost chat.port=7400 asset.port=7300 print.packets=true login.port=7600 games=[localhost:21000:summon_war,192.168.1.1:21100:MFM]

REM start ./debug/GatewayServer.exe server.name=Gateway1 listen.port=9600 chat.address=127.0.0.1 chat.port=7400 login.port=7600 login.address=localhost asset.address=localhost asset.port=7300

REM start ./debug/GatewayServer.exe server.name=Gateway2 listen.port=9601 chat.address=127.0.0.1 chat.port=7400 login.port=7600 login.address=localhost asset.address=localhost asset.port=7300

REM start ./debug/GatewayServer.exe server.name=Gateway3 listen.port=9602 chat.address=127.0.0.1 chat.port=7400 login.port=7600 login.address=localhost asset.address=localhost asset.port=7300
REM listen.port=9601 chat.address=localhost chat.port=7400 balancer.port=9502 login.address=localhost login.port=7600 asset.address=localhost asset.port=7300 games=[localhost:21000:summon_war,192.168.1.1:21100:MFM]

exit

REM listen.port=9601 chat.address=localhost chat.port=7400 balancer.port=9502 login.address=localhost login.port=7600 asset.address=localhost asset.port=7300 reroute.port=9600 reroute.address=10.16.160.10 games=[localhost:21000:summon_war,192.168.1.1:21100:MFM]