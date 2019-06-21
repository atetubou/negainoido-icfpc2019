import sys

def to_point(s):
    # x,y
    return map(int, p.split(','))

def print_table(H, W, table, f=sys.stderr):
    for i in range(H):
        for j in range(W):
            f.write(table[j][H-1-i])
        f.write('\n')
    return

input_fname = sys.argv[1]

# The name of output file is .in.
output_fname = input_fname[:input_fname.rfind('.')] + '.in'

# Assuming input data is one line.
data = open(input_fname).readline()

map_data, wrap_data, obst_data, mine_data = data.split('#')

# parse map_data
tmp_points = []
W = 0
H = 0
map_data = map_data[1:-1]
for p in map_data.split('),('):
    x,y = to_point(p)
    W = max(W, x)
    H = max(H, y)
    tmp_points.append((x,y))

points = [[] for i in range(W+1)]
for p in tmp_points:
    points[p[0]].append(p[1])

table = [['@' for _ in range(H)] for _ in range(W)]

for i in range(len(points)-1):
    mark = '|'
    do = False
    prev_y = 0
    points[i].sort()

    for y in points[i]:
        if do:
            for j in range(prev_y, y):
                table[i][j] = mark
        prev_y = y
        do = not do

for obst in obst_data.split(';'):
    if not obst:
        continue
    points = [[] for i in range(W+1)]
    obst = obst[1:-1]
    for p in obst.split('),('):
        x,y = to_point(p)
        points[x].append(y)
    for i in range(len(points)):
        mark = '|'
        do = True
        points[i].sort()
        for j in range(len(points[i])-1):
            y1,y2 = points[i][j], points[i][j+1]
            # fill mark in [y1, y2)
            if do:
                for k in range(y1, y2):
                    table[i][k] = mark
            do =  not do

# sort up table
for j in range(H):
    mark = '#'
    for i in range(W):
        if table[i][j] == '|':
            if mark == '.':
                mark = '#'
            else:
                mark = '.'
        table[i][j] = mark

# parse wrap_data
if wrap_data:
    wrap_data = wrap_data[1:-1]
    x,y = map(int, wrap_data.split(','))
    table[x][y] = 'W'

# parse min_data
for mine in mine_data.split(';'):
    if not mine:
        continue
    mine_desc = mine[0]
    mine = mine[2:-1]
    x, y = map(int, mine.split(','))
    table[x][y] = mine_desc

with open(output_fname, mode='w') as of:
    of.write('%d %d' % (H, W))
    print_table(H, W, table, of)


# output stderr too.
# sys.stderr.write('%d %d' % (H, W))
# print_table(H, W, table)
