# this script converts ECE 385 Codepage 437 font ROM file into C-formatted file for the OS
f = open('fontfile.txt', 'r')
writef = open("outputfile.c",'w')
line=''

writef.write('int getCodePage437Character(int index, int y) {\n')

for group in range(7):
    writef.write(f'    unsigned int characters_codepage437_{group}[][150] = {{')
    for glyphs in range(13):
        buf = ''
        buf += '    {\n'
        for lines in range(17):
            line = f.readline()
            buf += '        '
            buf += line
        buf += "    },\n"
        writef.write(buf)
        print(buf)
    writef.write('    };//################################################################################\n')

writef.write(f'    unsigned int characters_codepage437_{7}[][150] = {{')
for glyphs in range(4):
    buf = ''
    buf += '    {\n'
    for lines in range(17):
        line = f.readline()
        buf += '        '
        buf += line
    buf += "    },\n"
    writef.write(buf)
    print(buf)

writef.write('''};
    int start = (int)(' ');
	if (index >= start && index < start + 13) {
		return characters_codepage437_0[index - start][y];
	}
	else if (index >= start + 13 && index < start + 13 * 2) {
		return characters_codepage437_1[index - (start + 13)][y];
	}
	else if (index >= start + 13 * 2 && index < start + 13 * 3) {
		return characters_codepage437_2[index - (start + 13 * 2)][y];
	}
	else if (index >= start + 13 * 3 && index < start + 13 * 4) {
		return characters_codepage437_3[index - (start + 13 * 3)][y];
	}
	else if (index >= start + 13 * 4 && index < start + 13 * 5) {
		return characters_codepage437_4[index - (start + 13 * 4)][y];
	}
	else if (index >= start + 13 * 5 && index < start + 13 * 6) {
		return characters_codepage437_5[index - (start + 13 * 5)][y];
	}
	else if (index >= start + 13 * 6 && index < start + 13 * 7) {
		return characters_codepage437_6[index - (start + 13 * 6)][y];
	}
	else if (index >= start + 13 * 7 && index < start + 13 * 8) {
		return characters_codepage437_7[index - (start + 13 * 7)][y];
	}
}

const int font_codepage437_width = 8;
const int font_codepage437_height = 16;''')

f.close()
writef.close()