#ifndef GEOLOCATION_H
#define GEOLOCATION_H

#include <Arduino.h>
#include <ArduinoJson.h>

enum GeolocationState {
    GEO_STATE_INIT = 0,
    GEO_STATE_CONVERGING,
    GEO_STATE_LOCKED,
    GEO_STATE_STALE
};

enum GeolocationFixType {
    GEO_FIX_NONE = 0,
    GEO_FIX_RELATIVE,
    GEO_FIX_ABSOLUTE
};

struct GeolocationMotion {
    bool stationary = false;
    float speedMps = 0.0f;
    float headingDeg = 0.0f;
    float variance = 0.0f;
};

struct GeolocationPosition {
    double lat = 0.0;
    double lon = 0.0;
    double alt = 0.0;
    float accuracyM = 0.0f;
    bool valid = false;
};

struct GeolocationRelative {
    String frameId;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float accuracyM = 0.0f;
    bool valid = false;
};

struct GeolocationSourceSummary {
    String type;
    String id;
    float quality = 0.0f;
    uint32_t ageMs = 0;
};

class GeolocationService {
public:
    static GeolocationService& instance();

    void begin();
    void loop();

    void setState(GeolocationState state);
    void setFixType(GeolocationFixType fixType);
    void setConfidence(float confidence);
    void setMotion(const GeolocationMotion& motion);
    void setAbsolutePosition(const GeolocationPosition& position);
    void setRelativePosition(const GeolocationRelative& relative);

    void addSourceSummary(const GeolocationSourceSummary& summary);
    void clearSources();

    void populateStatus(JsonObject& geoObj);

private:
    GeolocationService() = default;

    void updateStaleness();
    const char* stateToString(GeolocationState state) const;
    const char* fixToString(GeolocationFixType fixType) const;

    GeolocationState _state = GEO_STATE_INIT;
    GeolocationFixType _fixType = GEO_FIX_NONE;
    GeolocationMotion _motion;
    GeolocationPosition _position;
    GeolocationRelative _relative;
    float _confidence = 0.0f;
    uint32_t _lastUpdatedMs = 0;

    static const uint8_t kMaxSources = 8;
    GeolocationSourceSummary _sources[kMaxSources];
    uint8_t _sourceCount = 0;
};

#endif
