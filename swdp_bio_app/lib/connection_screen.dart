import 'package:flutter/material.dart';
import 'package:flutter_reactive_ble/flutter_reactive_ble.dart';
import 'package:swdp_bio_app/ble_controller.dart';
import 'package:swdp_bio_app/permission_request_screen.dart';

/// Screen to see available BLE devices and connect to one.
/// Also handles the permission request flow by showing the [PermissionRequestScreen] if permissions are not granted.
class ConnectionScreen extends StatefulWidget {
  const ConnectionScreen({super.key});

  @override
  State<ConnectionScreen> createState() => _ConnectionScreenState();
}

class _ConnectionScreenState extends State<ConnectionScreen> {
  @override
  void initState() {
    BleController.instance.initPermissions();
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return ListenableBuilder(
      listenable: BleController.instance,
      builder: (context, _) {
        final BleController controller = BleController.instance;
        final List<DiscoveredDevice> devices = controller.foundBleDevices;

        // show the permission request screen if permissions are not granted
        if (!controller.permissionsGranted) {
          return PermissionRequestScreen(controller: controller);
        }

        // otherwise, show the proper connection screen
        return Scaffold(
          appBar: AppBar(
            backgroundColor: Theme.of(context).colorScheme.inversePrimary,
            title: const Text("Connect to a Device"),
            actions: [
              //region --- Scan button ---
              controller.scanning
                  ? const Center(
                      child: Padding(
                        padding: EdgeInsetsGeometry.symmetric(horizontal: 16.0),
                        child: SizedBox(
                          height: 20.0,
                          width: 20.0,
                          child: CircularProgressIndicator(strokeWidth: 2.0),
                        ),
                      ),
                    )
                  : IconButton(
                    icon: const Icon(Icons.refresh),
                    onPressed: () => controller.startScan(),
                  ),
              //endregion
            ],
          ),
          body: Stack(
            children: [
              //region --- List of Discovered Devices ---
              Positioned.fill(
                child: devices.isEmpty
                    ? const Center(child: Text("No devices found. Please start scanning."))
                    : ListView.builder(
                      itemCount: devices.length,
                      itemBuilder: (context, index) {
                        final device = devices[index];
                        return ListTile(
                          title: Text(device.name.isEmpty ? "Unknown Device" : device.name),
                          subtitle: Text(device.id),
                          trailing: const Icon(Icons.bluetooth),
                          onTap: () => controller.startConnection(device),
                        );
                      },
                    ),
              ),
              //endregion

              //region --- Overlay while connecting ---
              if (controller.connectionStatus == BleConnectionStatus.connecting) ...[
                const Opacity(
                  opacity: 0.5,
                  child: ModalBarrier(dismissible: false, color: Colors.black),
                ),
                const Center(
                  child: CircularProgressIndicator(),
                ),
              ],
              //endregion
            ],
          ),
        );
      }
    );
  }
}
