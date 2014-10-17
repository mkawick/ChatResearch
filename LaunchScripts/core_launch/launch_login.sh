#!/bin/bash

/usr/local/bin/login_serverd db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek listen.address=localhost listen.port=7600 chat.port=7402 contact.port=7502 asset.port=7302 userstats.port=12002 autoAddLoginProduct=false games=[10.16.50.62:21000:summon_war,192.168.1.1:21100:MFM] print.functions=false &
