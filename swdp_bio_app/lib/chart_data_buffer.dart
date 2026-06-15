import 'package:fl_chart/fl_chart.dart';

/// Auxiliary class holding the state of a FL Chart.
class ChartDataBuffer {
  ChartDataBuffer({this.maxPoints = 1000});

  final int maxPoints;
  final List<FlSpot> spots = [];
  int _counter = 0;

  void add(double value) {
    if (spots.length >= maxPoints) {
      spots.removeAt(0);
    }
    spots.add(FlSpot(_counter.toDouble(), value));
    _counter++;
  }
}
