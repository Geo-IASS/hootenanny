#!/bin/bash

cd $HOOT_HOME && make -s clean && aclocal && autoconf && autoheader && automake && ./configure --with-rnd --with-services --with-uitests && make -sj$(nproc)