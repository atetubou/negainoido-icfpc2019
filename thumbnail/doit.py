#!/usr/bin/env python3
from PIL import Image, ImageDraw, ImageFont
import glob
import os

def ch2color(ch):
    if ch == '.':
        color = (200,200,200)
    elif ch == 'W':
        color = (252, 13, 27) # RED
    elif ch == 'X':
        color = (11,36,251) # blue
    elif ch == 'F':
        color = (152,101,21)
    elif ch == 'B':
        color = (250,200,68)
    elif ch == 'L':
        color = (31,202,35)
    elif ch == 'R':
        color = (147,115,216)
    elif ch == 'C':
        color = (58,155,252)
    elif ch == '#':
        color = (30,30,30)
    else:
        print('Unexpected:', ch)
    return color

def conv(path):
    with open(path, 'r') as f:
        base = os.path.basename(path)
        h,w = map(int, f.readline().split())
        size = 800
        cell = size // max(h,w)
        print(cell * h, cell * w)
        img = Image.new("RGB", (cell * w, cell * h), (256,256,256))
        draw = ImageDraw.Draw(img)
        dotsize = max(3, cell)
        objs = []
        for y in range(h):
            line = f.readline().strip()
            for x in range(w):
                ch = line[x]
                X = x * cell
                Y = y * cell
                color = ch2color(ch)
                if ch in ".#":
                    draw.rectangle((X, Y, X + cell, Y+cell), fill=color)
                else:
                    objs.append((X,Y,color, ch))
        for (X,Y,color,ch) in objs:
            draw.ellipse(( X + cell / 2 - dotsize, Y + cell / 2 - dotsize
                         , X + cell / 2 + dotsize, Y + cell / 2 + dotsize),
                         fill = color, outline=(0,0,0))
        img.save(base + '.png')

def main():
    for path in glob.glob("../problems/*.in"):
        print(path)
        conv(path)
if __name__ == '__main__':
    main()
