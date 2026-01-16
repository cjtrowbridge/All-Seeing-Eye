#include "Geolocation.h"
#include "Logger.h"

namespace {
const uint32_t kStaleThresholdMs = 5 * 60 * 1000;
}

GeolocationService& GeolocationService::instance() {
    static GeolocationService instance;
    return instance;
}

void GeolocationService::begin() {
    _lastUpdatedMs = millis();
    _state = GEO_STATE_INIT;
    _fixType = GEO_FIX_NONE;
    _confidence = 0.0f;
    _position.valid = false;
    _relative.valid = false;
    _sourceCount = 0;
    Logger::instance().info("Geolocation", "Core service initialized");
}

void GeolocationService::loop() {
    updateStaleness();
}

void GeolocationService::setState(GeolocationState state) {
    if (_state != state) {
        _state = state;
        Logger::instance().info("Geolocation", "State -> %s", stateToString(state));
    }
}

void GeolocationService::setFixType(GeolocationFixType fixType) {
    _fixType = fixType;
    _lastUpdatedMs = millis();
}

void GeolocationService::setConfidence(float confidence) {
    _confidence = confidence;
    _lastUpdatedMs = millis();
}

void GeolocationService::setMotion(const GeolocationMotion& motion) {
    _motion = motion;
    _lastUpdatedMs = millis();
}

void GeolocationService::setAbsolutePosition(const GeolocationPosition& position) {
    _position = position;
    _position.valid = true;
    _relative.valid = false;
    _fixType = GEO_FIX_ABSOLUTE;
    _lastUpdatedMs = millis();
}

void GeolocationService::setRelativePosition(const GeolocationRelative& relative) {
    _relative = relative;
    _relative.valid = true;
    _position.valid = false;
    _fixType = GEO_FIX_RELATIVE;
    _lastUpdatedMs = millis();
}

void GeolocationService::addSourceSummary(const GeolocationSourceSummary& summary) {
    if (_sourceCount < kMaxSources) {
        _sources[_sourceCount] = summary;
        _sourceCount++;
    } else {
        for (uint8_t i = 1; i < kMaxSources; ++i) {
            _sources[i - 1] = _sources[i];
        }
        _sources[kMaxSources - 1] = summary;
    }
    _lastUpdatedMs = millis();
}

void GeolocationService::clearSources() {
    _sourceCount = 0;
}

void GeolocationService::populateStatus(JsonObject& geoObj) {
    geoObj["state"] = stateToString(_state);
    geoObj["fix"] = fixToString(_fixType);
    geoObj["confidence"] = _confidence;
    geoObj["last_updated"] = _lastUpdatedMs;

    JsonObject motion = geoObj.createNestedObject("motion");
    motion["stationary"] = _motion.stationary;
    motion["speed_mps"] = _motion.speedMps;
    motion["heading_deg"] = _motion.headingDeg;
    motion["variance"] = _motion.variance;

    if (_position.valid) {
        JsonObject pos = geoObj.createNestedObject("position");
        pos["lat"] = _position.lat;
        pos["lon"] = _position.lon;
        pos["alt"] = _position.alt;
        pos["accuracy_m"] = _position.accuracyM;
    } else {
        geoObj["position"] = nullptr;
    }

    if (_relative.valid) {
        JsonObject rel = geoObj.createNestedObject("relative");
        rel["frame_id"] = _relative.frameId;
        rel["x"] = _relative.x;
        rel["y"] = _relative.y;
        rel["z"] = _relative.z;
        rel["accuracy_m"] = _relative.accuracyM;
    } else {
        geoObj["relative"] = nullptr;
    }

    JsonArray sources = geoObj.createNestedArray("sources");
    for (uint8_t i = 0; i < _sourceCount; ++i) {
        JsonObject src = sources.createNestedObject();
        src["type"] = _sources[i].type;
        src["id"] = _sources[i].id;
        src["quality"] = _sources[i].quality;
        src["age_ms"] = _sources[i].ageMs;
    }
}

void GeolocationService::updateStaleness() {
    if (_lastUpdatedMs == 0) {
        return;
    }
    uint32_t age = millis() - _lastUpdatedMs;
    if (age > kStaleThresholdMs && _state != GEO_STATE_STALE) {
        _state = GEO_STATE_STALE;
        Logger::instance().warn("Geolocation", "State -> stale (age %lu ms)", age);
    }
}

const char* GeolocationService::stateToString(GeolocationState state) const {
    switch (state) {
        case GEO_STATE_INIT: return "init";
        case GEO_STATE_CONVERGING: return "converging";
        case GEO_STATE_LOCKED: return "locked";
        case GEO_STATE_STALE: return "stale";
        default: return "init";
    }
}

const char* GeolocationService::fixToString(GeolocationFixType fixType) const {
    switch (fixType) {
        case GEO_FIX_NONE: return "none";
        case GEO_FIX_RELATIVE: return "relative";
        case GEO_FIX_ABSOLUTE: return "absolute";
        default: return "none";
    }
}
