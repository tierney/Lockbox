#!/bin/bash
HOST=theseus.news.cs.nyu.edu
./client_main -host $HOST -port 9876 -email me1@you.com -password password -config_path $HOME/.lockbox 2>&1 | tee `date +%s`-client0.log
