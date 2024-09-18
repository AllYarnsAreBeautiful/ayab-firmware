# CRC algorithm after Maxim/Dallas
def crc8(crc: int, data: bytearray) -> int:
    for i in range(len(data)):
        n = data[i]
        for _j in range(8):
            f = (crc ^ n) & 1
            crc >>= 1
            if f:
                crc ^= 0x8C
            n >>= 1
    return crc & 0xFF