import 'dart:async';

import 'package:fl_chart/fl_chart.dart';
import 'package:flutter/material.dart';
import 'package:swdp_bio_app/ble_controller.dart';
import 'package:swdp_bio_app/chart_data_buffer.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  // Subscriptions to incoming data streams
  StreamSubscription<double>? _ppgDataSubscription;
  StreamSubscription<double>? _filteredPpgDataSubscription;
  StreamSubscription<int>? _heartRateSubscription;
  StreamSubscription<double>? _temperatureSubscription;

  // Temperature state
  double? _temperature;
  DateTime? _lastTemperatureUpdate;
  static const Duration _temperatureTimeout = Duration(minutes: 1);

  // Heart rate state
  int? _heartRate;
  DateTime? _lastHeartRateUpdate;
  static const Duration _heartRateTimeout = Duration(minutes: 1);

  // Chart selection
  int _selectedChartIndex = 0;

  // Charts' data buffers
  final ChartDataBuffer _ppgChartDataBuffer = ChartDataBuffer();
  final ChartDataBuffer _filteredPpgChartDataBuffer = ChartDataBuffer();

  @override
  void initState() {
    _ppgDataSubscription = BleController.instance.ppgDataStream.listen(
      (data) => setState(() {
        _ppgChartDataBuffer.add(data);
      }),
    );

    _filteredPpgDataSubscription = BleController.instance.filteredPpgDataStream
        .listen(
          (data) => setState(() {
            _filteredPpgChartDataBuffer.add(data);
          }),
        );

    _heartRateSubscription = BleController.instance.heartRateStream.listen((
      data,
    ) {
      setState(() {
        _heartRate = data;
        _lastHeartRateUpdate = DateTime.now();
      });
    });

    _temperatureSubscription = BleController.instance.temperatureStream.listen((
      data,
    ) {
      setState(() {
        _temperature = data;
        _lastTemperatureUpdate = DateTime.now();
      });
    });

    super.initState();
  }

  @override
  void dispose() {
    _ppgDataSubscription?.cancel();
    _filteredPpgDataSubscription?.cancel();
    _heartRateSubscription?.cancel();
    _temperatureSubscription?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final String deviceName =
        BleController.instance.connectedDevice?.name ?? "Unknown Device";

    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text("SWDP BIO A6 APP"),
      ),
      body: SafeArea(
        child: Column(
          children: [
            // Title
            Padding(
              padding: const EdgeInsets.symmetric(vertical: 8.0),
              child: Text(
                "Connected to $deviceName",
                style: Theme.of(
                  context,
                ).textTheme.titleLarge?.copyWith(fontWeight: FontWeight.bold),
              ),
            ),

            // Temperature card
            _buildDataCard(
              icon: Icons.thermostat,
              valueText: _temperature != null
                  ? "${_temperature!.toStringAsFixed(1)}°C"
                  : "--- °C",
              hasData: _temperature != null,
              lastUpdate: _lastTemperatureUpdate,
              timeout: _temperatureTimeout,
            ),

            // Heart Rate card
            _buildDataCard(
              icon: Icons.favorite,
              valueText: _heartRate != null ? "$_heartRate BPM" : "--- BPM",
              hasData: _heartRate != null,
              lastUpdate: _lastHeartRateUpdate,
              timeout: _heartRateTimeout,
            ),

            // Chart selector chips
            Padding(
              padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
              child: _buildChartSelector(),
            ),

            // Chart
            Expanded(
              child: Padding(
                padding: const EdgeInsets.all(8.0),
                child: _buildChart(),
              ),
            ),
          ],
        ),
      ),
    );
  }

  /// Returns a generic sensor data card with an [icon] displayed on the left,
  /// [valueText] as main content and a status text on the right indicating the freshness
  /// of the data, calculated using [lastUpdate] and [timeout].
  /// [hasData] indicates whether sensor data is available.
  Widget _buildDataCard({
    required IconData icon,
    required String valueText,
    required bool hasData,
    required DateTime? lastUpdate,
    required Duration timeout,
  }) {
    final String statusText;
    final Color bgColor;

    if (lastUpdate == null || !hasData) {
      statusText = "N/A";
      bgColor = Colors.grey.shade300;
    } else {
      final age = DateTime.now().difference(lastUpdate);
      final isStale = age > timeout;

      statusText = "${age.inSeconds}s ago";
      bgColor = isStale ? Colors.red.shade100 : Colors.grey.shade300;
    }

    return Container(
      width: double.infinity,
      margin: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
      decoration: BoxDecoration(
        color: bgColor,
        borderRadius: BorderRadius.circular(12),
      ),
      child: Row(
        children: [
          Icon(icon, size: 28),
          const SizedBox(width: 8),
          Expanded(
            child: Text(
              valueText,
              style: Theme.of(
                context,
              ).textTheme.titleMedium?.copyWith(fontWeight: FontWeight.w600),
            ),
          ),
          Text(
            statusText,
            style: Theme.of(
              context,
            ).textTheme.bodySmall?.copyWith(color: Colors.grey.shade700),
          ),
        ],
      ),
    );
  }

  /// Returns the chip selector widget for switching between different charts
  Widget _buildChartSelector() {
    const labels = ["PPG", "Filtered PPG"];
    final colors = [Colors.blue, Colors.green];

    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: List.generate(labels.length, (index) {
        final isSelected = _selectedChartIndex == index;
        return Padding(
          padding: const EdgeInsets.symmetric(horizontal: 4),
          child: ChoiceChip(
            label: Text(labels[index]),
            selected: isSelected,
            selectedColor: colors[index].withValues(alpha: 0.3),
            onSelected: (selected) {
              if (selected) {
                setState(() => _selectedChartIndex = index);
              }
            },
          ),
        );
      }),
    );
  }

  /// Returns the chart widget based on the currently selected chart index.
  Widget _buildChart() {
    final ChartDataBuffer buffer;
    final Color color;

    switch (_selectedChartIndex) {
      case 1:
        buffer = _filteredPpgChartDataBuffer;
        color = Colors.green;
        break;

      case 0:
      default:
        buffer = _ppgChartDataBuffer;
        color = Colors.blue;
    }

    return LineChart(
      LineChartData(
        lineBarsData: [
          LineChartBarData(
            spots: buffer.spots,
            isCurved: false,
            dotData: const FlDotData(show: false),
            color: color,
            barWidth: 2,
          ),
        ],
        titlesData: const FlTitlesData(
          show: true,
          topTitles: AxisTitles(sideTitles: SideTitles(showTitles: false)),
          rightTitles: AxisTitles(sideTitles: SideTitles(showTitles: false)),
          bottomTitles: AxisTitles(sideTitles: SideTitles(showTitles: false)),
          leftTitles: AxisTitles(
            sideTitles: SideTitles(showTitles: true, reservedSize: 40),
          ),
        ),
        borderData: FlBorderData(show: true),
        gridData: const FlGridData(show: true),
      ),
      duration: Duration.zero,
    );
  }
}
