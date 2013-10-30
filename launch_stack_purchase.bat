
start ./debug/LoginServer.exe listen.address=127.0.0.1 listen.port=3072 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=24604 contact.port=9802 asset.port=9702 autoAddLoginProduct=false

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/PurchaseServer.exe listen.port=9900 listen.address=localhost s2s.port=9902 s2s.address=localhost db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek


start ./debug/GatewayServer.exe listen.port=9600 chat.address=127.0.0.1 chat.port=9601 login.port=3072 login.address=localhost asset.address=localhost asset.port=9700 purchase.port=9900 purchase.address=localhost

exit

