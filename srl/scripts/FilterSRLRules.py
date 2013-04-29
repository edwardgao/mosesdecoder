#!/usr/bin/env python

from sys import *

if len(argv)<4:
    exit();

fi = open(argv[1],"r");
preds = set();
for l in fi:
    fields = l.strip().split("|||",2)
    preds.add(fields[0].strip());

fi.close();
fi = open(argv[2],"r");

fo = open(argv[3],"w");

for l in fi:
    fields = l.strip().split("|||",2);
    if fields[0].strip() in preds:
        fo.write(l);



