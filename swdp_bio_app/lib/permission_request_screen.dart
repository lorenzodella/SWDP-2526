import 'package:flutter/material.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:swdp_bio_app/ble_controller.dart';

/// Screen to be shown when Bluetooth permissions are not granted, prompting the user to grant them.
/// If permissions are permanently denied, it will show a button to open the app settings.
class PermissionRequestScreen extends StatelessWidget {
  const PermissionRequestScreen({super.key, required this.controller});

  final BleController controller;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text(
              "PERMISSIONS NOT GRANTED.",
              textAlign: TextAlign.center,
              style: TextStyle(fontSize: 18.0, fontWeight: FontWeight.bold),
            ),
            Text(
              "Please grant Bluetooth permissions to use the app.",
              textAlign: TextAlign.center,
            ),
            SizedBox(height: 8.0,),
            if (controller.permissionsPermanentlyDenied)
              ElevatedButton(
                onPressed: () async {
                  await openAppSettings();
                  await controller.initPermissions();
                },
                style: ElevatedButton.styleFrom(
                  backgroundColor: Theme.of(context).colorScheme.primary,
                  foregroundColor: Theme.of(context).colorScheme.onPrimary,
                ),
                child: const Text("GO TO APP SETTINGS"),
              )
            else
              ElevatedButton(
                onPressed: () async {
                  await controller.initPermissions();
                },
                style: ElevatedButton.styleFrom(
                  backgroundColor: Theme.of(context).colorScheme.primary,
                  foregroundColor: Theme.of(context).colorScheme.onPrimary,
                ),
                child: const Text("GRANT PERMISSIONS"),
              ),
          ],
        ),
      )
    );
  }
}
