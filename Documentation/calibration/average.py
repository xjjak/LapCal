import re
import sys

p = re.compile(r"\[-?\d*,-?\d*\] -->")
sub_p = re.compile(r"\[-?\d*,-?\d*\]")
values = [[] for _ in range(6)]
inp = "[-265,-264] --> [-7,13]    [-2993,-2992] --> [-9,8]        [1709,1710] --> [16368,16390]   [-174,-173] --> [-1,2]  [214,215] --> [0,1]     [45,46] --> [0,3]"

for i, line in enumerate(sys.stdin):
    for j, elem in enumerate(p.findall(line)):
        values[j].append(sub_p.match(elem).group().strip("[]").split(','))

result = []
for axis in values:
    buf = 0
    for pair in axis:
        buf += (int(pair[0])+int(pair[1]))/2
    result.append(round(buf/3,1))


print(result)
print('{' + ", ".join([str(round(num)) for num in result]) + '}')
    
