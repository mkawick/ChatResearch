
start ./debug/LoginServer.exe listen.address=127.0.0.1 listen.port=3072 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek game.port=24604 contact.port=9802 asset.port=9702

ping -n 1 -w 1000 127.0.0.1 > nul

start ./debug/AssetDeliveryServer.exe s2s.port=9702 asset.path='C:/projects/Mber/ServerStack/X_testFiles_X' listen.port=9700 listen.address=localhost asset.dictionary="assets.ini"


start ./debug/GatewayServer.exe listen.address=127.0.0.1 listen.port=9600 chat.address=127.0.0.1 chat.port=9601

exit

