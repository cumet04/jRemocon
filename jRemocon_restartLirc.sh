#!/bin/bash

# kill -1 $(pid)だと、そもそも起動してない場合は動かない
sudo kill -9 $(pidof lircd)
sudo lircd
