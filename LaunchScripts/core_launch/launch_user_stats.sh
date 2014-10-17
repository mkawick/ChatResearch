#!/bin/bash

/usr/local/bin/user_stats_serverd listen.port=12000 s2s.port=12002 db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek &
