#!/bin/bash

date_stmp=$(date +%y%m%d)

dir=/tmp

raw_forecast_file="$dir/raw_forecast_$date_stmp"
processed_forecast_file="$dir/processed_forecast_$date_stmp"

forecast_file="$dir/forecast"

curl -s "https://api.weather.gov/gridpoints/VEF/120,93/forecast" > $raw_forecast_file

jq -r '.properties | .periods | .[:4] | .[] | (.name, .detailedForecast)' $raw_forecast_file | sed 'N;s!\n!;!' > $processed_forecast_file

echo "#### Las Vegas, Nevada" > $forecast_file

awk 'BEGIN{ FS = ";" } { printf("#### %s:\n %s\n\n", $1, $2)}' $processed_forecast_file >> $forecast_file


