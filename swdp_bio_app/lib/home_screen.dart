import 'dart:async';

import 'package:flutter/material.dart';
import 'package:swdp_bio_app/ble_controller.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  StreamSubscription<int>? _batteryLevelSubscription;
  StreamSubscription<List<int>>? _sensedDataSubscription;

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
          ],
        ),
      ),
    );
  }

  void _onNewBatteryLevel(int newLevel) {
    //TODO: replace with proper implementation
    debugPrint("New battery level: $newLevel");
  }

  void _onNewSensedData(List<int> newData) {
    //TODO: replace with proper implementation
    debugPrint("New sensed data: $newData");
  }
}
