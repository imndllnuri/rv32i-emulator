"""Shared hexdump formatting used by memory_window.py and stack_window.py."""


def format_hexdump(data, start_addr, bytes_per_line=16):
    """Convert a list of byte values into a traditional hexdump.

    Args:
        data: list of integers (0-255)
        start_addr: address of the first byte
        bytes_per_line: number of bytes shown per line
    Returns:
        Multiline string: address + hex + ASCII
    """
    lines = []
    ascii_repr = []
    hex_bytes = []
    per_line = bytes_per_line

    for i, byte in enumerate(data):
        if i % per_line == 0 and i != 0:
            line_addr = start_addr + i - per_line
            line = f"{line_addr:08X}:  {' '.join(hex_bytes):<{per_line*3}}  |{''.join(ascii_repr)}|"
            lines.append(line)
            hex_bytes = []
            ascii_repr = []

        hex_bytes.append(f"{byte:02X}")
        if 32 <= byte <= 126:
            ascii_repr.append(chr(byte))
        else:
            ascii_repr.append('.')

    if hex_bytes:
        line_addr = start_addr + len(data) - len(hex_bytes)
        hex_pad = ' '.join(hex_bytes)
        hex_padded = f"{hex_pad:<{per_line*3}}"
        line = f"{line_addr:08X}:  {hex_padded}  |{''.join(ascii_repr)}|"
        lines.append(line)

    return '\n'.join(lines)
