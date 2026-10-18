#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Generate original release screenshot artwork and backdrop; requires Pillow."""
from PIL import Image,ImageDraw
import math
from pathlib import Path
p=Path(__file__).resolve().parents[1] / 'build' / 'website-art'
p.mkdir(parents=True, exist_ok=True)
w,h=1900,1250
im=Image.new('RGB',(w,h)); d=ImageDraw.Draw(im)
colors=[(34,45,100),(125,100,161),(235,155,137),(255,213,152)]
for y in range(h):
 t=y/h*3; n=min(2,int(t)); f=t-n
 c=tuple(round(colors[n][k]*(1-f)+colors[n+1][k]*f) for k in range(3));d.line((0,y,w,y),fill=c)
d.ellipse((1300,180,1560,440),fill='#ffe6b5')
for points,c in [([(0,720),(200,460),(350,610),(600,340),(880,670),(1170,440),(1500,700),(1750,390),(1900,640)],'#6e6c9d'), ([(0,850),(300,650),(500,810),(850,530),(1100,790),(1450,590),(1700,820),(1900,690)],'#405e83'), ([(0,1000),(280,830),(650,970),(930,760),(1200,920),(1600,730),(1900,920)],'#214f69')]:
 d.polygon(points+[(w,h),(0,h)],fill=c)
for y in range(1050,h,13):
 length=int(130+170*math.sin(y*.037)**2);x=1430+int(40*math.sin(y*.03));d.line((x-length,y,x+length,y),fill='#658b96',width=2)
im.save(p/'Backdrop.png')
# Original illustration opened as the sample drawing.
a=Image.new('RGB',(1100,620),'#f6efd9');q=ImageDraw.Draw(a)
q.ellipse((735,55,920,240),fill='#e8a047')
q.polygon([(0,420),(160,260),(310,390),(520,165),(740,400),(960,260),(1100,405),(1100,620),(0,620)],fill='#9ab5b0')
q.polygon([(0,510),(260,350),(470,500),(735,300),(1100,510),(1100,620),(0,620)],fill='#467c7c')
q.polygon([(0,570),(280,480),(520,575),(840,440),(1100,540),(1100,620),(0,620)],fill='#194b57')
a.save(p/'Mountain study.png')
