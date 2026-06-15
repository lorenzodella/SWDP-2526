import 'dart:async';
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:flutter_reactive_ble/flutter_reactive_ble.dart';
import 'package:permission_handler/permission_handler.dart';
import 'package:swdp_bio_app/packet_type.dart';

//region --- BLE Service and Characteristic UUIDs ---
// These are the specific addresses for the BLE device RN4871 (Microchip) on the board.
final Uuid serviceUuid = Uuid.parse("49535343-FE7D-4AE5-8FA9-9FAFD205E455");
final Uuid characteristicUuid = Uuid.parse("49535343-1E4D-4BD9-BA61-23C647249616"); // RX Characteristic
final Uuid characteristicUuidTX = Uuid.parse("49535343-8841-43F4-A8D4-ECBE34729BB3"); // TX Characteristic
//endregion

/// Enum representing the current BLE connection status.
enum BleConnectionStatus { disconnected, connecting, connected }

/// Singleton class responsible for managing all BLE-related operations.
class BleController extends ChangeNotifier {
  //region --- Singleton Pattern (ensure only one instance of BleController exists) ---

  static final BleController instance = BleController._internal();
  BleController._internal();

  //endregion

  //region --- CONSTANTS ---

  final String bleDeviceNameFilter = "BLE_SW";  //TODO: maybe update with a custom one
  final int fixedPacketLength = 20;
  final int scanDuration = 10; // in seconds
  final int requestedMtu = 512;
  final int connectionTimeout = 5; // in seconds

  //endregion

  //region --- Internal State ---

  final FlutterReactiveBle _ble = FlutterReactiveBle();

  StreamSubscription<DiscoveredDevice>? _scanStream;
  StreamSubscription<ConnectionStateUpdate>? _connection;

  StreamSubscription<List<int>>? _rxSubscription;
  QualifiedCharacteristic? _txCharacteristic;

  bool _permGranted = false;
  bool _permPermanentlyDenied = false;
  bool _scanning = false;
  BleConnectionStatus _connectionStatus = BleConnectionStatus.disconnected;

  final List<DiscoveredDevice> _foundBleDevices = [];
  final List<DiscoveredDevice> _foundBleDevicesFiltered = [];
  DiscoveredDevice? _connectedDevice;

  final StreamController<double> _ppgDataController =
      StreamController<double>.broadcast();
  final StreamController<double> _filteredPpgDataController =
      StreamController<double>.broadcast();
  final StreamController<int> _heartRateController =
      StreamController<int>.broadcast();
  final StreamController<double> _temperatureController =
      StreamController<double>.broadcast();

  //endregion

  //region --- Public API ---

  /// Whether the necessary permissions for BLE operations have been granted.
  bool get permissionsGranted => _permGranted;

  /// Whether any of the necessary permissions for BLE operations have been permanently denied.
  bool get permissionsPermanentlyDenied => _permPermanentlyDenied;

  /// Whether a BLE scan is currently in progress.
  bool get scanning => _scanning;

  /// The current BLE connection status.
  BleConnectionStatus get connectionStatus => _connectionStatus;

  /// List of BLE devices discovered during the last scan, filtered by [bleDeviceNameFilter].
  List<DiscoveredDevice> get foundBleDevices => _foundBleDevicesFiltered;

  /// The currently connected BLE device, or null if no device is connected.
  DiscoveredDevice? get connectedDevice => _connectedDevice;

  /// Stream providing the PPG data received from the connected device.
  Stream<double> get ppgDataStream => _ppgDataController.stream;

  /// Stream providing the filtered PPG data received from the connected device.
  Stream<double> get filteredPpgDataStream => _filteredPpgDataController.stream;

  /// Stream providing the heart rate updates received from the connected device.
  Stream<int> get heartRateStream => _heartRateController.stream;

  /// Stream providing the temperature updates received from the connected device.
  Stream<double> get temperatureStream => _temperatureController.stream;

  /// Initializes the BLE-related permissions.
  /// Should be called before any BLE operation.
  Future<void> initPermissions() async {
    Map<Permission, PermissionStatus> statuses = await [
      Permission.bluetoothScan,
      Permission.locationWhenInUse,
      Permission.bluetoothConnect,
    ].request();

    _permGranted =
        (statuses[Permission.bluetoothScan] == PermissionStatus.granted &&
        statuses[Permission.bluetoothConnect] == PermissionStatus.granted &&
        statuses[Permission.locationWhenInUse] == PermissionStatus.granted);

    _permPermanentlyDenied =
        (statuses[Permission.bluetoothScan] == PermissionStatus.permanentlyDenied ||
        statuses[Permission.bluetoothConnect] == PermissionStatus.permanentlyDenied ||
        statuses[Permission.locationWhenInUse] == PermissionStatus.permanentlyDenied);

    notifyListeners();
  }

  /// Starts scanning for available BLE devices,
  /// and updates the [foundBleDevices] list with the compatible ones found.
  void startScan() {
    // avoid starting a new scan if one is already in progress
    if (_scanning) {
      return;
    }

    // ensure needed permissions have been granted
    if (!_permGranted) {
      throw Exception("Permissions not granted. Call initPermissions() first.");
    }

    // clear previous scan results
    _foundBleDevices.clear();
    _foundBleDevicesFiltered.clear();
    _scanning = true;
    notifyListeners();

    _scanStream = _ble
        .scanForDevices(
          withServices: [/* serviceUuid */],
        ) //TODO: uncomment once we've ensured the firmware uses the same service UUID defined above
        .listen(
          (device) {
            // if the device hasn't been discovered yet, add it to the list of found devices
            if (!_foundBleDevices.any((element) => element.id == device.id)) {
              _foundBleDevices.add(device);
              // and if it matches the name filter, also add it to the filtered list (which is the one returned to the UI)
              if (device.name.contains(bleDeviceNameFilter)) {
                _foundBleDevicesFiltered.add(device);
              }
              notifyListeners();
            }
          },
          onError: (Object error) {
            debugPrint("ERROR during scan: $error");
          },
        );

    // stop scanning after scanDuration elapsed
    Future.delayed(Duration(seconds: scanDuration), () {
      if (_scanning) {
        stopScan();
      }
    });
  }

  /// Stops any ongoing BLE scan.
  void stopScan() {
    _scanStream?.cancel();
    _scanStream = null;
    _scanning = false;
    notifyListeners();
  }

  /// Initiates a connection to the specified [device].
  Future<void> startConnection(DiscoveredDevice device) async {
    // interrupt ongoing scan (if any)
    if (_scanning) {
      stopScan();
    }

    // if already connected -> do nothing
    if (_connectionStatus == BleConnectionStatus.connected) {
      return;
    }

    // update local state
    _connectionStatus = BleConnectionStatus.connecting;
    notifyListeners();

    try {
      await _ble.requestMtu(deviceId: device.id, mtu: requestedMtu);

      final completer = Completer<void>();

      _connection = _ble
          .connectToDevice(
            id: device.id,
            connectionTimeout: Duration(seconds: connectionTimeout),
          )
          .listen(
            (event) {
              switch (event.connectionState) {
                case DeviceConnectionState.connecting:
                  debugPrint("Connecting to ${event.deviceId}...");
                  break;

                case DeviceConnectionState.connected:
                  // execute the connection logic
                  _onDeviceConnected(event.deviceId);
                  // notify the UI that the connection is established
                  if (!completer.isCompleted) {
                    completer.complete();
                  }
                  break;

                case DeviceConnectionState.disconnected:
                  // execute the disconnection logic
                  _onDeviceDisconnected();
                  // notify the UI that the connection is lost
                  if (!completer.isCompleted) {
                    completer.completeError(
                      "Disconnected unexpectedly during connection setup",
                    );
                  }
                  break;

                default:
                  break;
              }
            },
            onError: (error) {
              // execute the disconnection logic
              _onDeviceDisconnected();
              // notify the UI that an error occurred
              if (!completer.isCompleted) {
                completer.completeError("Connection error: $error");
              }
            },
          );

      // wait until either connection is established or an error occurs
      await completer.future;
    } catch (e) {
      // execute the disconnection logic (just in case)
      _onDeviceDisconnected();
      // log the error
      debugPrint("ERROR during connection setup: $e");
      // and rethrow it to be handled by the UI
      rethrow;
    }
  }

  /// Sends the specified [command] to the currently connected BLE device.
  //TODO: maybe replace the List<int> type for command with a custom data model
  Future<void> sendCommand(List<int> command) async {
    if (_connectionStatus != BleConnectionStatus.connected) {
      throw Exception("Cannot send command: not connected to any device.");
    }
    if (_txCharacteristic == null) {
      throw Exception("TX characteristic not initialized.");
    }

    try {
      await _ble.writeCharacteristicWithoutResponse(
        _txCharacteristic!,
        value: command,
      );
    } catch (e) {
      debugPrint("ERROR during command send: $e");
      rethrow;
    }
  }

  /// Method that enable to inject synthetic data into the corresponding streams,
  /// simulating the reception of data from a real BLE device.
  ///
  /// (To be used for UI testing purposes, not intended for production use).
  void injectSyntheticData({
    double? ppg,
    double? filteredPpg,
    int? heartRate,
    double? temperature,
  }) {
    if (ppg != null) _ppgDataController.add(ppg);
    if (filteredPpg != null) _filteredPpgDataController.add(filteredPpg);
    if (heartRate != null) _heartRateController.add(heartRate);
    if (temperature != null) _temperatureController.add(temperature);
  }

  //endregion

  //region --- Internal Logic for Connection Handling ---

  void _onDeviceConnected(String deviceId) {
    // update local state
    _connectionStatus = BleConnectionStatus.connected;
    _connectedDevice = _foundBleDevicesFiltered.firstWhere(
      (d) => d.id == deviceId,
    );
    notifyListeners();
    debugPrint("Connected to $deviceId");

    // SETUP RX
    final rxCharacteristic = QualifiedCharacteristic(
      serviceId: serviceUuid,
      characteristicId: characteristicUuid,
      deviceId: deviceId,
    );

    // define what happens when a new packet is received by the RX
    List<int> packetBuffer = [];

    _rxSubscription = _ble
        .subscribeToCharacteristic(rxCharacteristic)
        .listen(
          (packet) {
            // add the received packet to the buffer
            packetBuffer.addAll(packet);

            // if at least a full packet has been received, process it and forward to the custom stream
            while (packetBuffer.length >= fixedPacketLength) {
              // pop the first full packet from the buffer
              List<int> data = packetBuffer.sublist(0, fixedPacketLength);
              packetBuffer.removeRange(0, fixedPacketLength);

              _processReceivedPacket(data);
            }
          },
          onError: (Object error) {
            debugPrint("RX error: $error");
          },
        );

    // SETUP TX
    final txCharacteristic = QualifiedCharacteristic(
      serviceId: serviceUuid,
      characteristicId: characteristicUuidTX,
      deviceId: deviceId,
    );
    _txCharacteristic = txCharacteristic;
  }

  void _processReceivedPacket(List<int> packet) {
    if (packet[0] != 123 || packet[packet.length - 1] != 125) {
      throw Exception("Invalid packet received: missing start/end delimiters");
    }

    PacketType packetType = PacketType.fromByte(packet[1]);

    switch (packetType) {
      case PacketType.ppgData:
        _processPpgDataPacket(packet, _ppgDataController);
        break;

      case PacketType.filteredPpgData:
        _processPpgDataPacket(packet, _filteredPpgDataController);
        break;

      case PacketType.heartRate:
        _processHeartRatePacket(packet);
        break;

      case PacketType.temperature:
        _processTemperaturePacket(packet);
        break;

      case PacketType.accelerationData:
      case PacketType.gyroscopeData:
      case PacketType.unknown:
        debugPrint(
          "Received packet of type $packetType, which is not supported yet.\nPacket content: $packet",
        );
    }
  }

  /// Process the received packet and forward the extracted data to the corresponding stream.
  ///
  /// Expected packet format: `{ | 'P' or 'F' | sample_count (≤4) | (up to) 4 × uint32_t in Big Endian | (eventually) padding | }`
  void _processPpgDataPacket(
    List<int> packet,
    StreamController<double> outputStream,
  ) {
    int packetLength = packet[2];
    List<int> dataBytes = packet.sublist(3, packet.length - 1);

    for (int i = 0; i < 4 * packetLength; i += 4) {
      // Extract the 4 bytes corresponding to the current float value
      List<int> valueBytes = dataBytes.sublist(i, i + 4);
      ByteData valueByteData = ByteData.sublistView(
        Uint8List.fromList(valueBytes),
      );

      // convert the bytes to a float value
      double value = valueByteData.getFloat32(0, Endian.big);

      // forward the value to the corresponding stream
      outputStream.add(value);
    }
  }

  /// Process the received heart rate packet and forward the extracted data to the corresponding stream.
  ///
  /// Expected packet format: `{ | 'H' | BPM (uint8_t) | padding | }`
  void _processHeartRatePacket(List<int> packet) {
    int bpm = packet[2];

    _heartRateController.add(bpm);
  }

  /// Process the received temperature packet and forward the extracted data to the corresponding stream.
  ///
  /// Expected packet format: `{ | 'T' | temperature (1 x float32_t in Big Endian) | padding | }`
  void _processTemperaturePacket(List<int> packet) {
    List<int> dataBytes = packet.sublist(2, packet.length - 1);

    // Extract the 4 bytes corresponding to the float temperature value
    List<int> valueBytes = dataBytes.sublist(0, 4);
    ByteData valueByteData = ByteData.sublistView(
      Uint8List.fromList(valueBytes),
    );

    // convert the bytes to a float
    double temperature = valueByteData.getFloat32(0, Endian.big);

    // forward the temperature value to the corresponding stream
    _temperatureController.add(temperature);
  }

  void _onDeviceDisconnected() {
    // clean up internal state
    _connectionStatus = BleConnectionStatus.disconnected;
    _rxSubscription?.cancel();
    _rxSubscription = null;
    _connection?.cancel();
    _connection = null;
    _connectedDevice = null;
    _txCharacteristic = null;

    notifyListeners();
    debugPrint("Device disconnected");
  }

  //endregion
}
