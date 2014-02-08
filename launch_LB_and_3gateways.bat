start ./debug/LoadBalancer.exe 

start ./debug/GatewayServer.exe server.name=Gateway1 listen.port=9600 chat.address=127.0.0.1 chat.port=7400 login.port=7600 login.address=localhost asset.address=localhost asset.port=7300

start ./debug/GatewayServer.exe server.name=Gateway2 listen.port=9601 chat.address=127.0.0.1 chat.port=7400 login.port=7600 login.address=localhost asset.address=localhost asset.port=7300

start ./debug/GatewayServer.exe server.name=Gateway3 listen.port=9602 chat.address=127.0.0.1 chat.port=7400 login.port=7600 login.address=localhost asset.address=localhost asset.port=7300

exit