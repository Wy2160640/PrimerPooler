#!/bin/bash
wget -N http://people.ds.cam.ac.uk/ssb22/pooler/pooler.tgz
wget -N http://people.ds.cam.ac.uk/ssb22/pooler/script2canvas.py
tar -zxvf pooler.tgz
git add pooler/*
git commit -am update && git push
