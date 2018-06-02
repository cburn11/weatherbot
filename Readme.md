# weatherbot

weatherbot is a linux c++ cgi/daemon pair of programs that interface through mattermost (https://about.mattermost.com) through slash commands and incoming webhooks.

The rest static library, against which weatherbot-daemon and cache-forecast link, uses Microsoft's C++ Rest SDK (https://microsoft.github.io/cpprestsdk).

# /weatherbot [location]
The weatherbot slash command send an "ephemeral" response to the user with the weather forecast for [location] provided by the National Weather Service.
The national weather service hosts a web service (https://www.weather.gov/documentation/services-web-api) that a webservice that provides, among other things, forecasts for given latitude/longitude coordinates.
Google's geocoding api conveniently converts a location string into latitude and longitude.

# Incoming webhook
The weatherbot incoming webhook is a pair of bash scripts suitable for cron jobs which download a forecast from the National Weather Service and then send that forecast to MatterMost.

# weatherbotd
weatherbotd should be run as/owned by root. It also expects a user/group named weatherbot that it the daemon demotes itself to once it has "forked". Prior to compiling, rest/rest.cpp @GetCoords should have your google api added to the query string portion of the uri. 

<img src="https://github.com/cburn11/weatherbot/raw/master/Screenshot_20180601-220531.png">
