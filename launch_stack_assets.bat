
start ./debug/LoginServer.exe listen.port=7600 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=21002 contact.port=7502 asset.port=7302 autoAddLoginProduct=false

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/AssetDeliveryServer.exe asset.path='C:/projects/Mber/ServerStack/X_testFiles_X' listen.port=7300 asset.dictionary="assets.ini" s2s.port=7302 


start ./debug/GatewayServer.exe listen.port=9600 login.port=7600 asset.port=7300

exit

