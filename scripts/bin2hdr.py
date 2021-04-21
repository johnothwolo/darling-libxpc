#!/usr/bin/env python

import sys

def to_padded_hex(value, hex_digits=2):
	return '0x{0:0{1}x}'.format(value if isinstance(value, int) else ord(value), hex_digits)

def to_c_array(array_name, values, array_type='uint8_t', formatter=to_padded_hex, column_count=8, static=True):
	values = [formatter(v) for v in values]
	rows = [values[i:i + column_count] for i in range(0, len(values), column_count)]
	body = ',\n\t'.join([', '.join(r) for r in rows])
	return '{}{} {}[] = {{\n\t{},\n}};\n'.format('static ' if static else '', array_type, array_name, body)

if len(sys.argv) < 4:
	print("Usage: {} <array-name> <binary-path> <output-path>".format(sys.argv[0]))
	exit(1)

array_name = sys.argv[1]
binary_path = sys.argv[2]
output_path = sys.argv[3]

input_binary = open(binary_path, "rb")
input_data = input_binary.read()
input_binary.close()

output_string = str()

output_string += '#ifndef _BIN2HDR_GENERATED_' + array_name + '_\n'
output_string += '#define _BIN2HDR_GENERATED_' + array_name + '_\n\n'
output_string += '#include <stdint.h>\n\n'

output_string += to_c_array(array_name, input_data) + '\n'

output_string += '#endif // _BIN2HDR_GENERATED_' + array_name + '_\n'

output_file = open(output_path, 'w')
output_file.write(output_string)
