#!/bin/bash

/usr/local/bin/asset_gateway_serverd listen.port=9601 asset.address=10.16.50.66 asset.port=7300 server.name="Asset-Only-Gateway" asset.only=true balancer.port=9502 external.ip.address=70.186.140.93 &