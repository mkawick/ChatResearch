#!/bin/bash

/usr/local/bin/login_serverd db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek listen.address=localhost listen.port=7600 chat.port=7402 contact.port=7502 asset.port=7302 userstats.port=12002 autoAddLoginProduct=false games=[10.16.50.62:21000:summon_war,192.168.1.1:21100:MFM] print.functions=false &
sleep 1

/usr/local/bin/chat_serverd db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek listen.address=localhost listen.port=7400 s2s.port=7402 &

/usr/local/bin/contacts_serverd listen.port=7500 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek s2s.port=7502 &

/usr/local/bin/asset_serverd listen.port=7300 s2s.port=7302 asset.path='/usr/local/share/asset' asset.dictionary="assets_of_assets.ini" &

sleep 3

/usr/local/bin/purchase_serverd listen.port=7700 s2s.port=7702 s2s.address=localhost db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek &

/usr/local/bin/notification_serverd listen.port=7900 s2s.port=7902 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek ios.certpath='/usr/local/share/ios/certificates'

## /usr/local/bin/AnalyticsServer.exe listen.port=7800 s2s.port=7802 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek &

/usr/local/bin/user_stats_serverd listen.port=12000 s2s.port=12002 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek &

