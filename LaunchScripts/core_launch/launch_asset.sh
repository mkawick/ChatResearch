#!/bin/bash

/usr/local/bin/asset_serverd listen.port=7300 s2s.port=7302 asset.path='/usr/local/share/asset' asset.dictionary="assets_of_assets.ini" &
