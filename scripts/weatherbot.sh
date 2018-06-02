#!/bin/bash

channel=$1

dir=/tmp

forecast_file="$dir/forecast"

hook_id=$2

forecast_str=$(cat $forecast_file | sed 's!\x0a!\\n!g')

payload_str="{\"channel\": \"$channel\",\"icon_url\": \"https://www.mattermost.org/wp-content/uploads/2016/04/icon.png\", \"username\": \"Weather Bot\", \"text\": \"$forecast_str\"}"

curl -i -X POST --data-urlencode "payload=$payload_str" "https://mattermost.danger-rocket.com:8066/hooks/$hook_id"

