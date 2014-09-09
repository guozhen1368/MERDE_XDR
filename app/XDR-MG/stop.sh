#!/bin/sh
# edit by guozhen

ps -ef | grep "mg_xdr"|grep -v grep|awk '{print $2}'|xargs -i kill -9 {}