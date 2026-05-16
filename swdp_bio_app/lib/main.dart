import 'package:flutter/material.dart';
import 'package:swdp_bio_app/ble_controller.dart';
import 'package:swdp_bio_app/connection_screen.dart';
import 'package:swdp_bio_app/home_screen.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'SWDP BIO A6 APP',
      theme: ThemeData(
        colorScheme: .fromSeed(seedColor: Colors.deepPurple),
      ),
      home: ListenableBuilder(
        listenable: BleController.instance,
        builder: (context, _) {
          if (BleController.instance.connectionStatus == BleConnectionStatus.connected) {
            return const HomeScreen();
          } else {
            return const ConnectionScreen();
          }
        }
      ),
    );
  }
}