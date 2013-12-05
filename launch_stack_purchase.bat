
start ./debug/LoginServer.exe listen.port=7600 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=21002 contact.port=7502 asset.port=7302 autoAddLoginProduct=false

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/PurchaseServer.exe listen.port=7700 s2s.port=7702 s2s.address=localhost db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek


start ./debug/GatewayServer.exe listen.port=9600 login.port=7600 purchase.port=7700 print.packets=true 
#reroute.port=9600 reroute.address=10.16.160.10

exit

