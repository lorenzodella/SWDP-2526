import 'dart:async';

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:swdp_bio_app/ble_controller.dart';
import 'package:swdp_bio_app/ppg_data_point.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  StreamSubscription<int>? _batteryLevelSubscription;
  StreamSubscription<double>? _sensedDataSubscription;

  final double _maxPoints = 1000;
  int _samplesCounter = 0;
  final List<PpgDataPoint> _chartDataWindow = [];

  @override
  void initState() {
    _batteryLevelSubscription = BleController.instance.batteryLevelStream.listen(_onNewBatteryLevel);
    _sensedDataSubscription = BleController.instance.sensedDataStream.listen(_onNewSensedData);
    super.initState();
  }

  @override
  void dispose() {
    _batteryLevelSubscription?.cancel();
    _sensedDataSubscription?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final String deviceName = BleController.instance.connectedDevice?.name ?? "Unknown Device";

    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text("SWDP BIO A6 APP"),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: .center,
          children: [
            Text('Connected to $deviceName'),
            Expanded(
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: LineChart(
                  LineChartData(
                    lineBarsData: [
                      LineChartBarData(
                        spots: _chartDataWindow.map((e) => FlSpot(e.index, e.value)).toList(),
                        isCurved: false,
                        dotData: const FlDotData(show: false),
                        color: Theme.of(context).colorScheme.primary,
                        barWidth: 2,
                      ),
                    ],
                    titlesData: const FlTitlesData(
                      show: true,
                      topTitles: AxisTitles(sideTitles: SideTitles(showTitles: false)),
                      rightTitles: AxisTitles(sideTitles: SideTitles(showTitles: false)),
                      bottomTitles: AxisTitles(sideTitles: SideTitles(showTitles: false)),
                      leftTitles: AxisTitles(
                        sideTitles: SideTitles(
                          showTitles: true,
                          reservedSize: 40,
                        ),
                      ),
                    ),
                    borderData: FlBorderData(show: true),
                    gridData: const FlGridData(show: true),
                  ),
                  duration: Duration.zero,
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }

  void _onNewBatteryLevel(int newLevel) {
    //TODO: replace with proper implementation
    debugPrint("New battery level: $newLevel");
  }

  void _onNewSensedData(double newData) {
    //TODO: replace with proper implementation

    // maintain a sliding window of the last _maxPoints data points for the chart
    if (_chartDataWindow.length >= _maxPoints) {
      _chartDataWindow.removeAt(0);
    }

    // add the new data point to the chart data window
    setState(() {
      _chartDataWindow.add(PpgDataPoint(_samplesCounter.toDouble(), newData));
      _samplesCounter++;
    });
  }
}
