/// Enumeration of supported packet types for communication with the smart wearable device.
enum PacketType {
  accelerationData(65), // 'A' = 65
  gyroscopeData(71), // 'G' = 71
  ppgData(80), // 'P' = 80
  filteredPpgData(70), // 'F' = 70
  heartRate(72), // 'H' = 72
  temperature(84), // 'T' = 84
  unknown(85); // 'U' = 85

  /// The numeric value associated with the packet type.
  final int value;

  const PacketType(this.value);

  /// Factory constructor to create a [PacketType] from its byte value.
  factory PacketType.fromByte(int byte) {
    return PacketType.values.firstWhere(
      (type) => type.value == byte,
      orElse: () => PacketType.unknown,
    );
  }
}
