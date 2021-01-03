import sys


if len(sys.argv) < 2:
  print('Usage: python3 get_blom_key_as_array.py *_blom_key')
  exit()

with open(sys.argv[1], 'r') as blom_key_file:
  for hex_blom_key_line in blom_key_file:
    hex_blom_key = hex_blom_key_line.strip()
    print([int(hex_blom_key[i:i+2],16) for i in range(0,len(hex_blom_key),2)])
