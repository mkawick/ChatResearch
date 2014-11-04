#!/bin/bash

cd /home/build/perforce/Mber/ServerStack
p4 sync //build_mber/Mber/ServerStack/...

cd Analytics
make install
cd ..

cd AssetDelivery
make install
cd ..

cd Chat
make install
cd ..

cd Contacts
make install
cd ..

cd Gateway
make install
cd ..

cd LoadBalancer
make install
cd ..

cd Login
make install
cd ..

cd Notification
make install
cd ..

cd Purchase
make install
cd ..

cd UserStats
make install
cd ..

cd /home/build/perforce/SummonWar/main/server
p4 sync //build_mber/SummonWar/main/server/...

make install
cd /home/build/perforce/Mber/ServerStack
chmod 555 build_all.sh