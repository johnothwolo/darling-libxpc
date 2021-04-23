#!/usr/bin/env python

#
# This file is part of Darling.
#
# Copyright (C) 2021 Darling developers
#
# Darling is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Darling is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Darling.  If not, see <http://www.gnu.org/licenses/>.
#

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
