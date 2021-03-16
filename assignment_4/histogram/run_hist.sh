#!/bin/bash
prun -v -1 -np 1 -1 -native '-C TitanX --gres=gpu:1' ./myhistogram
