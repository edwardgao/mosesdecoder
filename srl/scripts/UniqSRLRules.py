#!/usr/bin/env python

from sys import *

filter = False;

if len(argv)>1 and argv[1] == 'f':
    filter = True;
    stderr.write("Filtering...");

currprd_role = None;
consmap = {};

count = 0;

def IsWord(w):
    return w.isalpha();

for line in stdin:
    fields = map(lambda x: x.strip(), line.strip().split("|||"));
    if len(fields) < 3:
        continue;
    if filter and not IsWord(fields[0]):
        continue;

    pred = fields[0];
    role = fields[1];
    cons = fields[2];

    pr = (pred, role);

    if currprd_role == pr:
        if consmap.has_key(cons):
            consmap[cons] += 1;
            count += 1;
        else:
            consmap[cons] = 1;
            count += 1
    else:
        if currprd_role <> None:
            for k,v in consmap.items():
                stdout.write("%s ||| %s ||| %s ||| %d %d\n"% (pr[0],pr[1],k,v, count));
        currprd_role = pr;
        consmap={cons : 1}
        count = 1;


for k,v in consmap.items():
    stdout.write("%s ||| %s ||| %s ||| %d %d\n"% (currprd_role[0],currprd_role[1],k,v,count));


