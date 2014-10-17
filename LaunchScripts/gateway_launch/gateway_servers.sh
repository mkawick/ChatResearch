#!/bin/bash

## /usr/local/bin/load_balancer_serverd listen.port=9500 s2s.port=9502 &
## sleep 2

/usr/local/bin/gateway_serverd listen.port=9600 server.name="gateway01" asset.block=true print.functions=false print.packets=false balancer.address=localhost balancer.port=9502 chat.address=10.16.50.66 chat.port=7400 contact.address=10.16.50.66 contact.port=7500 login.address=10.16.50.66 login.port=7600 purchase.address=10.16.50.66 login.port=7700 notification.address=10.16.50.66 notification.port=7900 userstats.address=10.16.50.66 userstats.port=12000 analytics.address=10.16.50.66 analytics.port=7802 games=[10.16.50.62:21000:summon_war,10.16.50.62:21100:MFM] &

/usr/local/bin/gateway_serverd listen.port=9601 asset.address=10.16.50.66 asset.port=7300 server.name="Asset-Only-Gateway" asset.only=true balancer.port=9502 external.ip.address=70.186.140.93 &

## /////////////////////////////////////////
