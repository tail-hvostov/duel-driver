def finalize_letter(line):
    global line_counter, sym_counter, fast_bytes, line_delim
    line_counter = 1
    sym_counter += 1
    print(line_delim + "{", end="", file=out)
    delim = ""
    for i in range(5):
        print(delim + hex(fast_bytes[i]), end="", file=out)
        delim = ", "
    print("}", file=out, end="")
    line_delim = ",\n    "
    fast_bytes = [0, 0, 0, 0, 0]

def handle_letter_layer(line):
    global line_counter
    for i in range(5):
        if line[i] == '-':
            val = 0
        else:
            val = 1
        val = val << (line_counter - 1)
        fast_bytes[i] |= val
    line_counter += 1

file = open("font.txt", "r")
out = open("encoded.txt", "w")

print("static const fast_sym fast_syms[] = {", file=out)

line = file.readline()
sym_counter = 0
line_counter = 1
fast_bytes = [0, 0, 0, 0, 0]
line_delim = "    "
while (line != ''):
    if (line_counter < 8):
        handle_letter_layer(line)
    else:
        finalize_letter(line)
    line = file.readline()
file.close()

finalize_letter(line)
print("\n};", file=out)

out.close()

print(f"Parsed {sym_counter} symbols.")
