#ifndef __HTTP_DEFS_
#define __HTTP_DEFS_

#include <string>

void GetCoords(std::string loc_str, std::string& loc_address, double& latitude, double& longitude);

std::string FormatCoordinate(double coord);

//std::string GetForecastUrl(double lat, double lng);
std::string GetForecastUrl(const std::string& lat, const std::string& lng);

std::string GetForecast(const std::string& forecast_url,
			std::string * pStrLatitude = nullptr, std::string * pStrLongitude = nullptr);

bool GetForecastCurrentTempAndIcon(const std::string& forecast_url,
				   std::string& temp, std::string& icon_url);

std::string GetHumidity(const std::string& lat, const std::string& lng);

std::string BuildJSONAttachment(const std::string& header, const std::string& forecast,
				const std::string& icon);

std::string DecodePercentEncodedString(const std::string& strEncoded);

void SendJSON(const std::string& url, const std::string& json, 
	      const std::string& channel = std::string{""});

void SendResponse(const std::string& url, const std::string& text);

void SendAttachment(const std::string& url, const std::string& header,
		    const std::string& forecast, const std::string& icon);

std::string BuildNeedAttachment(const std::string& text);
#endif // __HTTP_DEFS_
