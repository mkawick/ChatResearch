#!/bin/bash

/usr/local/bin/chat_serverd db.address=10.16.4.44 db.port=3306 db.username=incinerator db.password=Cm8235 db.schema=playdek listen.address=localhost listen.port=7400 s2s.port=7402 &
