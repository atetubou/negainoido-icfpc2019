#!/bin/python3

def split_input(s):
    arr = s.split('#')
    nums = [int(x) for x in arr[0].split(',')]

    include_vs_tmp = arr[1][1:-1].split('),(')
    include_vs = []
    for x in include_vs_tmp:
        tpl = list(map(int, x.split(',')))
        include_vs.append(tpl)

    exclude_vs_tmp = arr[2][1:-1].split('),(')
    exclude_vs = []
    for x in exclude_vs_tmp:
        tpl = list(map(int, x.split(',')))
        exclude_vs.append(tpl)

    include_vs.sort()
    exclude_vs.sort()

    return nums, include_vs, exclude_vs

def inBoard(tSize, x,y):
    return 0 <= x < tSize and 0 <= y < tSize


def create_map(nums, ivs, evs):


    tSize = nums[2]

    board = [["."]* tSize for _ in range(tSize)]

    for (ix, iy) in ivs:
        board[ix][iy] = 'I'

    for (x, y) in evs:
        if board[x][y] != '.':
            continue

        if x == 0 or x == tSize-1 or y == 0 or y == tSize - 1:
            board[x][y] = "#"
            continue

        success = False

        for dx, dy in [ (1, 0), (0, 1), (-1, 0), (0, -1)]:
            nx = x + dx
            ny = y + dy
            ok = True
            while (inBoard(tSize, nx, ny) and board[nx][ny] == '.'):
                if board[nx][ny] == 'I':
                    ok = false
                    break
                nx += dx
                ny += dy

            if not ok:
                continue

            nx = x
            ny = y
            while (inBoard(tSize, nx, ny)):
                board[nx][ny] = '#'
                nx += dx
                ny += dy

            success = True
            break
        assert(success)

    return board


def count_angles(board, tSize):
    ans = 0

    for i in range(-1, tSize+1):
        for j in range(-1, tSize+1):
            cnt = 0
            for nx, ny in [(i, j), (i+1, j), (i, j+1), (i+1, j+1)]:
                if (not inBoard(tSize, nx, ny)) or board[nx][ny] == "#":
                    cnt += 1

            if cnt == 1 or cnt ==3:
                ans += 1

    return ans

def fill_items(nums, board):
    # bNum, eNum, tSize, vMin, vMax, mNum, fNum, dNum,rNum,cNum, xNum
    item_nums = nums[5:]
    item_char =["B", "F", "L", "R", "C", "X"]

    for i in range(6):
        cnt = item_nums[i]

        while(cnt > 0):
            for x in range(len(board)):
                if cnt == 0:
                    break
                for y in range(len(board)):
                    if cnt == 0:
                        break
                    if board[x][y] == ".":
                        board[x][y] = item_char[i]
                        cnt -= 1

    for x in range(len(board)):
        for y in range(len(board)):
            if board[x][y] == ".":
                board[x][y] = "W"
                return

def main():
    #inp='1,1,200,400,1200,6,10,5,1,3,4#(117,146),(36,132),(119,69),(173,139),(45,26),(169,84),(135,74),(89,44),(111,98),(73,125),(127,140),(95,74),(36,20),(180,76),(47,28),(71,92),(177,138),(52,179),(122,83),(173,137),(179,102),(24,139),(91,95),(91,162),(79,116),(187,115),(127,123),(116,68),(60,168),(56,180),(37,19),(12,135),(120,90),(39,19),(59,131),(105,48),(29,131),(38,36),(31,135),(118,119),(73,160),(57,85),(100,77),(87,74),(89,97),(182,70),(141,96),(45,17),(29,149),(134,57)#(166,37),(92,14),(62,9),(137,187),(152,35),(148,52),(20,30),(43,136),(149,116),(28,99),(147,185),(145,18),(33,182),(153,0),(159,70),(18,8),(127,158),(65,95),(14,9),(110,186),(34,81),(183,25),(154,66),(53,98),(33,120),(44,172),(162,138),(1,60),(13,1),(137,12),(112,16),(7,19),(39,6),(1,41),(7,94),(105,90),(155,128),(90,170),(107,26),(134,52),(23,128),(190,90),(106,155),(110,150),(151,72),(19,52),(189,70),(53,93),(132,100),(92,81)'

    #inp = '0,1,150,400,1200,6,10,5,1,3,4#(73,61),(49,125),(73,110),(98,49),(126,89),(68,102),(51,132),(101,123),(22,132),(71,120),(97,129),(118,76),(85,100),(88,22),(84,144),(93,110),(96,93),(113,138),(91,52),(27,128),(84,140),(93,143),(83,17),(123,85),(50,74),(139,97),(101,110),(77,56),(86,23),(117,59),(133,126),(83,135),(76,90),(70,12),(12,141),(116,87),(102,76),(19,138),(86,129),(86,128),(83,60),(100,98),(60,105),(61,103),(94,99),(130,124),(141,132),(68,84),(86,143),(72,119)#(145,82),(20,65),(138,99),(38,137),(85,8),(125,104),(117,48),(57,48),(64,119),(3,25),(40,22),(82,54),(121,119),(1,34),(43,98),(97,120),(10,90),(15,32),(41,13),(86,40),(3,83),(2,127),(4,40),(139,18),(96,49),(53,22),(5,103),(112,33),(38,47),(16,121),(133,99),(113,45),(50,5),(94,144),(16,0),(93,113),(18,141),(36,25),(56,120),(3,126),(143,144),(99,62),(144,117),(48,97),(69,9),(0,9),(141,16),(55,68),(81,3),(47,53)'

    inp="0,1,10,20,40,0,0,0,0,0,0#(2,7),(7,8),(7,1)#(3,2),(2,5),(6,3),(7,6)"

    nums, ivs, evs = split_input(inp)
    board = create_map(nums, ivs, evs)
    tSize = nums[2]
    ang = count_angles(board, nums[2])

    vmin = nums[3]
    vmax = nums[4]

    for i in range(tSize):
        if i % 2 == 0:
            continue

        ok = True
        for dx in [-1, 0, 1]:
            for dy in [-1, 0, 1]:
                if inBoard(tSize, dx, i+dy) and board[0 + dx][i + dy] == "#":
                    ok = False
        if ok:
            board[0][i] = "#"


    fill_items(nums, board)

    print("{} {}".format(tSize, tSize))
    for l in board:
        for i in range(len(l)):
            if l[i] == 'I':
                l[i] = '.'
        print(''.join(l))

main()
