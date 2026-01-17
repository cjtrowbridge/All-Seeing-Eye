#ifndef SPECTRUMPLUGIN_H
#define SPECTRUMPLUGIN_H

#include "ASEPlugin.h"
#include "HAL.h"
#include "Kernel.h"
#include "Logger.h"

class SpectrumPlugin : public ASEPlugin {
public:
    void setup() override {
        Logger::instance().info("Spectrum", "Setup: Sweeping...");
        _lastLoopMs = 0;
        _iterations = 0;
        resetSweepState();
    }
    
    void loop() override {
        _lastLoopMs = millis();
        CC1101* radio = HAL::instance().getRadio();
        if (!radio || !HAL::instance().hasRadio()) {
            vTaskDelay(pdMS_TO_TICKS(200));
            return;
        }
        if (_stepMhz <= 0.0f || _stopMhz <= _startMhz) {
            vTaskDelay(pdMS_TO_TICKS(200));
            return;
        }

        time_t epoch = Kernel::instance().getEpochTime();
        if (epoch <= 0) {
            vTaskDelay(pdMS_TO_TICKS(200));
            return;
        }

        if ((epoch % 10) != 0 || _lastSweepEpoch == static_cast<uint32_t>(epoch)) {
            vTaskDelay(pdMS_TO_TICKS(50));
            return;
        }

        _lastSweepEpoch = static_cast<uint32_t>(epoch);
        Logger::instance().info("Spectrum", "Synchronized sweep at UTC %lu", _lastSweepEpoch);

        for (float freq = _startMhz; freq <= _stopMhz; freq += _stepMhz) {
            int state = radio->setFrequency(freq);
            if (state != RADIOLIB_ERR_NONE) {
                if (millis() - _lastErrorLogMs > 5000) {
                    Logger::instance().error("Spectrum", "Set Freq Failed: %d", state);
                    _lastErrorLogMs = millis();
                }
                break;
            }

            float rssi = radio->getRSSI();
            storePoint(freq, rssi);
            vTaskDelay(pdMS_TO_TICKS(5));
        }

        _currentFreqMhz = _startMhz;
        _iterations++;
    }
    
    void teardown() override {
        Logger::instance().info("Spectrum", "Teardown");
    }

    String getName() override { return "Spectrum"; }
    String getTaskName() override { return _taskName; }

    void configure(String taskId, const JsonObject& params) override {
        _taskName = taskId;
        if (params.containsKey("start") && params.containsKey("stop")) {
            _startMhz = params["start"].as<float>();
            _stopMhz = params["stop"].as<float>();
        }
        if (params.containsKey("bandwidth")) {
            _bandwidthKHz = params["bandwidth"].as<float>();
        }
        if (params.containsKey("power")) {
            _powerDbm = params["power"].as<float>();
        }

        if (!HAL::instance().isCc1101FrequencyRangeAllowed(_startMhz, _stopMhz)) {
            Logger::instance().warn("Spectrum", "Invalid freq range %.2f-%.2f MHz. Using defaults.", _startMhz, _stopMhz);
            _startMhz = HAL::kCc1101DefaultStartMhz;
            _stopMhz = HAL::kCc1101DefaultStopMhz;
        }

        if (!HAL::instance().isCc1101BandwidthAllowed(_bandwidthKHz)) {
            Logger::instance().warn("Spectrum", "Invalid bandwidth %.2f kHz. Using default %.2f kHz.", _bandwidthKHz, HAL::kCc1101DefaultBandwidthKhz);
            _bandwidthKHz = HAL::kCc1101DefaultBandwidthKhz;
        }

        if (!HAL::instance().isCc1101PowerAllowed(_powerDbm)) {
            Logger::instance().warn("Spectrum", "Invalid power %.1f dBm. Using default %.1f dBm.", _powerDbm, HAL::kCc1101DefaultPowerDbm);
            _powerDbm = HAL::kCc1101DefaultPowerDbm;
        }

        if (_bandwidthKHz > 0.0f) {
            _stepMhz = _bandwidthKHz / 1000.0f;
        }
        Logger::instance().info("Spectrum", "Sweep %.2f-%.2f MHz", _startMhz, _stopMhz);
        Logger::instance().info("Spectrum", "Bandwidth %.2f kHz | Power %.1f dBm", _bandwidthKHz, _powerDbm);
        resetSweepState();
    }

    bool getJsonData(JsonObject report) override {
        report["task_id"] = _taskName;
        report["start_mhz"] = _startMhz;
        report["stop_mhz"] = _stopMhz;
        report["bandwidth_khz"] = _bandwidthKHz;
        report["power_dbm"] = _powerDbm;
        report["step_mhz"] = _stepMhz;
        report["points_count"] = _pointCount;
        report["points_max"] = kMaxPoints;
        JsonArray points = report.createNestedArray("points");
        for (uint16_t i = 0; i < _pointCount; ++i) {
            uint16_t idx = (_pointIndex + kMaxPoints - _pointCount + i) % kMaxPoints;
            JsonObject point = points.add<JsonObject>();
            point["freq_mhz"] = _freqMhz[idx];
            point["rssi_dbm"] = _rssiDbm[idx];
        }
        report["iterations"] = _iterations;
        report["last_loop_ms"] = _lastLoopMs;
        return true;
    }

private:
    static constexpr uint16_t kMaxPoints = 255;

    String _taskName = "Spectrum Analysis";
    float _startMhz = 0.0f;
    float _stopMhz = 0.0f;
    float _bandwidthKHz = 500.0f;
    float _powerDbm = -1.0f;
    float _stepMhz = 0.5f;
    float _currentFreqMhz = 0.0f;
    uint32_t _lastSweepEpoch = 0;
    unsigned long _lastErrorLogMs = 0;
    unsigned long _lastLoopMs = 0;
    unsigned long _iterations = 0;
    uint16_t _pointCount = 0;
    uint16_t _pointIndex = 0;
    float _freqMhz[kMaxPoints] = {0};
    float _rssiDbm[kMaxPoints] = {0};

    void resetSweepState() {
        _pointCount = 0;
        _pointIndex = 0;
        _currentFreqMhz = _startMhz;
        _lastErrorLogMs = 0;
        _iterations = 0;
    }

    void storePoint(float freqMhz, float rssiDbm) {
        _freqMhz[_pointIndex] = freqMhz;
        _rssiDbm[_pointIndex] = rssiDbm;
        _pointIndex = (_pointIndex + 1) % kMaxPoints;
        if (_pointCount < kMaxPoints) {
            _pointCount++;
        }
    }
};

#endif
