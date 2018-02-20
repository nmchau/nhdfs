#!/bin/bash

/start-hadoop.sh && sleep 20

npx mocha -t 0

