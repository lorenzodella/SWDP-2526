import 'dart:async';
import 'dart:math';

import 'package:flutter/foundation.dart';
import 'package:swdp_bio_app/ble_controller.dart';

Timer? _mockTimer;
int _sampleIndex = 0;
final Random _random = Random();

/// Starts generating mock BLE data and injects it into [BleController].
///
/// Only works in debug mode. In release mode this is a no-op.
void startMockData() {
  // early exit if not in debug mode to avoid accidentally running this in production
  if (!kDebugMode) return;

  // PPG + Filtered PPG: sine wave (simulating ~1.2 Hz cardiac pulse) at ~100 Hz
  // HR: updates every 100 samples (~1s)
  // Temperature: updates every 200 samples (~2s)
  _mockTimer = Timer.periodic(const Duration(milliseconds: 10), (_) {
    const double frequency = 1.2; // ~72 BPM
    final double t = _sampleIndex * 0.01;
    final double raw = sin(2 * pi * frequency * t) * 500 + 2048;
    final double noise = (_random.nextDouble() - 0.5) * 80;

    BleController.instance.injectSyntheticData(
      ppg: raw + noise,
      filteredPpg: raw,
    );
    _sampleIndex++;

    // Heart Rate every ~100 samples (~1s)
    if (_sampleIndex % 100 == 0) {
      final int bpm = 65 + _random.nextInt(16); // 65–80 BPM
      BleController.instance.injectSyntheticData(heartRate: bpm);
    }

    // Temperature every ~3000 samples (~30s)
    if (_sampleIndex % 3000 == 0) {
      // Stable around 36.5°C with small drift ±0.15°C
      final double temp = 36.5 + (_random.nextDouble() - 0.5) * 0.3;
      BleController.instance.injectSyntheticData(temperature: temp);
    }

    // Keep sample index bounded to avoid overflow
    if (_sampleIndex > 100000) _sampleIndex = 0;
  });
}

/// Stops generating mock BLE data.
void stopMockData() {
  _mockTimer?.cancel();
  _mockTimer = null;
}
