#ifndef __REST_DEFS_
#define __REST_DEFS_


void GetCoords(std::string loc_str, std::string& loc_address, double& latitude, double& longitude);

std::string GetForecastUrl(double lat, double lng);

std::string GetForecast(const std::string& forecast_url);

void SendResponse(const std::string& url, const std::string& text);

#endif // __REST_DEFS_
